#include "adb.hpp"
#include "ProcessUtils.hpp"
#include <TlHelp32.h>
#include <vector>

using namespace ProcessUtils;

namespace {

    void TerminateProcessesByName(const wchar_t* exeName) {
        if (!exeName || !*exeName)
            return;

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE)
            return;

        PROCESSENTRY32W pe{};
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(hSnap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, exeName) == 0) {
                    HANDLE hProc =
                        OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProc) {
                        TerminateProcess(hProc, 0);
                        CloseHandle(hProc);
                    }
                }
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }

    void HiddenTaskKill(const char* imageName) {
        if (!imageName || !*imageName)
            return;

        STARTUPINFOA si = {};
        PROCESS_INFORMATION pi = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        std::string cmd = std::string("taskkill /IM ") + imageName + " /F /T";
        std::vector<char> cmdLine(cmd.begin(), cmd.end());
        cmdLine.push_back('\0');

        if (CreateProcessA(nullptr, cmdLine.data(), nullptr, nullptr, FALSE,
                           CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 5000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

} // namespace

namespace adb {

    std::string GetExecutableDirectory() {
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);

        std::string fullPath(path);
        size_t lastSlashIndex = fullPath.find_last_of("\\/");
        if (lastSlashIndex != std::string::npos) {
            return fullPath.substr(0, lastSlashIndex);
        }
        return "";
    }

    std::string GetExecutableDirectoryPID(DWORD processID) {
        char path[MAX_PATH];
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

        if (hProcess) {
            if (GetModuleFileNameExA(hProcess, NULL, path, MAX_PATH)) {
                std::string fullPath(path);
                size_t lastSlashIndex = fullPath.find_last_of("\\/");
                CloseHandle(hProcess);
                if (lastSlashIndex != std::string::npos) {
                    return fullPath.substr(0, lastSlashIndex);
                }
            }
            CloseHandle(hProcess);
        }

        return "";
    }

    bool ChangeDirectory(const std::string& path) {
        return SetCurrentDirectoryA(path.c_str());
    }

    void TerminateAdbProcesses() {
        DWORD ProcIdAdb = GetProc(L"adb.exe");
        DWORD ProcIdAdb2 = 0;
        KillProc(ProcIdAdb);
        if (IsProcRun(L"adb.exe", ProcIdAdb2)) {
            ForgeKillProc(ProcIdAdb2);
        }

        DWORD ProcIdAdbHD = GetProc(L"HD-Adb.exe");
        DWORD ProcIdAdbHD2 = 0;
        KillProc(ProcIdAdbHD);
        if (IsProcRun(L"HD-Adb.exe", ProcIdAdbHD2)) {
            ForgeKillProc(ProcIdAdbHD2);
        }
    }

    void KillEmulatorAndAdbOnExit() {
        static const wchar_t* kEmulatorProcesses[] = {
            L"HD-Player.exe",
            L"HD-Adb.exe",
            L"HD-ADB.exe",
            L"adb.exe",
            L"BlueStacks.exe",
            L"Bluestacks.exe",
            L"BstkSVC.exe",
            L"BlueStacks_nxt.exe",
            L"MSIAppPlayer.exe",
            L"HD-Frontend.exe",
            L"HD-MultiInstanceManager.exe",
            L"HD-Agent.exe",
            L"HD-LogCollector.exe",
        };

        for (const wchar_t* exe : kEmulatorProcesses)
            TerminateProcessesByName(exe);

        TerminateAdbProcesses();

        HiddenTaskKill("HD-Adb.exe");
        HiddenTaskKill("HD-ADB.exe");
        HiddenTaskKill("adb.exe");
        HiddenTaskKill("HD-Player.exe");
        HiddenTaskKill("BlueStacks.exe");
        HiddenTaskKill("Bluestacks.exe");
        HiddenTaskKill("BstkSVC.exe");
        HiddenTaskKill("BlueStacks_nxt.exe");
        HiddenTaskKill("MSIAppPlayer.exe");
    }

    std::string ExtractLibAddress(const std::string& input) {
        std::size_t pos = input.find('-');
        if (pos != std::string::npos && pos >= 8) {
            return input.substr(pos - 8, 8);
        }
        return "";
    }

    uintptr_t ConvertToUintPtr(std::string str, int base) {
        if (sizeof(uintptr_t) == sizeof(unsigned long)) {
            return strtoul(str.c_str(), nullptr, base);
        }
        return strtoull(str.c_str(), nullptr, base);
    }

    std::string ExecuteShellCommand(const std::string& firstCommand, const std::string& secondCommand) {
        HANDLE hStdOutRead = NULL, hStdOutWrite = NULL;
        HANDLE hStdInRead = NULL, hStdInWrite = NULL;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
            std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
            return "";
        }

        if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
            std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            return "";
        }

        if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
            std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            return "";
        }

        if (!SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0)) {
            std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            CloseHandle(hStdInRead); CloseHandle(hStdInWrite);
            return "";
        }

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.hStdError = hStdOutWrite;
        si.hStdOutput = hStdOutWrite;
        si.hStdInput = hStdInRead;
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessA(NULL, (LPSTR)".\\HD-Adb shell", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            CloseHandle(hStdInRead); CloseHandle(hStdInWrite);
            return "";
        }

        CloseHandle(hStdOutWrite);
        CloseHandle(hStdInRead);

        DWORD written;
        std::string commands = firstCommand + "\n" + secondCommand + "\n";
        if (!WriteFile(hStdInWrite, commands.c_str(), commands.length(), &written, NULL)) {
            std::cerr << "WriteFile failed: " << GetLastError() << std::endl;
            CloseHandle(hStdInWrite);
            CloseHandle(hStdOutRead);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return "";
        }
        CloseHandle(hStdInWrite);

        CHAR buffer[128];
        DWORD read;
        std::string output;
        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
            buffer[read] = '\0';
            output += buffer;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hStdOutRead);

        return ExtractLibAddress(output);
    }

    std::string ExecuteShellCommandNoSu(const std::string& command) {
        HANDLE hStdOutRead = NULL, hStdOutWrite = NULL;
        HANDLE hStdInRead = NULL, hStdInWrite = NULL;

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
            std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
            return "";
        }

        if (!SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0)) {
            std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            return "";
        }

        if (!CreatePipe(&hStdInRead, &hStdInWrite, &sa, 0)) {
            std::cerr << "CreatePipe failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            return "";
        }

        if (!SetHandleInformation(hStdInWrite, HANDLE_FLAG_INHERIT, 0)) {
            std::cerr << "SetHandleInformation failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            CloseHandle(hStdInRead); CloseHandle(hStdInWrite);
            return "";
        }

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.hStdError = hStdOutWrite;
        si.hStdOutput = hStdOutWrite;
        si.hStdInput = hStdInRead;
        si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessA(NULL, (LPSTR)".\\HD-Adb shell \"getprop ro.secure ; /boot/android/android/system/xbin/bstk/su\"", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
            CloseHandle(hStdOutRead); CloseHandle(hStdOutWrite);
            CloseHandle(hStdInRead); CloseHandle(hStdInWrite);
            return "";
        }

        CloseHandle(hStdOutWrite);
        CloseHandle(hStdInRead);

        DWORD written;
        std::string commands = command + "\n";
        if (!WriteFile(hStdInWrite, commands.c_str(), commands.length(), &written, NULL)) {
            std::cerr << "WriteFile failed: " << GetLastError() << std::endl;
            CloseHandle(hStdInWrite);
            CloseHandle(hStdOutRead);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return "";
        }
        CloseHandle(hStdInWrite);

        CHAR buffer[128];
        DWORD read;
        std::string output;
        while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
            buffer[read] = '\0';
            output += buffer;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hStdOutRead);

        return ExtractLibAddress(output);
    }

    void ExecuteADBCommand(const std::string& command) {
        std::string fullCommand = ".\\HD-Adb shell " + command;
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.wShowWindow = SW_HIDE;
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcessA(NULL, const_cast<LPSTR>(fullCommand.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            std::cerr << "Failed to start process. Error: " << GetLastError() << std::endl;
            return;
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
