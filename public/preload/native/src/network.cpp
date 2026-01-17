#include "network.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <vector>
#include <string>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

// Helper function
std::string WideToUtf8Network(const wchar_t* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
    return result;
}

// Network cache for speed calculation
struct NetCache { DWORD idx; ULONGLONG rx, tx; };
static std::vector<NetCache> netCache;
static ULONGLONG netTime = 0;

napi_value GetNetworkStats(napi_env env, napi_callback_info info) {
    napi_value result, ifaces;
    napi_create_object(env, &result);
    napi_create_array(env, &ifaces);
    
    ULONG bufLen = 15000;
    PIP_ADAPTER_ADDRESSES addrs = (PIP_ADAPTER_ADDRESSES)malloc(bufLen);
    if (!addrs) {
        napi_set_named_property(env, result, "interfaces", ifaces);
        return result;
    }
    
    // Get adapter addresses with more info (including DNS)
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_INCLUDE_PREFIX;
    if (GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addrs, &bufLen) == ERROR_BUFFER_OVERFLOW) {
        free(addrs);
        addrs = (PIP_ADAPTER_ADDRESSES)malloc(bufLen);
        if (!addrs) {
            napi_set_named_property(env, result, "interfaces", ifaces);
            return result;
        }
    }
    
    if (GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addrs, &bufLen) != NO_ERROR) {
        free(addrs);
        napi_set_named_property(env, result, "interfaces", ifaces);
        return result;
    }
    
    ULONGLONG now = GetTickCount64();
    double dt = netTime > 0 ? (now - netTime) / 1000.0 : 1.0;
    if (dt < 0.1) dt = 1.0;
    
    std::vector<NetCache> newCache;
    uint32_t idx = 0;
    
    for (auto a = addrs; a; a = a->Next) {
        // Skip non-physical adapters
        if (a->IfType != IF_TYPE_ETHERNET_CSMACD && a->IfType != IF_TYPE_IEEE80211) continue;
        if (a->OperStatus != IfOperStatusUp) continue;
        
        MIB_IF_ROW2 row = {0};
        row.InterfaceIndex = a->IfIndex;
        if (GetIfEntry2(&row) != NO_ERROR) continue;
        
        napi_value iface;
        napi_create_object(env, &iface);
        napi_value v;
        
        // Interface name
        napi_create_string_utf8(env, WideToUtf8Network(a->FriendlyName).c_str(), NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, iface, "iface", v);
        
        // Connection type
        const char* type = (a->IfType == IF_TYPE_IEEE80211) ? "wireless" : "wired";
        napi_create_string_utf8(env, type, NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, iface, "type", v);
        
        // IPv4 and IPv6 addresses
        std::string ip4, ip6, subnet;
        for (auto ua = a->FirstUnicastAddress; ua; ua = ua->Next) {
            if (ua->Address.lpSockaddr->sa_family == AF_INET) {
                auto sa = (sockaddr_in*)ua->Address.lpSockaddr;
                char buf[16];
                sprintf(buf, "%d.%d.%d.%d", 
                    sa->sin_addr.S_un.S_un_b.s_b1, sa->sin_addr.S_un.S_un_b.s_b2,
                    sa->sin_addr.S_un.S_un_b.s_b3, sa->sin_addr.S_un.S_un_b.s_b4);
                ip4 = buf;
                
                // Calculate subnet mask from prefix length
                ULONG mask = 0xFFFFFFFF << (32 - ua->OnLinkPrefixLength);
                sprintf(buf, "%d.%d.%d.%d",
                    (mask >> 24) & 0xFF, (mask >> 16) & 0xFF,
                    (mask >> 8) & 0xFF, mask & 0xFF);
                subnet = buf;
            } else if (ua->Address.lpSockaddr->sa_family == AF_INET6 && ip6.empty()) {
                auto sa = (sockaddr_in6*)ua->Address.lpSockaddr;
                char buf[46];
                inet_ntop(AF_INET6, &sa->sin6_addr, buf, sizeof(buf));
                ip6 = buf;
            }
        }
        
        napi_create_string_utf8(env, ip4.c_str(), NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, iface, "ip4", v);
        
        napi_create_string_utf8(env, ip6.c_str(), NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, iface, "ip6", v);
        
        napi_create_string_utf8(env, subnet.c_str(), NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, iface, "subnet", v);
        
        // DNS servers
        napi_value dnsArray;
        napi_create_array(env, &dnsArray);
        uint32_t dnsIdx = 0;
        for (auto dns = a->FirstDnsServerAddress; dns && dnsIdx < 2; dns = dns->Next) {
            char buf[46] = {0};
            if (dns->Address.lpSockaddr->sa_family == AF_INET) {
                auto sa = (sockaddr_in*)dns->Address.lpSockaddr;
                sprintf(buf, "%d.%d.%d.%d",
                    sa->sin_addr.S_un.S_un_b.s_b1, sa->sin_addr.S_un.S_un_b.s_b2,
                    sa->sin_addr.S_un.S_un_b.s_b3, sa->sin_addr.S_un.S_un_b.s_b4);
            } else if (dns->Address.lpSockaddr->sa_family == AF_INET6) {
                auto sa = (sockaddr_in6*)dns->Address.lpSockaddr;
                inet_ntop(AF_INET6, &sa->sin6_addr, buf, sizeof(buf));
            }
            if (buf[0]) {
                napi_create_string_utf8(env, buf, NAPI_AUTO_LENGTH, &v);
                napi_set_element(env, dnsArray, dnsIdx++, v);
            }
        }
        napi_set_named_property(env, iface, "dns", dnsArray);
        
        // DHCP enabled
        napi_get_boolean(env, a->Dhcpv4Enabled, &v);
        napi_set_named_property(env, iface, "dhcp", v);
        
        // MAC address
        char mac[18] = {0};
        if (a->PhysicalAddressLength == 6) {
            sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
                a->PhysicalAddress[0], a->PhysicalAddress[1], a->PhysicalAddress[2],
                a->PhysicalAddress[3], a->PhysicalAddress[4], a->PhysicalAddress[5]);
        }
        napi_create_string_utf8(env, mac, NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, iface, "mac", v);
        
        // Traffic statistics
        ULONGLONG rx = row.InOctets, tx = row.OutOctets;
        napi_create_double(env, (double)rx, &v);
        napi_set_named_property(env, iface, "rxBytes", v);
        napi_create_double(env, (double)tx, &v);
        napi_set_named_property(env, iface, "txBytes", v);
        
        // Packet statistics
        napi_create_double(env, (double)row.InUcastPkts, &v);
        napi_set_named_property(env, iface, "rxPackets", v);
        napi_create_double(env, (double)row.OutUcastPkts, &v);
        napi_set_named_property(env, iface, "txPackets", v);
        
        // Speed calculation
        double rxSec = 0, txSec = 0;
        for (auto& c : netCache) {
            if (c.idx == a->IfIndex) {
                rxSec = (rx - c.rx) / dt;
                txSec = (tx - c.tx) / dt;
                break;
            }
        }
        if (rxSec < 0) rxSec = 0;
        if (txSec < 0) txSec = 0;
        
        napi_create_double(env, rxSec, &v);
        napi_set_named_property(env, iface, "rxSec", v);
        napi_create_double(env, txSec, &v);
        napi_set_named_property(env, iface, "txSec", v);
        
        // Link speed (Mbps)
        double linkSpeed = (double)a->TransmitLinkSpeed / 1e6;
        napi_create_double(env, linkSpeed, &v);
        napi_set_named_property(env, iface, "speed", v);
        
        // Network utilization (%)
        double maxSpeed = linkSpeed * 1e6 / 8; // Convert Mbps to bytes/sec
        double utilization = 0;
        if (maxSpeed > 0) {
            double currentSpeed = (rxSec > txSec) ? rxSec : txSec;
            utilization = (currentSpeed / maxSpeed) * 100.0;
            if (utilization > 100) utilization = 100;
        }
        napi_create_double(env, utilization, &v);
        napi_set_named_property(env, iface, "utilization", v);
        
        newCache.push_back({ a->IfIndex, rx, tx });
        napi_set_element(env, ifaces, idx++, iface);
    }
    
    free(addrs);
    netCache = newCache;
    netTime = now;
    
    napi_set_named_property(env, result, "interfaces", ifaces);
    return result;
}
