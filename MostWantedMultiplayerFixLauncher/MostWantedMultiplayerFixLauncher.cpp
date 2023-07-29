#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <iostream>
#include <conio.h>

// Get the process ID from the process name
DWORD GetProcessIdByName(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = { 0 };
    entry.dwSize = sizeof(entry);
    
    wchar_t* processNameW = new wchar_t[strlen(processName) + 1];
    mbstowcs_s(NULL, processNameW, strlen(processName) + 1, processName, strlen(processName));

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (wcscmp(entry.szExeFile, processNameW) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0; // not found
}

bool createdProcess = false;

void cleanupProcess(HANDLE t, HANDLE p, void* remoteDllPath, bool terminate) {
   

    if (t != NULL) {
        CloseHandle(t);
    }

    if (p != NULL) {
        if (remoteDllPath != NULL) {
            VirtualFreeEx(p, remoteDllPath, 0, MEM_RELEASE);
        }

        if (createdProcess && terminate) {
            TerminateProcess(p, 1);
        }
        CloseHandle(p);
    }
}

int run() {
    const char* dllName = "MostWantedMultiplayerFix.dll";
    const char* exeName = "speed.exe";

    // Verify if DLL and EXE exist in current working directory
    if (GetFileAttributesA(dllName) == INVALID_FILE_ATTRIBUTES) {
        printf("DLL not found: %s\n", dllName);
        return 1;
    }

    if (GetFileAttributesA(exeName) == INVALID_FILE_ATTRIBUTES) {
        printf("EXE not found: %s\n", exeName);
        return 1;
    }


    // Get the process ID
    DWORD processId = GetProcessIdByName(exeName);
    HANDLE process = NULL;
    HANDLE thread = NULL;

    if (processId != 0) {

        // Open the target process
        process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (process == NULL) {
            std::cerr << "Error: Unable to open running process. Error code: " << GetLastError() << std::endl;
            return 1;
        }
        std::cerr << "INFO: Attaching to PID " << process << std::endl;
    }
    else {
        // Set up the STARTUPINFO and PROCESS_INFORMATION structures
        STARTUPINFOA si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;

        // Start the target process in a suspended state
        if (!CreateProcessA(exeName, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
            std::cerr << "Error: Unable to create process. Error code: " << GetLastError() << std::endl;
            return 1;
        }

        process = pi.hProcess;
        thread = pi.hThread;

        createdProcess = true;
        std::cerr << "INFO: Spawning new process " << exeName << std::endl;
    }
    
    // Allocate memory in the target process for the DLL path
    void* remoteDllPath = VirtualAllocEx(process, NULL, strlen(dllName) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (remoteDllPath == NULL) {
        std::cerr << "Error: Unable to allocate memory for process. Error code: " << GetLastError() << std::endl;
        cleanupProcess(process, thread, remoteDllPath, true);
        return 1;
    }

    // Write the DLL path to the allocated memory
    if (!WriteProcessMemory(process, remoteDllPath, dllName, strlen(dllName) + 1, NULL)) {
        std::cerr << "Error: Unable to write to process memory. Error code: " << GetLastError() << std::endl;
        cleanupProcess(process, thread, remoteDllPath, true);
        return 1;
    }

    
    HMODULE kernel32handle = GetModuleHandleA("kernel32.dll");
    if (kernel32handle == 0) {
        std::cerr << "Error: Unable to get module handle. Error code: " << GetLastError() << std::endl;
        cleanupProcess(process, thread, remoteDllPath, true);
        return 1;
    }

    LPTHREAD_START_ROUTINE procAddress = (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32handle, "LoadLibraryA");
    if (procAddress == 0) {
        std::cerr << "Error: Unable to get proc address of LoadLibraryA. Error code: " << GetLastError() << std::endl;
        cleanupProcess(process, thread, remoteDllPath, true);
        return 1;
    }

    HANDLE rthread = CreateRemoteThread(process, NULL, 0, procAddress, remoteDllPath, 0, NULL);
    if (rthread == NULL) {
        std::cerr << "Error: Unable to create remote thread. Error code: " << GetLastError() << std::endl;
        cleanupProcess(process, thread, remoteDllPath, true);
        return 1;
    }
    CloseHandle(rthread);

    if (createdProcess) {
        ResumeThread(thread);
    }

    cleanupProcess(process, thread, remoteDllPath, false);

    return 0;
}


int main() {
    int code = run();
    if (code != 0) {
        std::cerr << "Press any key to continue..." << std::endl;
        _getch();
    }    
    return code;
}