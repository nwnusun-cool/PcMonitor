#include "connections.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <psapi.h>
#include <vector>
#include <string>
#include <unordered_map>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "psapi.lib")

// Helper function
std::string WideToUtf8Conn(const wchar_t* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
    return result;
}

// Get process name from PID
std::string GetProcessNameFromPID(DWORD pid) {
    if (pid == 0) return "System Idle Process";
    if (pid == 4) return "System";
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return "";
    
    wchar_t path[MAX_PATH] = {0};
    DWORD size = MAX_PATH;
    if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
        CloseHandle(hProcess);
        // Extract filename from path
        wchar_t* filename = wcsrchr(path, L'\\');
        if (filename) return WideToUtf8Conn(filename + 1);
        return WideToUtf8Conn(path);
    }
    
    CloseHandle(hProcess);
    return "";
}

// Cache for process names
static std::unordered_map<DWORD, std::string> processNameCache;

napi_value GetNetworkConnections(napi_env env, napi_callback_info info) {
    napi_value result, tcpConns, udpConns;
    napi_create_object(env, &result);
    napi_create_array(env, &tcpConns);
    napi_create_array(env, &udpConns);
    
    // Clear old cache entries (keep it small)
    if (processNameCache.size() > 500) processNameCache.clear();
    
    // Get TCP connections
    ULONG tcpSize = 0;
    GetExtendedTcpTable(nullptr, &tcpSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    
    if (tcpSize > 0) {
        std::vector<BYTE> tcpBuffer(tcpSize);
        auto tcpTable = (PMIB_TCPTABLE_OWNER_PID)tcpBuffer.data();
        
        if (GetExtendedTcpTable(tcpTable, &tcpSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
            uint32_t tcpIdx = 0;
            
            for (DWORD i = 0; i < tcpTable->dwNumEntries; i++) {
                auto& row = tcpTable->table[i];
                
                // Skip closed connections
                if (row.dwState == MIB_TCP_STATE_CLOSED || row.dwState == MIB_TCP_STATE_DELETE_TCB) {
                    continue;
                }
                
                napi_value conn;
                napi_create_object(env, &conn);
                napi_value v;
                
                // Protocol
                napi_create_string_utf8(env, "TCP", NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "protocol", v);
                
                // Local address
                char localAddr[16];
                sprintf(localAddr, "%d.%d.%d.%d",
                    row.dwLocalAddr & 0xFF,
                    (row.dwLocalAddr >> 8) & 0xFF,
                    (row.dwLocalAddr >> 16) & 0xFF,
                    (row.dwLocalAddr >> 24) & 0xFF);
                napi_create_string_utf8(env, localAddr, NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "localAddress", v);
                
                // Local port
                napi_create_uint32(env, ntohs((u_short)row.dwLocalPort), &v);
                napi_set_named_property(env, conn, "localPort", v);
                
                // Remote address
                char remoteAddr[16];
                sprintf(remoteAddr, "%d.%d.%d.%d",
                    row.dwRemoteAddr & 0xFF,
                    (row.dwRemoteAddr >> 8) & 0xFF,
                    (row.dwRemoteAddr >> 16) & 0xFF,
                    (row.dwRemoteAddr >> 24) & 0xFF);
                napi_create_string_utf8(env, remoteAddr, NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "remoteAddress", v);
                
                // Remote port
                napi_create_uint32(env, ntohs((u_short)row.dwRemotePort), &v);
                napi_set_named_property(env, conn, "remotePort", v);
                
                // State
                const char* state = "UNKNOWN";
                switch (row.dwState) {
                    case MIB_TCP_STATE_LISTEN: state = "LISTENING"; break;
                    case MIB_TCP_STATE_ESTAB: state = "ESTABLISHED"; break;
                    case MIB_TCP_STATE_SYN_SENT: state = "SYN_SENT"; break;
                    case MIB_TCP_STATE_SYN_RCVD: state = "SYN_RECEIVED"; break;
                    case MIB_TCP_STATE_FIN_WAIT1: state = "FIN_WAIT_1"; break;
                    case MIB_TCP_STATE_FIN_WAIT2: state = "FIN_WAIT_2"; break;
                    case MIB_TCP_STATE_CLOSE_WAIT: state = "CLOSE_WAIT"; break;
                    case MIB_TCP_STATE_CLOSING: state = "CLOSING"; break;
                    case MIB_TCP_STATE_LAST_ACK: state = "LAST_ACK"; break;
                    case MIB_TCP_STATE_TIME_WAIT: state = "TIME_WAIT"; break;
                }
                napi_create_string_utf8(env, state, NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "state", v);
                
                // PID
                napi_create_uint32(env, row.dwOwningPid, &v);
                napi_set_named_property(env, conn, "pid", v);
                
                // Process name (cached)
                std::string processName;
                auto it = processNameCache.find(row.dwOwningPid);
                if (it != processNameCache.end()) {
                    processName = it->second;
                } else {
                    processName = GetProcessNameFromPID(row.dwOwningPid);
                    processNameCache[row.dwOwningPid] = processName;
                }
                napi_create_string_utf8(env, processName.c_str(), NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "process", v);
                
                napi_set_element(env, tcpConns, tcpIdx++, conn);
            }
        }
    }
    
    // Get UDP connections
    ULONG udpSize = 0;
    GetExtendedUdpTable(nullptr, &udpSize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0);
    
    if (udpSize > 0) {
        std::vector<BYTE> udpBuffer(udpSize);
        auto udpTable = (PMIB_UDPTABLE_OWNER_PID)udpBuffer.data();
        
        if (GetExtendedUdpTable(udpTable, &udpSize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0) == NO_ERROR) {
            uint32_t udpIdx = 0;
            
            for (DWORD i = 0; i < udpTable->dwNumEntries; i++) {
                auto& row = udpTable->table[i];
                
                napi_value conn;
                napi_create_object(env, &conn);
                napi_value v;
                
                // Protocol
                napi_create_string_utf8(env, "UDP", NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "protocol", v);
                
                // Local address
                char localAddr[16];
                sprintf(localAddr, "%d.%d.%d.%d",
                    row.dwLocalAddr & 0xFF,
                    (row.dwLocalAddr >> 8) & 0xFF,
                    (row.dwLocalAddr >> 16) & 0xFF,
                    (row.dwLocalAddr >> 24) & 0xFF);
                napi_create_string_utf8(env, localAddr, NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "localAddress", v);
                
                // Local port
                napi_create_uint32(env, ntohs((u_short)row.dwLocalPort), &v);
                napi_set_named_property(env, conn, "localPort", v);
                
                // UDP has no remote address/port
                napi_create_string_utf8(env, "*", NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "remoteAddress", v);
                napi_create_uint32(env, 0, &v);
                napi_set_named_property(env, conn, "remotePort", v);
                
                // State (UDP is always listening)
                napi_create_string_utf8(env, "LISTENING", NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "state", v);
                
                // PID
                napi_create_uint32(env, row.dwOwningPid, &v);
                napi_set_named_property(env, conn, "pid", v);
                
                // Process name (cached)
                std::string processName;
                auto it = processNameCache.find(row.dwOwningPid);
                if (it != processNameCache.end()) {
                    processName = it->second;
                } else {
                    processName = GetProcessNameFromPID(row.dwOwningPid);
                    processNameCache[row.dwOwningPid] = processName;
                }
                napi_create_string_utf8(env, processName.c_str(), NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, conn, "process", v);
                
                napi_set_element(env, udpConns, udpIdx++, conn);
            }
        }
    }
    
    napi_set_named_property(env, result, "tcp", tcpConns);
    napi_set_named_property(env, result, "udp", udpConns);
    
    return result;
}
