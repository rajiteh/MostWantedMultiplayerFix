#include "pch.h"

#include <Ws2tcpip.h>
#include "detours.h"

#include <iostream>
#include <fstream>
#include <vector>

class Logger {
private:
    std::ofstream log_file;
    bool debug_mode;
public:
    Logger(const std::string& file_name, bool debug): debug_mode(debug) {
        if (!debug_mode) 
        {
            return;
        }

        log_file.open(file_name.c_str(), std::ios_base::out | std::ios_base::app);
        if (!log_file) {
            std::cerr << "Failed to open log file\n";
            exit(1);
        }
    }

    ~Logger() {
        if (log_file) {
            log_file.close();
        }
    }

    void log(const char* fmt, ...) {
        if (!debug_mode) {
            return;
        }

        va_list args;
        va_start(args, fmt);

        // We draw the line at 256 characters
        std::vector<char> buf(256);
        vsnprintf(buf.data(), buf.size(), fmt, args);
        log_file << buf.data() << std::endl << std::flush;

        va_end(args);
    }
};

bool debug_flag = false;  // Set this to true to enable logging, false to disable it
static Logger logger("mostwantedmultiplayerfix.log", debug_flag);


static int (WSAAPI* TrueBind)(SOCKET, const struct sockaddr*, int) = bind;

static int WSAAPI CustomBind(SOCKET s, const struct sockaddr* name, int namelen)
{
    WCHAR ipstr[INET6_ADDRSTRLEN];
    int port;
    struct sockaddr_storage storage;
    struct sockaddr* addr_any = (struct sockaddr*)&storage;

    // Cast the sockaddr to sockaddr_in or sockaddr_in6, depending on the IP version
    if (name->sa_family == AF_INET) { // IPv4
        struct sockaddr_in* addr_in = (struct sockaddr_in*)name;
        port = ntohs(addr_in->sin_port);
        InetNtop(AF_INET, &(addr_in->sin_addr), ipstr, sizeof(ipstr));

        // Create a new sockaddr_in that listens on all interfaces
        struct sockaddr_in* addr_in_any = (struct sockaddr_in*)addr_any;
        addr_in_any->sin_family = AF_INET;
        addr_in_any->sin_port = addr_in->sin_port;
        addr_in_any->sin_addr.s_addr = INADDR_ANY;
    }
    else if (name->sa_family == AF_INET6) { // IPv6
        struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)name;
        port = ntohs(addr_in6->sin6_port);
        InetNtop(AF_INET6, &(addr_in6->sin6_addr), ipstr, sizeof(ipstr));

        // Create a new sockaddr_in6 that listens on all interfaces
        struct sockaddr_in6* addr_in6_any = (struct sockaddr_in6*)addr_any;
        addr_in6_any->sin6_family = AF_INET6;
        addr_in6_any->sin6_port = addr_in6->sin6_port;
        addr_in6_any->sin6_addr = in6addr_any;
    }
    else {
        // Unsupported address family
        return SOCKET_ERROR;
    }

    logger.log("Original IP address: %ls\n", ipstr);
    logger.log("Port: %d\n", port);

    // Call the original bind function with the new sockaddr that listens on all interfaces
    return TrueBind(s, addr_any, namelen);
}

// DllMain function attaches and detaches the TimedSleep detour to the
// Sleep target function.  The Sleep target function is referred to
// through the TrueSleep target pointer.
//
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{   
    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueBind, CustomBind);
        LONG error = DetourTransactionCommit();
        if (error == NO_ERROR) {
            logger.log("Attached.");
            return TRUE;
        }
        else {
            logger.log("detrour transaction commit failed");
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueBind, CustomBind);
        DetourTransactionCommit();
    }
    return TRUE;
}

