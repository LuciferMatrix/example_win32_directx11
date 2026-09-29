#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <D3DX11tex.h>
#include <d3d11.h>
#include <windows.h>
#pragma comment(lib, "D3DX11.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#include <thread>

#include <iostream>

#include <atomic>

#include <string>

#include <vector>

#include "examples/example_win32_directx11/ext/MinHook/include/MinHook.h"

#include <TlHelp32.h>

#include <Psapi.h>

#pragma comment(lib, "Psapi.lib")

#include <VersionHelpers.h>

#include <winver.h>

#pragma comment(lib, "Version.lib")

#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#include <sstream>

#include <chrono>

#include <cmath>

#include <cstdlib>

#include <sddl.h>

#pragma comment(lib, "advapi32.lib")

#include <examples/example_win32_directx11/src/adb/adb.hpp>

#include <examples/example_win32_directx11/ImAnim/Easing.h>

#include "imgui_internal.h"

#ifndef IM_PI

#define IM_PI 3.14159265358979323846f

#endif

#include <examples/example_win32_directx11/src/Overlay/Overlay.hpp>

#include <examples/example_win32_directx11/src/ui/ui.hpp>

#include <examples/example_win32_directx11/src/Globals.hpp>

#include <examples/example_win32_directx11/EspLines/Data/Data.hpp>

#include <examples/example_win32_directx11/EspLines/Memory/Memory.hpp>

#include <examples/example_win32_directx11/EspLines/Offsets.hpp>

#include <examples/example_win32_directx11/EspLines/Visuals/Visual.hpp>
#include <examples/example_win32_directx11/EspLines/Loot/LootVisual.hpp>
#include "src/Overlay/Render.hpp"

#include <examples/example_win32_directx11/src/Fonts/Fonts.hpp>
#include <imgui_settings.h>

// Extern font declarations

namespace font {

extern ImFont *inter_semibold;
extern ImFont *icomoon_page;
extern ImFont *esp_font;
extern ImFont *small_font;

} // namespace font

// Global logo texture handle for GravityX header (optional).

// If never assigned, related rendering code simply won't draw the logo.

namespace image {

ID3D11ShaderResourceView *logogravity = nullptr;

}

using namespace adb;

HMODULE g_hModule;

bool bShouldUnload = false;

FWork::Interface *g_pInterface = nullptr;

// ---------------- Startup overlay helpers ----------------

std::atomic<bool> g_AdbReady{false};
std::atomic<bool> g_AdbFailed{false};
std::atomic<bool> g_AuthStarted{false};
std::atomic<bool> g_AuthDone{false};
std::atomic<bool> g_AuthOK{false};

static std::wstring GetHWID() {

  std::wstring result = L"";

  HANDLE hToken;

  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {

    DWORD tokenUserSize = 0;

    GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenUserSize);

    if (tokenUserSize > 0) {

      std::vector<BYTE> tokenUser(tokenUserSize);

      if (GetTokenInformation(hToken, TokenUser, tokenUser.data(),
                              tokenUserSize, &tokenUserSize)) {

        PTOKEN_USER pTokenUser = (PTOKEN_USER)tokenUser.data();

        LPWSTR sidString = NULL;

        if (ConvertSidToStringSidW(pTokenUser->User.Sid, &sidString)) {

          result = std::wstring(sidString);

          LocalFree(sidString);
        }
      }
    }

    CloseHandle(hToken);
  }

  return result;
}

static std::string GetProcessPathFromWindow(HWND hwnd) {

  DWORD pid = 0;

  GetWindowThreadProcessId(hwnd, &pid);

  if (pid == 0)
    return "";

  HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

  if (!h)
    return "";

  char buf[MAX_PATH * 4] = {0};

  DWORD size = static_cast<DWORD>(sizeof(buf));

  std::string path;

  if (QueryFullProcessImageNameA(h, 0, buf, &size)) {

    path.assign(buf, size);
  }

  CloseHandle(h);

  return path;
}

static std::string GetFileVersionString(const std::string &filePath) {

  DWORD handle = 0;

  DWORD verSize = GetFileVersionInfoSizeA(filePath.c_str(), &handle);

  if (verSize == 0)
    return "";

  std::vector<char> data(verSize);

  if (!GetFileVersionInfoA(filePath.c_str(), handle, verSize, data.data()))

    return "";

  VS_FIXEDFILEINFO *info = nullptr;

  UINT len = 0;

  if (!VerQueryValueA(data.data(), "\\", (LPVOID *)&info, &len) || len == 0 ||
      info == nullptr)

    return "";

  std::ostringstream oss;

  oss << HIWORD(info->dwFileVersionMS) << "."

      << LOWORD(info->dwFileVersionMS) << "."

      << HIWORD(info->dwFileVersionLS) << "."

      << LOWORD(info->dwFileVersionLS);

  return oss.str();
}

static std::string DetectEmulatorInfo(HWND targetWnd) {

  if (!targetWnd)
    return "Emulator: Unknown";

  char title[256] = {0};

  GetWindowTextA(targetWnd, title, sizeof(title) - 1);

  std::string windowTitle = title;

  std::string path = GetProcessPathFromWindow(targetWnd);

  std::string ver = GetFileVersionString(path);

  std::string name = "Unknown";

  std::string lowerTitle = windowTitle;

  for (auto &c : lowerTitle)
    c = (char)tolower(c);

  if (lowerTitle.find("bluestacks") != std::string::npos ||
      path.find("BlueStacks") != std::string::npos) {

    name = "BlueStacks";

  }

  else if (lowerTitle.find("msi") != std::string::npos ||
           path.find("MSI") != std::string::npos) {

    name = "MSI App Player";
  }

  std::ostringstream oss;

  oss << name;

  if (!ver.empty())
    oss << " v" << ver;

  return std::string("Emulator: ") + oss.str();
}

static void ApplyEmulatorPerformanceMode() {
  static bool s_lastPerf = false;
  const bool perf = g_Globals.General.DisableAllEffects;
  if (perf == s_lastPerf)
    return;
  s_lastPerf = perf;
  // Lower overlay priority so the emulator gets more CPU time.
  SetPriorityClass(GetCurrentProcess(), perf ? BELOW_NORMAL_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS);
}

static void DrawStartupOverlay(float elapsed, float total,
                               const std::string &emulatorInfo,
                               float exitCountdown = -1.0f,
                               bool adbError = false, bool authError = false) {

  ImGuiIO &io = ImGui::GetIO();

  ImDrawList *dl = ImGui::GetForegroundDrawList();

  ImFont *font = font::inter_semibold;

  if (!font)
    font = ImGui::GetFont();

  ImColor successGreen = ImColor(0, 255, 150, 255);

  ImColor errorRed = ImColor(255, 80, 80, 255);

  const ImVec2 center =
      ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

  // Panel size - 300x300

  const ImVec2 panelSize(300.0f, 300.0f);

  const ImVec2 panelMin(center.x - panelSize.x * 0.5f,
                        center.y - panelSize.y * 0.5f);

  const ImVec2 panelMax(center.x + panelSize.x * 0.5f,
                        center.y + panelSize.y * 0.5f);

  // Blur background

  dl->AddRectFilled(ImVec2(0, 0), io.DisplaySize, ImColor(0, 0, 0, 180));

  // Black panel with rounded edges

  dl->AddRectFilled(panelMin, panelMax, ImColor(0, 0, 0, 255), 12.0f);

  dl->AddRect(panelMin, panelMax, ImColor(50, 50, 50, 255), 12.0f, 0, 1.0f);

  // Loading circle position (center of panel)

  ImVec2 circlePos(center.x, center.y - 20.0f);

  // Static variables for animation

  static float fade_alpha = 1.0f;

  static int current_stage = 0;

  static float stage_timer = 0.0f;

  static float text_offset_y = 0.0f;

  static float text_alpha = 0.0f;

  static bool done = false;

  static float final_text_alpha = 0.0f;

  const char *stages[] = {

      "Checking updates...",

      "Validating license...",

      "Synchronizing files...",

      "Fetching config...",

      "Injecting modules...",

      "Patching memory...",

      "Finalizing startup..."

  };

  const int total_stages = sizeof(stages) / sizeof(stages[0]);

  // Determine success state

  bool success = g_AuthDone.load() && g_AuthOK.load() && !g_AdbFailed.load();

  // Animation time

  static float time = 0.0f;

  time += io.DeltaTime * 1.0f;

  float t = fmodf(time, 1.0f);

  // Smoother easing function for angle with better interpolation

  float angle =
      (t < 0.5f)

          ? (float)imanim::Easing::easeInOutCubic(t / 0.5f) * IM_PI

          : IM_PI + (float)imanim::Easing::easeInOutCubic((t - 0.5f) / 0.5f) *
                        IM_PI;

  float radius = 30.0f; // Larger radius

  float thickness = 6.0f; // Thicker circle

  // Pulsing glow effect for circle

  static float pulse_phase = 0.0f;

  pulse_phase += io.DeltaTime * 2.0f;

  float pulse_alpha = 0.3f + sinf(pulse_phase) * 0.2f;

  // Draw circle glow layers

  for (int glow_layer = 3; glow_layer > 0; glow_layer--) {

    float glow_radius = radius + (glow_layer * 3.0f);

    float glow_alpha = (0.15f / glow_layer) * fade_alpha * pulse_alpha;

    dl->PathClear();

    dl->PathArcTo(circlePos, glow_radius, 0.0f, 2.0f * IM_PI, 40.0f);

    ImColor glowColor = utils::GetColorWithAlpha(c::anim::active, glow_alpha);

    dl->PathStroke(glowColor, 0, thickness + (glow_layer * 1.5f));
  }

  // Draw circle background

  dl->PathClear();

  dl->PathArcTo(circlePos, radius, 0.0f, 2.0f * IM_PI, 40.0f);

  ImColor circleBgColor = utils::GetColorWithAlpha(
      c::anim::active, 0.4f * fade_alpha * pulse_alpha);

  dl->PathStroke(circleBgColor, 0, thickness);

  // Draw animated particles with smoother gradient

  float arc_size = 0.5f;

  float initialAngle = IM_PI * (1.5f + arc_size) + angle;

  ImVec2 currentPos = ImVec2(circlePos.x + cosf(initialAngle) * radius,
                             circlePos.y + sinf(initialAngle) * radius);

  for (int i = 0; i < 50; i++) { // More particles for smoother effect

    float particle_progress = i / 50.0f;

    float alpha = (1.0f - particle_progress) * fade_alpha;

    float particle_size =
        3.0f + sinf(particle_progress * IM_PI) * 1.0f; // Varying size

    ImColor particleColor = utils::GetColorWithAlpha(c::anim::active, alpha);

    dl->AddCircleFilled(currentPos, particle_size, particleColor);

    // Enhanced glow for particles

    ImColor shadowColor =
        utils::GetColorWithAlpha(c::anim::active, alpha * 0.6f);

    dl->AddShadowCircle(currentPos, particle_size, shadowColor, 20.0f,
                        ImVec2(0, 0));

    float nextAngle = initialAngle + i * (3.2f / radius);

    currentPos = ImVec2(circlePos.x + cosf(nextAngle) * radius,
                        circlePos.y + sinf(nextAngle) * radius);
  }

  // Stage progression

  stage_timer += io.DeltaTime;

  if (!done && stage_timer >= 1.5f) {

    stage_timer = 0.0f;

    current_stage++;

    if (current_stage >= total_stages) {

      done = true;
    }
  }

  // Final stage with enhanced effects

  if (done) {

    // Smoother alpha interpolation

    final_text_alpha = ImLerp(final_text_alpha, 1.0f, io.DeltaTime * 4.0f);

    const char *final_text;

    if (success) {

      final_text = "Success injection";
    } else if (adbError) {
      final_text = "ADB Failed";
    } else if (authError) {
      final_text = "Authentication Failed";
    } else {
      final_text = "Wrong injection";
    }
    ImColor final_color = success ? successGreen : errorRed;

    // Larger text size
    const float final_text_size = 18.0f;
    ImVec2 text_size =
        font->CalcTextSizeA(final_text_size, FLT_MAX, 0.0f, final_text);
    ImVec2 text_pos =
        ImVec2(circlePos.x - text_size.x * 0.5f, circlePos.y + radius + 35.0f);

    // Text glow effect

    for (int glow = 3; glow > 0; glow--) {

      float glow_alpha = (final_text_alpha / glow) * 0.3f;

      ImColor glowColor = utils::GetColorWithAlpha(final_color, glow_alpha);

      ImVec2 glow_offset = ImVec2((float)glow * 0.5f, (float)glow * 0.5f);

      dl->AddText(
          font, final_text_size,
          ImVec2(text_pos.x - glow_offset.x, text_pos.y - glow_offset.y),
          glowColor, final_text);

      dl->AddText(
          font, final_text_size,
          ImVec2(text_pos.x + glow_offset.x, text_pos.y + glow_offset.y),
          glowColor, final_text);
    }

    // Main text

    ImColor finalTextColor =
        utils::GetColorWithAlpha(final_color, final_text_alpha);

    dl->AddText(font, final_text_size, text_pos, finalTextColor, final_text);

    // Fade out with smoother easing

    fade_alpha = ImLerp(fade_alpha, 0.0f, io.DeltaTime * 3.0f);

    if (fade_alpha <= 0.01f) {

      // Reset

      fade_alpha = 1.0f;

      current_stage = 0;

      stage_timer = 0.0f;

      text_offset_y = 0.0f;

      text_alpha = 0.0f;

      done = false;

      final_text_alpha = 0.0f;

      time = 0.0f;
    }
  }

  // Current stage text with enhanced effects

  if (current_stage < total_stages && !done) {

    // Smoother progress with easing

    float raw_progress = stage_timer / 1.0f;

    float progress =
        (float)imanim::Easing::easeOutCubic(ImClamp(raw_progress, 0.0f, 1.0f));

    // Smoother animations

    text_offset_y = ImLerp(25.0f, 0.0f, progress);

    text_alpha = ImLerp(0.0f, 1.0f, progress) * fade_alpha;

    // Larger text size

    const float stage_text_size = 16.0f;

    ImVec2 text_size = font->CalcTextSizeA(stage_text_size, FLT_MAX, 0.0f,
                                           stages[current_stage]);

    ImVec2 text_pos = ImVec2(circlePos.x - text_size.x * 0.5f,
                             circlePos.y + radius + 30.0f + text_offset_y);

    // Text glow effect

    ImColor whiteColor = ImColor(255, 255, 255, 255);

    for (int glow = 2; glow > 0; glow--) {

      float glow_alpha = (text_alpha / glow) * 0.25f;

      ImColor glowColor = utils::GetColorWithAlpha(c::anim::active, glow_alpha);

      ImVec2 glow_offset = ImVec2((float)glow * 0.3f, (float)glow * 0.3f);

      dl->AddText(
          font, stage_text_size,
          ImVec2(text_pos.x - glow_offset.x, text_pos.y - glow_offset.y),
          glowColor, stages[current_stage]);
    }

    // Main text with slight blue tint

    ImColor stageTextColor = utils::GetColorWithAlpha(whiteColor, text_alpha);

    dl->AddText(font, stage_text_size, text_pos, stageTextColor,
                stages[current_stage]);
  }
}

DWORD WINAPI Unload() {

  adb::KillEmulatorAndAdbOnExit();

  if (g_pInterface) {

    g_pInterface->ShutDown();

    g_pInterface = nullptr;
  }

  if (MemoryUtils::ogPhysRead) {

    if (MH_DisableHook((LPVOID)MemoryUtils::ogPhysRead) != MH_OK) {

      std::cout << "Failed to disable PGMPhysRead hook!" << std::endl;
    }

    if (MH_RemoveHook((LPVOID)MemoryUtils::ogPhysRead) != MH_OK) {

      std::cout << "Failed to remove PGMPhysRead hook!" << std::endl;
    }
  }

  MH_Uninitialize();

  FreeConsole();

  fclose(stdout);

  bShouldUnload = true;

  if (g_hModule) {

    FreeLibraryAndExitThread(g_hModule, 0);
  }

  return 0;
}

bool MemoryInit = false;

void Memory() {

  auto vmm = GetModuleHandleA("BstkVMM.dll");

  if (vmm == nullptr) {

    return;
  }

  auto readFunc =
      (MemoryUtils::PGMPhysReadFunc)GetProcAddress(vmm, "PGMPhysRead");

  if (readFunc == nullptr) {

    return;
  }

  MH_Initialize();

  if (MH_CreateHook((LPVOID)readFunc, MemoryUtils::HookedPGMPhysRead,
                    (LPVOID *)&MemoryUtils::ogPhysRead) != MH_OK) {

    return;
  }

  if (MH_EnableHook((LPVOID)readFunc) != MH_OK) {

    return;
  }

  while (MemoryUtils::vmPtr == nullptr) {

    Sleep(10);
  }

  MemoryUtils::ogCPU =
      (MemoryUtils::VMMGetCpuByIdFunc)GetProcAddress(vmm, "VMMGetCpuById");

  if (MemoryUtils::ogCPU == nullptr) {

    return;
  }

  MemoryUtils::ogCast = (MemoryUtils::PGMPhysGCPtr2GCPhysFunc)GetProcAddress(
      vmm, "PGMPhysGCPtr2GCPhys");

  if (MemoryUtils::ogCast == nullptr) {

    return;
  }

  MemoryUtils::ogWrite =
      (MemoryUtils::PGMPhysSimpleWriteGCPhysFunc)GetProcAddress(
          vmm, "PGMPhysSimpleWriteGCPhys");

  if (MemoryUtils::ogWrite == nullptr) {

    return;
  }

  MemoryUtils::Initialize(MemoryUtils::vmPtr);

  std::cout << "Virt Memory: " << MemoryUtils::pVMAddr << std::endl;

  MemoryInit = true;
}

void adbInit() {

  TerminateAdbProcesses();

  ExecuteADBCommand("kill-server");

  ExecuteADBCommand("devices");

  bool GetDir = ChangeDirectory(GetExecutableDirectory());

  if (!GetDir) {
    g_AdbFailed = true;
    return;
  }

  std::string il2cppStr = ExecuteShellCommandNoSu(
      "cat /proc/$(pidof com.dts.freefireth)/maps | grep libil2cpp.so");

  if (il2cppStr.empty()) {
    g_AdbFailed = true;
    return;
  }

  Offsets::Il2Cpp = ConvertToUintPtr(il2cppStr);

  if (Offsets::Il2Cpp == 0) {
    g_AdbFailed = true;
    return;
  }

  g_AdbReady = true;
}

void authInit() {

  if (g_AuthStarted.exchange(true))
    return; // already started

  g_AuthOK = true;

  g_AuthDone = true;
}

namespace Cheat {

void Initialize() {

  Memory();

  if (!MemoryInit) {

    MessageBoxW(NULL, L"Error Initialize Memory", L"Error", NULL);
  }

  std::thread([&] { adbInit(); }).detach();

  std::thread([&] { authInit(); }).detach();

  FWork::Overlay::Setup(Render::FindRenderWindow());

  FWork::Overlay::Initialize();

  if (!FWork::Overlay::IsInitialized())
    return;

  if (!FWork::Overlay::dxGetDevice() || !FWork::Overlay::GetOverlayWindow())
    return;

  if (FWork::Overlay::IsInitialized())

  {

    FWork::Interface Interface(
        FWork::Overlay::GetOverlayWindow(), FWork::Overlay::GetTargetWindow(),
        FWork::Overlay::dxGetDevice(), FWork::Overlay::dxGetDeviceContext());

    g_pInterface = &Interface;

    FWork::Overlay::SetupWindowProcHook(std::bind(
        &FWork::Interface::WindowProc, &Interface, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

    MSG Message;

    ZeroMemory(&Message, sizeof(Message));

    while (Message.message != WM_QUIT)

    {

      HWND hWindow = FWork::Overlay::GetOverlayWindow();

      if (hWindow == nullptr) {

        std::cout << "[ERROR] Overlay window handle is nullptr" << std::endl;

        break;
      }

      if (PeekMessage(&Message, hWindow, NULL, NULL, PM_REMOVE))

      {

        TranslateMessage(&Message);

        DispatchMessage(&Message);
      }

      if (ImGui::GetCurrentContext()) {
        ImGui::GetIO().MouseDrawCursor = Interface.GetMenuOpen();
      }

      if (Interface.ResizeHeight != 0 || Interface.ResizeWidht != 0)

      {

        FWork::Overlay::dxCleanupRenderTarget();

        if (IDXGISwapChain* pSwapChain = FWork::Overlay::dxGetSwapChain()) {
          pSwapChain->ResizeBuffers(
              0, Interface.ResizeWidht, Interface.ResizeHeight,
              DXGI_FORMAT_UNKNOWN, 0);

          Interface.ResizeHeight = Interface.ResizeWidht = 0;

          FWork::Overlay::dxCreateRenderTarget();
        }
      }

      Interface.HandleMenuKey();

      // Startup loading overlay - show for 3 seconds after ADB is successful

      static bool startupInitialized = false;

      static std::chrono::steady_clock::time_point startupT0;

      static std::chrono::steady_clock::time_point adbSuccessTime;

      static bool adbSuccessTimeSet = false;

      static std::chrono::steady_clock::time_point errorTime;

      static bool errorTimeSet = false;

      static std::string emulatorInfo;

      if (!startupInitialized) {

        startupInitialized = true;

        startupT0 = std::chrono::steady_clock::now();

        emulatorInfo = DetectEmulatorInfo(FWork::Overlay::GetTargetWindow());
      }

      const auto now = std::chrono::steady_clock::now();

      bool adbError = g_AdbFailed.load();

      bool authError = g_AuthDone.load() && !g_AuthOK.load();

      bool hasError = adbError;

      // Track when error occurs (first time error is detected)

      if (hasError && !errorTimeSet) {

        errorTime = std::chrono::steady_clock::now();

        errorTimeSet = true;
      }

      // Check if ADB and Auth are both ready

      bool adbAndAuthReady =
          g_AdbReady.load() && g_AuthDone.load() && g_AuthOK.load();

      // Track when ADB becomes ready AND auth is successful (first time both
      // are true)

      if (adbAndAuthReady && !adbSuccessTimeSet && !hasError) {

        adbSuccessTime = std::chrono::steady_clock::now();

        adbSuccessTimeSet = true;
      }

      // Terminate HD-Adb 3 seconds after ADB success

      static bool hdAdbTerminated = false;

      if (adbSuccessTimeSet && !hdAdbTerminated && !hasError) {

        const float timeSinceReady =
            std::chrono::duration<float>(now - adbSuccessTime).count();

        if (timeSinceReady >= 3.0f) {

          {
            STARTUPINFOA si = {};
            PROCESS_INFORMATION pi_kill = {};
            si.cb = sizeof(si);
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            char hdadb_cmd[] = "taskkill /IM HD-Adb.exe /F";
            if (CreateProcessA(nullptr, hdadb_cmd, nullptr, nullptr, FALSE,
                               CREATE_NO_WINDOW, nullptr, nullptr, &si,
                               &pi_kill)) {
              WaitForSingleObject(pi_kill.hProcess, 3000);
              CloseHandle(pi_kill.hProcess);
              CloseHandle(pi_kill.hThread);
            }
          }

          hdAdbTerminated = true;
        }
      }

      bool showLoading = false;

      if (hasError) {

        // Error state - keep showing loading overlay with exit countdown

        showLoading = true;

      }

      else if (!adbAndAuthReady) {

        // ADB not ready OR auth not done OR auth failed - show loading

        showLoading = true;

      }

      else if (adbSuccessTimeSet) {

        // ADB is ready and auth is OK - show for 3 seconds after both are ready

        const float timeSinceReady =
            std::chrono::duration<float>(now - adbSuccessTime).count();

        showLoading = (timeSinceReady < 3.0f);

      }

      else {

        // Shouldn't reach here, but show loading as fallback

        showLoading = true;
      }

      // Always keep overlay same size as emulator (revert to old behavior)

      FWork::Overlay::UpdateWindowPos();

      ApplyEmulatorPerformanceMode();

      // Skip rendering and resizing if emulator is minimized or closed
      if (g_Globals.EspConfig.Width <= 0 || g_Globals.EspConfig.Height <= 0 || IsIconic(FWork::Overlay::GetTargetWindow())) {
          std::this_thread::sleep_for(std::chrono::milliseconds(20));
          continue;
      }

      static bool CaptureBypassOn = false;

      if (g_Globals.General.Capture != CaptureBypassOn)

      {

        CaptureBypassOn = g_Globals.General.Capture;

        SetWindowDisplayAffinity(FWork::Overlay::GetOverlayWindow(),
                                 CaptureBypassOn ? WDA_EXCLUDEFROMCAPTURE
                                                 : WDA_NONE);
      }

      ImGui_ImplDX11_NewFrame();

      ImGui_ImplWin32_NewFrame();

      if (!g_Globals.General.DisableAllEffects) {
        ImGui::GetIO().MouseWheel *=
            3.5f; // Boosts window list scroll speed to medium-fast
      }

      ImGui::NewFrame();

      {
        ImGuiIO& ioFrame = ImGui::GetIO();
        // Keep menus/animations stable on frame spikes (minimized restore, driver stalls).
        ioFrame.DeltaTime = ImMin(ioFrame.DeltaTime, 1.0f / 36.0f);
      }

      {

        if (MemoryInit) {
          FWork::Data::Work();
        }

        // World ESP draws on the background layer so it stays visible behind the menu.
        if (g_Globals.Visuals.Enabled) {
          ESP::Players();
        }
        if (g_Globals.Loot.Enabled) {
          ESP::Loot();
        }
        Interface.RenderGui();

        if (g_Globals.Misc.ShowAimbotFov && !g_Globals.General.DisableAllEffects) {

          ImColor Color = ImColor(g_Globals.Misc.AimbotFovColor[0],
                                  g_Globals.Misc.AimbotFovColor[1],
                                  g_Globals.Misc.AimbotFovColor[2],
                                  g_Globals.Misc.AimbotFovColor[3]);

          ImGui::GetBackgroundDrawList()->AddCircle(
              ImVec2(g_Globals.EspConfig.Width * 0.5f,
                     g_Globals.EspConfig.Height * 0.5f),
              g_Globals.AimBot.Fov, Color, 360);
        }

        // --- ESP OVERLAY: "Developed by GREJ" watermark (top-right corner) ---
        {
          ImDrawList* esp_dl = ImGui::GetForegroundDrawList();
          ImFont* dev_font = font::inter_semibold ? font::inter_semibold : ImGui::GetFont();
          const char* dev_label = "Developed by SamXRizz";
          const float dev_font_size = 13.0f;
          ImVec2 dev_sz = dev_font->CalcTextSizeA(dev_font_size, FLT_MAX, -1.0f, dev_label);
          float scr_w = ImGui::GetIO().DisplaySize.x;
          ImVec2 dev_pos = ImVec2(scr_w - dev_sz.x - 12.f, 10.f);
          // Shadow
          esp_dl->AddText(dev_font, dev_font_size,
              ImVec2(dev_pos.x + 1.f, dev_pos.y + 1.f),
              IM_COL32(0, 0, 0, 160), dev_label);
          // Main text — soft white with slight cyan tint
          esp_dl->AddText(dev_font, dev_font_size,
              dev_pos,
              IM_COL32(0, 202, 252, 210), dev_label);
        }
      }

      ImGui::EndFrame();

      ImGui::Render();

      FWork::Overlay::dxRefresh();

      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

      if (IDXGISwapChain* pSwapChain = FWork::Overlay::dxGetSwapChain()) {
        pSwapChain->Present(1, 0);
      }

      // Sleep 1ms to prevent high CPU usage (100% core usage) if Present fails to block or VSync is disabled
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

      // If error occurred, check if 10 seconds have passed - exit after
      // rendering countdown

      if (hasError && errorTimeSet) {

        const auto nowAfterRender = std::chrono::steady_clock::now();

        const float timeSinceError =
            std::chrono::duration<float>(nowAfterRender - errorTime).count();

        if (timeSinceError >= 10.0f) {
          // std::exit(0);
        }
      }

      if (g_Globals.General.ShutDown) {

        Unload();

        return;
      }
    }
  }
}

} // namespace Cheat

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                      LPWSTR lpCmdLine, int nCmdShow) {

#ifdef _DEBUG

  AllocConsole();

  SetConsoleOutputCP(CP_UTF8);

  SetConsoleCP(CP_UTF8);

  freopen("CONIN$", "r", stdin);

  freopen("CONOUT$", "w", stdout);

  freopen("CONOUT$", "w", stderr);

  SetConsoleTitleA("Debug Console");

#endif

  Cheat::Initialize();

  while (!bShouldUnload && !g_Globals.General.ShutDown) {

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

#ifdef _DEBUG

  FreeConsole();

  fclose(stdout);

#endif

  return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {

  switch (ul_reason_for_call) {

  case DLL_PROCESS_ATTACH:

    g_hModule = hModule;

    DisableThreadLibraryCalls(hModule);

    CreateThread(NULL, 0, [](LPVOID param) -> DWORD {
      Sleep(150);
      wWinMain((HINSTANCE)param, nullptr, nullptr, SW_SHOW);
      return 0;
    }, hModule, 0, NULL);

    break;

  case DLL_PROCESS_DETACH:

    FreeConsole();

    fclose(stdout);

    adb::KillEmulatorAndAdbOnExit();

    bShouldUnload = true;

    break;
  }

  return TRUE;
}
