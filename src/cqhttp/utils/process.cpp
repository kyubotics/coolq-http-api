#include "./process.h"

#include <process.h>
#include <tlhelp32.h>

namespace cqhttp::utils::process {
    int get_pid() { return _getpid(); }

    int get_parent_pid() {
        // see https://gist.github.com/mattn/253013/d47b90159cf8ffa4d92448614b748aa1d235ebe4
        HANDLE hSnapshot;
        PROCESSENTRY32 pe32;
        DWORD ppid = 0, pid = GetCurrentProcessId();

        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        __try {
            if (hSnapshot == INVALID_HANDLE_VALUE) __leave;

            ZeroMemory(&pe32, sizeof(pe32));
            pe32.dwSize = sizeof(pe32);
            if (!Process32First(hSnapshot, &pe32)) __leave;

            do {
                if (pe32.th32ProcessID == pid) {
                    ppid = pe32.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));

        } __finally {
            if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
        }
        return ppid;
    }
} // namespace cqhttp::utils::process
