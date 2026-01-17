#define NAPI_VERSION 8
#include <node_api.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <winioctl.h>
#include <pdh.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <vector>
#include <string>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "wbemuuid.lib")

std::string WideToUtf8(const wchar_t* wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
    return result;
}

// Memory Info (extended with GetPerformanceInfo)
napi_value GetMemoryInfo(napi_env env, napi_callback_info info) {
    napi_value result;
    napi_create_object(env, &result);
    napi_value v;
    
    MEMORYSTATUSEX mem; mem.dwLength = sizeof(mem);
    if (GlobalMemoryStatusEx(&mem)) {
        napi_create_double(env, (double)mem.ullTotalPhys, &v); napi_set_named_property(env, result, "total", v);
        napi_create_double(env, (double)mem.ullAvailPhys, &v); napi_set_named_property(env, result, "free", v);
        napi_create_double(env, (double)(mem.ullTotalPhys - mem.ullAvailPhys), &v); napi_set_named_property(env, result, "used", v);
        napi_create_double(env, (double)mem.dwMemoryLoad, &v); napi_set_named_property(env, result, "usedPercent", v);
        // Swap (PageFile - Physical = Swap)
        double swapTotal = (double)(mem.ullTotalPageFile - mem.ullTotalPhys);
        double swapFree = (double)(mem.ullAvailPageFile - mem.ullAvailPhys);
        if (swapTotal < 0) swapTotal = 0;
        if (swapFree < 0) swapFree = 0;
        napi_create_double(env, swapTotal, &v); napi_set_named_property(env, result, "swapTotal", v);
        napi_create_double(env, swapTotal - swapFree, &v); napi_set_named_property(env, result, "swapUsed", v);
        napi_create_double(env, swapFree, &v); napi_set_named_property(env, result, "swapFree", v);
    }
    
    // GetPerformanceInfo for detailed memory composition
    PERFORMANCE_INFORMATION pi; pi.cb = sizeof(pi);
    if (GetPerformanceInfo(&pi, sizeof(pi))) {
        SIZE_T pageSize = pi.PageSize;
        // Committed memory (total commit charge)
        napi_create_double(env, (double)(pi.CommitTotal * pageSize), &v); 
        napi_set_named_property(env, result, "committed", v);
        napi_create_double(env, (double)(pi.CommitLimit * pageSize), &v); 
        napi_set_named_property(env, result, "commitLimit", v);
        // Cache (system cache)
        napi_create_double(env, (double)(pi.SystemCache * pageSize), &v); 
        napi_set_named_property(env, result, "cached", v);
        // Paged pool
        napi_create_double(env, (double)(pi.KernelPaged * pageSize), &v); 
        napi_set_named_property(env, result, "pagedPool", v);
        // Non-paged pool
        napi_create_double(env, (double)(pi.KernelNonpaged * pageSize), &v); 
        napi_set_named_property(env, result, "nonPagedPool", v);
        // Page size
        napi_create_double(env, (double)pageSize, &v); 
        napi_set_named_property(env, result, "pageSize", v);
    }
    
    return result;
}

// CPU Usage - using PDH % Processor Utility (matches Task Manager on modern CPUs)
static PDH_HQUERY cpuQuery = NULL;
static PDH_HCOUNTER cpuCounter = NULL;
static bool cpuInit = false;
static double lastCpuLoad = 0;

// Per-Core CPU Usage
static PDH_HQUERY perCoreQuery = NULL;
static std::vector<PDH_HCOUNTER> perCoreCounters;
static bool perCoreInit = false;
static std::vector<double> lastPerCoreLoad;

napi_value GetCpuUsage(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_object(env, &result);
    
    if (!cpuInit) {
        if (PdhOpenQuery(NULL, 0, &cpuQuery) == ERROR_SUCCESS) {
            // % Processor Utility is what Task Manager uses on modern CPUs
            if (PdhAddEnglishCounterW(cpuQuery, L"\\Processor Information(_Total)\\% Processor Utility", 0, &cpuCounter) != ERROR_SUCCESS) {
                // Fallback to % Processor Time for older systems
                PdhAddEnglishCounterW(cpuQuery, L"\\Processor(_Total)\\% Processor Time", 0, &cpuCounter);
            }
            PdhCollectQueryData(cpuQuery); // First call to initialize
        }
        cpuInit = true;
    }
    
    double load = lastCpuLoad;
    
    if (cpuQuery && cpuCounter) {
        if (PdhCollectQueryData(cpuQuery) == ERROR_SUCCESS) {
            PDH_FMT_COUNTERVALUE value;
            if (PdhGetFormattedCounterValue(cpuCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                load = value.doubleValue;
                if (load < 0) load = 0;
                if (load > 100) load = 100;
                lastCpuLoad = load;
            }
        }
    }
    
    napi_value v;
    napi_create_double(env, load, &v); napi_set_named_property(env, result, "load", v);
    return result;
}

// Per-Core CPU Usage
napi_value GetPerCoreUsage(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_array(env, &result);
    
    SYSTEM_INFO si; GetNativeSystemInfo(&si);
    int numCores = si.dwNumberOfProcessors;
    
    if (!perCoreInit) {
        if (PdhOpenQuery(NULL, 0, &perCoreQuery) == ERROR_SUCCESS) {
            perCoreCounters.resize(numCores);
            lastPerCoreLoad.resize(numCores, 0);
            
            for (int i = 0; i < numCores; i++) {
                wchar_t path[256];
                // Try Processor Information first (modern, matches Task Manager)
                swprintf(path, 256, L"\\Processor Information(0,%d)\\%% Processor Utility", i);
                if (PdhAddEnglishCounterW(perCoreQuery, path, 0, &perCoreCounters[i]) != ERROR_SUCCESS) {
                    // Fallback to old Processor counter
                    swprintf(path, 256, L"\\Processor(%d)\\%% Processor Time", i);
                    PdhAddEnglishCounterW(perCoreQuery, path, 0, &perCoreCounters[i]);
                }
            }
            PdhCollectQueryData(perCoreQuery); // First call to initialize
        }
        perCoreInit = true;
    }
    
    if (perCoreQuery) {
        if (PdhCollectQueryData(perCoreQuery) == ERROR_SUCCESS) {
            for (int i = 0; i < numCores; i++) {
                if (perCoreCounters[i]) {
                    PDH_FMT_COUNTERVALUE value;
                    if (PdhGetFormattedCounterValue(perCoreCounters[i], PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                        double load = value.doubleValue;
                        if (load < 0) load = 0;
                        if (load > 100) load = 100;
                        lastPerCoreLoad[i] = load;
                    }
                }
            }
        }
    }
    
    for (int i = 0; i < numCores; i++) {
        napi_value v;
        napi_create_double(env, lastPerCoreLoad[i], &v);
        napi_set_element(env, result, i, v);
    }
    
    return result;
}

// Uptime
napi_value GetUptime(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_object(env, &result);
    napi_value v; napi_create_double(env, (double)(GetTickCount64() / 1000), &v);
    napi_set_named_property(env, result, "seconds", v);
    return result;
}

// System Stats (process count, thread count, handle count)
napi_value GetSystemStats(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_object(env, &result);
    napi_value v;
    
    DWORD processCount = 0, threadCount = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        // Count processes
        PROCESSENTRY32W pe; pe.dwSize = sizeof(pe);
        if (Process32FirstW(snap, &pe)) {
            do { processCount++; } while (Process32NextW(snap, &pe));
        }
        // Count threads
        THREADENTRY32 te; te.dwSize = sizeof(te);
        if (Thread32First(snap, &te)) {
            do { threadCount++; } while (Thread32Next(snap, &te));
        }
        CloseHandle(snap);
    }
    
    // Get system handle count using PDH (same as Task Manager)
    DWORD handleCount = 0;
    PDH_HQUERY hQuery = NULL;
    PDH_HCOUNTER hCounter = NULL;
    if (PdhOpenQuery(NULL, 0, &hQuery) == ERROR_SUCCESS) {
        // Try different counter paths
        const wchar_t* counterPaths[] = {
            L"\\Process(_Total)\\Handle Count",
            L"\\System\\Handle Count"
        };
        for (auto path : counterPaths) {
            if (PdhAddEnglishCounterW(hQuery, path, 0, &hCounter) == ERROR_SUCCESS) {
                PdhCollectQueryData(hQuery);
                Sleep(10);
                if (PdhCollectQueryData(hQuery) == ERROR_SUCCESS) {
                    PDH_FMT_COUNTERVALUE value;
                    if (PdhGetFormattedCounterValue(hCounter, PDH_FMT_LONG, NULL, &value) == ERROR_SUCCESS) {
                        if (value.longValue > 0) {
                            handleCount = value.longValue;
                            break;
                        }
                    }
                }
                PdhRemoveCounter(hCounter);
                hCounter = NULL;
            }
        }
        PdhCloseQuery(hQuery);
    }
    
    napi_create_uint32(env, processCount, &v); napi_set_named_property(env, result, "processCount", v);
    napi_create_uint32(env, threadCount, &v); napi_set_named_property(env, result, "threadCount", v);
    napi_create_uint32(env, handleCount, &v); napi_set_named_property(env, result, "handleCount", v);
    return result;
}

// System Info
napi_value GetSystemInfo(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_object(env, &result);
    wchar_t hostname[256] = {0}; DWORD size = 256;
    GetComputerNameW(hostname, &size);
    napi_value v;
    napi_create_string_utf8(env, WideToUtf8(hostname).c_str(), NAPI_AUTO_LENGTH, &v);
    napi_set_named_property(env, result, "hostname", v);
    
    OSVERSIONINFOEXW osvi = {0}; osvi.dwOSVersionInfoSize = sizeof(osvi);
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll) { auto fn = (RtlGetVersionPtr)GetProcAddress(ntdll, "RtlGetVersion"); if (fn) fn((PRTL_OSVERSIONINFOW)&osvi); }
    
    const char* platform = osvi.dwBuildNumber >= 22000 ? "Windows 11" : "Windows 10";
    napi_create_string_utf8(env, platform, NAPI_AUTO_LENGTH, &v); napi_set_named_property(env, result, "platform", v);
    napi_create_uint32(env, osvi.dwBuildNumber, &v); napi_set_named_property(env, result, "build", v);
    
    SYSTEM_INFO si; GetNativeSystemInfo(&si);
    napi_create_string_utf8(env, si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "x64" : "x86", NAPI_AUTO_LENGTH, &v);
    napi_set_named_property(env, result, "arch", v);
    
    // 设备厂商和型号 (从注册表获取)
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\BIOS", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t manufacturer[256] = {0}; DWORD mfSize = sizeof(manufacturer);
        if (RegQueryValueExW(hKey, L"SystemManufacturer", nullptr, nullptr, (LPBYTE)manufacturer, &mfSize) == ERROR_SUCCESS) {
            napi_create_string_utf8(env, WideToUtf8(manufacturer).c_str(), NAPI_AUTO_LENGTH, &v);
            napi_set_named_property(env, result, "manufacturer", v);
        }
        wchar_t model[256] = {0}; DWORD mdSize = sizeof(model);
        if (RegQueryValueExW(hKey, L"SystemProductName", nullptr, nullptr, (LPBYTE)model, &mdSize) == ERROR_SUCCESS) {
            napi_create_string_utf8(env, WideToUtf8(model).c_str(), NAPI_AUTO_LENGTH, &v);
            napi_set_named_property(env, result, "model", v);
        }
        RegCloseKey(hKey);
    }
    
    return result;
}

// CPU Info
napi_value GetCpuInfo(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_object(env, &result);
    SYSTEM_INFO si; GetNativeSystemInfo(&si);
    napi_value v;
    
    // 逻辑核心数
    napi_create_uint32(env, si.dwNumberOfProcessors, &v);
    napi_set_named_property(env, result, "cores", v);
    
    // 物理核心数 (通过 GetLogicalProcessorInformation)
    DWORD len = 0;
    GetLogicalProcessorInformation(nullptr, &len);
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(len / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    int physicalCores = 0;
    if (GetLogicalProcessorInformation(buffer.data(), &len)) {
        for (auto& info : buffer) {
            if (info.Relationship == RelationProcessorCore) physicalCores++;
        }
    }
    if (physicalCores == 0) physicalCores = si.dwNumberOfProcessors / 2;
    napi_create_uint32(env, physicalCores, &v);
    napi_set_named_property(env, result, "physicalCores", v);
    
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t brand[256] = {0}; DWORD brandSize = sizeof(brand);
        if (RegQueryValueExW(hKey, L"ProcessorNameString", nullptr, nullptr, (LPBYTE)brand, &brandSize) == ERROR_SUCCESS) {
            napi_create_string_utf8(env, WideToUtf8(brand).c_str(), NAPI_AUTO_LENGTH, &v);
            napi_set_named_property(env, result, "brand", v);
        }
        DWORD mhz = 0, mhzSize = sizeof(mhz);
        if (RegQueryValueExW(hKey, L"~MHz", nullptr, nullptr, (LPBYTE)&mhz, &mhzSize) == ERROR_SUCCESS) {
            napi_create_double(env, mhz / 1000.0, &v); napi_set_named_property(env, result, "speed", v);
            napi_create_double(env, mhz / 1000.0, &v); napi_set_named_property(env, result, "currentSpeed", v);
        }
        
        // Check virtualization support
        wchar_t identifier[256] = {0}; DWORD idSize = sizeof(identifier);
        if (RegQueryValueExW(hKey, L"Identifier", nullptr, nullptr, (LPBYTE)identifier, &idSize) == ERROR_SUCCESS) {
            std::string id = WideToUtf8(identifier);
            bool hasVirt = (id.find("AMD") != std::string::npos || id.find("Intel") != std::string::npos);
            napi_create_string_utf8(env, hasVirt ? "Supported" : "Unknown", NAPI_AUTO_LENGTH, &v);
            napi_set_named_property(env, result, "virtualization", v);
        }
        RegCloseKey(hKey);
    }
    return result;
}


// GPU Info
napi_value GetGpuInfo(napi_env env, napi_callback_info info) {
    napi_value result, gpus, displays; 
    napi_create_object(env, &result); 
    napi_create_array(env, &gpus);
    napi_create_array(env, &displays);
    
    // 从注册表获取显卡信息（包含显存）
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        uint32_t gpuIdx = 0;
        for (int i = 0; i < 10; i++) {
            wchar_t subKey[16]; swprintf(subKey, 16, L"%04d", i);
            HKEY hSubKey;
            if (RegOpenKeyExW(hKey, subKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                napi_value gpu; napi_create_object(env, &gpu);
                napi_value v;
                
                // 显卡描述
                wchar_t desc[256] = {0}; DWORD descSize = sizeof(desc);
                if (RegQueryValueExW(hSubKey, L"DriverDesc", nullptr, nullptr, (LPBYTE)desc, &descSize) == ERROR_SUCCESS) {
                    napi_create_string_utf8(env, WideToUtf8(desc).c_str(), NAPI_AUTO_LENGTH, &v);
                    napi_set_named_property(env, gpu, "model", v);
                    
                    // 厂商
                    std::string model = WideToUtf8(desc);
                    const char* vendor = model.find("NVIDIA") != std::string::npos ? "NVIDIA" :
                                        model.find("AMD") != std::string::npos ? "AMD" :
                                        model.find("Intel") != std::string::npos ? "Intel" : "Unknown";
                    napi_create_string_utf8(env, vendor, NAPI_AUTO_LENGTH, &v);
                    napi_set_named_property(env, gpu, "vendor", v);
                }
                
                // 显存大小 (qwMemorySize 是 QWORD)
                ULONGLONG vram = 0; DWORD vramSize = sizeof(vram);
                if (RegQueryValueExW(hSubKey, L"HardwareInformation.qwMemorySize", nullptr, nullptr, (LPBYTE)&vram, &vramSize) == ERROR_SUCCESS) {
                    napi_create_double(env, (double)vram, &v);
                    napi_set_named_property(env, gpu, "vram", v);
                } else {
                    // 尝试 DWORD 版本
                    DWORD vram32 = 0; DWORD vram32Size = sizeof(vram32);
                    if (RegQueryValueExW(hSubKey, L"HardwareInformation.MemorySize", nullptr, nullptr, (LPBYTE)&vram32, &vram32Size) == ERROR_SUCCESS) {
                        napi_create_double(env, (double)vram32, &v);
                        napi_set_named_property(env, gpu, "vram", v);
                    }
                }
                
                // 总线类型
                napi_create_string_utf8(env, "PCI", NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, gpu, "bus", v);
                
                napi_set_element(env, gpus, gpuIdx++, gpu);
                RegCloseKey(hSubKey);
            }
        }
        RegCloseKey(hKey);
    }
    
    // Display info
    uint32_t dispIdx = 0;
    DISPLAY_DEVICEW dd = {0}; dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); i++) {
        if (!(dd.StateFlags & DISPLAY_DEVICE_ACTIVE)) continue;
        
        DEVMODEW dm = {0}; dm.dmSize = sizeof(dm);
        if (EnumDisplaySettingsW(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
            napi_value disp; napi_create_object(env, &disp);
            napi_value v;
            
            // Display name
            DISPLAY_DEVICEW mon = {0}; mon.cb = sizeof(mon);
            if (EnumDisplayDevicesW(dd.DeviceName, 0, &mon, 0)) {
                napi_create_string_utf8(env, WideToUtf8(mon.DeviceString).c_str(), NAPI_AUTO_LENGTH, &v);
            } else {
                napi_create_string_utf8(env, "Monitor", NAPI_AUTO_LENGTH, &v);
            }
            napi_set_named_property(env, disp, "model", v);
            
            // Resolution
            napi_create_uint32(env, dm.dmPelsWidth, &v); napi_set_named_property(env, disp, "resolutionX", v);
            napi_create_uint32(env, dm.dmPelsHeight, &v); napi_set_named_property(env, disp, "resolutionY", v);
            napi_create_uint32(env, dm.dmPelsWidth, &v); napi_set_named_property(env, disp, "currentResX", v);
            napi_create_uint32(env, dm.dmPelsHeight, &v); napi_set_named_property(env, disp, "currentResY", v);
            
            // Refresh rate
            napi_create_uint32(env, dm.dmDisplayFrequency, &v); napi_set_named_property(env, disp, "refreshRate", v);
            
            // Pixel depth
            napi_create_uint32(env, dm.dmBitsPerPel, &v); napi_set_named_property(env, disp, "pixelDepth", v);
            
            // Is primary
            napi_get_boolean(env, (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0, &v);
            napi_set_named_property(env, disp, "main", v);
            
            napi_set_element(env, displays, dispIdx++, disp);
        }
    }
    
    napi_set_named_property(env, result, "controllers", gpus);
    napi_set_named_property(env, result, "displays", displays);
    return result;
}

// Battery Info
napi_value GetBatteryInfo(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_object(env, &result);
    SYSTEM_POWER_STATUS sps; napi_value v;
    if (GetSystemPowerStatus(&sps)) {
        napi_get_boolean(env, sps.BatteryFlag != 128 && sps.BatteryFlag != 255, &v);
        napi_set_named_property(env, result, "hasBattery", v);
        napi_create_uint32(env, sps.BatteryLifePercent, &v); napi_set_named_property(env, result, "percent", v);
        napi_get_boolean(env, sps.ACLineStatus == 1, &v); napi_set_named_property(env, result, "isCharging", v);
    } else {
        napi_get_boolean(env, false, &v); napi_set_named_property(env, result, "hasBattery", v);
    }
    return result;
}

// Disk Info
napi_value GetDiskInfo(napi_env env, napi_callback_info info) {
    napi_value result, partitions, physical; 
    napi_create_object(env, &result); 
    napi_create_array(env, &partitions);
    napi_create_array(env, &physical);
    
    // 分区信息
    DWORD drives = GetLogicalDrives();
    uint32_t idx = 0;
    for (char letter = 'A'; letter <= 'Z'; letter++) {
        if (!(drives & (1 << (letter - 'A')))) continue;
        char root[4] = { letter, ':', '\\', 0 };
        if (GetDriveTypeA(root) != DRIVE_FIXED) continue;
        ULARGE_INTEGER freeAvail, total, totalFree;
        if (!GetDiskFreeSpaceExA(root, &freeAvail, &total, &totalFree)) continue;
        
        napi_value disk; napi_create_object(env, &disk);
        napi_value v;
        char fs[3] = { letter, ':', 0 };
        napi_create_string_utf8(env, root, NAPI_AUTO_LENGTH, &v); napi_set_named_property(env, disk, "mount", v);
        napi_create_string_utf8(env, fs, NAPI_AUTO_LENGTH, &v); napi_set_named_property(env, disk, "fs", v);
        napi_create_double(env, (double)total.QuadPart, &v); napi_set_named_property(env, disk, "size", v);
        napi_create_double(env, (double)totalFree.QuadPart, &v); napi_set_named_property(env, disk, "free", v);
        napi_create_double(env, (double)(total.QuadPart - totalFree.QuadPart), &v); napi_set_named_property(env, disk, "used", v);
        double pct = total.QuadPart > 0 ? (double)(total.QuadPart - totalFree.QuadPart) / total.QuadPart * 100.0 : 0;
        napi_create_double(env, pct, &v); napi_set_named_property(env, disk, "usedPercent", v);
        
        // 获取文件系统类型
        char fsType[32] = {0};
        GetVolumeInformationA(root, nullptr, 0, nullptr, nullptr, nullptr, fsType, sizeof(fsType));
        napi_create_string_utf8(env, fsType[0] ? fsType : "NTFS", NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, disk, "type", v);
        
        napi_set_element(env, partitions, idx++, disk);
    }
    napi_set_named_property(env, result, "partitions", partitions);
    
    // 物理磁盘信息
    uint32_t pIdx = 0;
    for (int i = 0; i < 16; i++) {
        char path[32]; sprintf(path, "\\\\.\\PhysicalDrive%d", i);
        HANDLE hDisk = CreateFileA(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hDisk == INVALID_HANDLE_VALUE) continue;
        
        STORAGE_PROPERTY_QUERY query = {StorageDeviceProperty, PropertyStandardQuery};
        char buffer[1024] = {0};
        DWORD bytesReturned;
        
        if (DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer, sizeof(buffer), &bytesReturned, nullptr)) {
            STORAGE_DEVICE_DESCRIPTOR* desc = (STORAGE_DEVICE_DESCRIPTOR*)buffer;
            napi_value disk; napi_create_object(env, &disk);
            napi_value v;
            
            // 型号
            if (desc->ProductIdOffset) {
                std::string model = (char*)buffer + desc->ProductIdOffset;
                // 去除首尾空格
                size_t start = model.find_first_not_of(" ");
                size_t end = model.find_last_not_of(" ");
                if (start != std::string::npos) model = model.substr(start, end - start + 1);
                napi_create_string_utf8(env, model.c_str(), NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, disk, "name", v);
            }
            
            // 厂商
            if (desc->VendorIdOffset) {
                std::string vendor = (char*)buffer + desc->VendorIdOffset;
                size_t start = vendor.find_first_not_of(" ");
                size_t end = vendor.find_last_not_of(" ");
                if (start != std::string::npos) vendor = vendor.substr(start, end - start + 1);
                napi_create_string_utf8(env, vendor.c_str(), NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, disk, "vendor", v);
            }
            
            // 类型 (SSD/HDD)
            const char* type = desc->BusType == BusTypeSata ? "SATA" : 
                              desc->BusType == BusTypeNvme ? "NVMe" : "Unknown";
            napi_create_string_utf8(env, type, NAPI_AUTO_LENGTH, &v);
            napi_set_named_property(env, disk, "interfaceType", v);
            
            // 大小
            DISK_GEOMETRY_EX geo;
            if (DeviceIoControl(hDisk, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, &geo, sizeof(geo), &bytesReturned, nullptr)) {
                napi_create_double(env, (double)geo.DiskSize.QuadPart, &v);
                napi_set_named_property(env, disk, "size", v);
            }
            
            napi_set_element(env, physical, pIdx++, disk);
        }
        CloseHandle(hDisk);
    }
    napi_set_named_property(env, result, "physical", physical);
    
    return result;
}

// Disk IO Stats (read/write bytes per second)
static ULONGLONG prevDiskReadBytes = 0, prevDiskWriteBytes = 0;
static ULONGLONG prevDiskTime = 0;

napi_value GetDiskIO(napi_env env, napi_callback_info info) {
    napi_value result; napi_create_object(env, &result);
    napi_value v;
    
    // Use PDH to get disk IO and performance metrics
    static PDH_HQUERY diskQuery = NULL;
    static PDH_HCOUNTER readCounter = NULL, writeCounter = NULL;
    static PDH_HCOUNTER diskTimeCounter = NULL, queueLengthCounter = NULL;
    static PDH_HCOUNTER avgReadTimeCounter = NULL, avgWriteTimeCounter = NULL;
    static PDH_HCOUNTER readsPerSecCounter = NULL, writesPerSecCounter = NULL;
    static bool diskIoInit = false;
    
    if (!diskIoInit) {
        if (PdhOpenQuery(NULL, 0, &diskQuery) == ERROR_SUCCESS) {
            // IO throughput
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\Disk Read Bytes/sec", 0, &readCounter);
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\Disk Write Bytes/sec", 0, &writeCounter);
            // Performance metrics
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\% Disk Time", 0, &diskTimeCounter);
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\Current Disk Queue Length", 0, &queueLengthCounter);
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\Avg. Disk sec/Read", 0, &avgReadTimeCounter);
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\Avg. Disk sec/Write", 0, &avgWriteTimeCounter);
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\Disk Reads/sec", 0, &readsPerSecCounter);
            PdhAddEnglishCounterW(diskQuery, L"\\PhysicalDisk(_Total)\\Disk Writes/sec", 0, &writesPerSecCounter);
            PdhCollectQueryData(diskQuery);
        }
        diskIoInit = true;
    }
    
    double readSec = 0, writeSec = 0;
    double diskTime = 0, queueLength = 0;
    double avgReadTime = 0, avgWriteTime = 0;
    double readsPerSec = 0, writesPerSec = 0;
    
    if (diskQuery) {
        if (PdhCollectQueryData(diskQuery) == ERROR_SUCCESS) {
            PDH_FMT_COUNTERVALUE value;
            
            // IO throughput
            if (readCounter && PdhGetFormattedCounterValue(readCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                readSec = value.doubleValue;
            }
            if (writeCounter && PdhGetFormattedCounterValue(writeCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                writeSec = value.doubleValue;
            }
            
            // Performance metrics
            if (diskTimeCounter && PdhGetFormattedCounterValue(diskTimeCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                diskTime = value.doubleValue;
            }
            if (queueLengthCounter && PdhGetFormattedCounterValue(queueLengthCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                queueLength = value.doubleValue;
            }
            if (avgReadTimeCounter && PdhGetFormattedCounterValue(avgReadTimeCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                avgReadTime = value.doubleValue * 1000; // Convert to ms
            }
            if (avgWriteTimeCounter && PdhGetFormattedCounterValue(avgWriteTimeCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                avgWriteTime = value.doubleValue * 1000; // Convert to ms
            }
            if (readsPerSecCounter && PdhGetFormattedCounterValue(readsPerSecCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                readsPerSec = value.doubleValue;
            }
            if (writesPerSecCounter && PdhGetFormattedCounterValue(writesPerSecCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
                writesPerSec = value.doubleValue;
            }
        }
    }
    
    // Clamp values
    if (readSec < 0) readSec = 0;
    if (writeSec < 0) writeSec = 0;
    if (diskTime < 0) diskTime = 0;
    if (diskTime > 100) diskTime = 100;
    if (queueLength < 0) queueLength = 0;
    if (avgReadTime < 0) avgReadTime = 0;
    if (avgWriteTime < 0) avgWriteTime = 0;
    if (readsPerSec < 0) readsPerSec = 0;
    if (writesPerSec < 0) writesPerSec = 0;
    
    // IO throughput
    napi_create_double(env, readSec, &v); napi_set_named_property(env, result, "readSec", v);
    napi_create_double(env, writeSec, &v); napi_set_named_property(env, result, "writeSec", v);
    // Performance metrics
    napi_create_double(env, diskTime, &v); napi_set_named_property(env, result, "activeTime", v);
    napi_create_double(env, queueLength, &v); napi_set_named_property(env, result, "queueLength", v);
    napi_create_double(env, avgReadTime, &v); napi_set_named_property(env, result, "avgReadTime", v);
    napi_create_double(env, avgWriteTime, &v); napi_set_named_property(env, result, "avgWriteTime", v);
    napi_create_double(env, readsPerSec, &v); napi_set_named_property(env, result, "readsPerSec", v);
    napi_create_double(env, writesPerSec, &v); napi_set_named_property(env, result, "writesPerSec", v);
    
    return result;
}

// Network Stats
struct NetCache { DWORD idx; ULONGLONG rx, tx; };
static std::vector<NetCache> netCache;
static ULONGLONG netTime = 0;

napi_value GetNetworkStats(napi_env env, napi_callback_info info) {
    napi_value result, ifaces; napi_create_object(env, &result); napi_create_array(env, &ifaces);
    ULONG bufLen = 15000;
    PIP_ADAPTER_ADDRESSES addrs = (PIP_ADAPTER_ADDRESSES)malloc(bufLen);
    if (!addrs) { napi_set_named_property(env, result, "interfaces", ifaces); return result; }
    
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
    if (GetAdaptersAddresses(AF_INET, flags, nullptr, addrs, &bufLen) == ERROR_BUFFER_OVERFLOW) {
        free(addrs); addrs = (PIP_ADAPTER_ADDRESSES)malloc(bufLen);
        if (!addrs) { napi_set_named_property(env, result, "interfaces", ifaces); return result; }
    }
    if (GetAdaptersAddresses(AF_INET, flags, nullptr, addrs, &bufLen) != NO_ERROR) {
        free(addrs); napi_set_named_property(env, result, "interfaces", ifaces); return result;
    }
    
    ULONGLONG now = GetTickCount64();
    double dt = netTime > 0 ? (now - netTime) / 1000.0 : 1.0;
    if (dt < 0.1) dt = 1.0;
    std::vector<NetCache> newCache;
    uint32_t idx = 0;
    
    for (auto a = addrs; a; a = a->Next) {
        if (a->IfType != IF_TYPE_ETHERNET_CSMACD && a->IfType != IF_TYPE_IEEE80211) continue;
        if (a->OperStatus != IfOperStatusUp) continue;
        MIB_IF_ROW2 row = {0}; row.InterfaceIndex = a->IfIndex;
        if (GetIfEntry2(&row) != NO_ERROR) continue;
        
        napi_value iface; napi_create_object(env, &iface);
        napi_value v;
        napi_create_string_utf8(env, WideToUtf8(a->FriendlyName).c_str(), NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, iface, "iface", v);
        
        std::string ip4;
        for (auto ua = a->FirstUnicastAddress; ua; ua = ua->Next) {
            if (ua->Address.lpSockaddr->sa_family == AF_INET) {
                auto sa = (sockaddr_in*)ua->Address.lpSockaddr;
                char buf[16]; sprintf(buf, "%d.%d.%d.%d", sa->sin_addr.S_un.S_un_b.s_b1, sa->sin_addr.S_un.S_un_b.s_b2,
                    sa->sin_addr.S_un.S_un_b.s_b3, sa->sin_addr.S_un.S_un_b.s_b4);
                ip4 = buf; break;
            }
        }
        napi_create_string_utf8(env, ip4.c_str(), NAPI_AUTO_LENGTH, &v); napi_set_named_property(env, iface, "ip4", v);
        
        char mac[18] = {0};
        if (a->PhysicalAddressLength == 6) sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
            a->PhysicalAddress[0], a->PhysicalAddress[1], a->PhysicalAddress[2],
            a->PhysicalAddress[3], a->PhysicalAddress[4], a->PhysicalAddress[5]);
        napi_create_string_utf8(env, mac, NAPI_AUTO_LENGTH, &v); napi_set_named_property(env, iface, "mac", v);
        
        ULONGLONG rx = row.InOctets, tx = row.OutOctets;
        napi_create_double(env, (double)rx, &v); napi_set_named_property(env, iface, "rxBytes", v);
        napi_create_double(env, (double)tx, &v); napi_set_named_property(env, iface, "txBytes", v);
        
        double rxSec = 0, txSec = 0;
        for (auto& c : netCache) if (c.idx == a->IfIndex) { rxSec = (rx - c.rx) / dt; txSec = (tx - c.tx) / dt; break; }
        if (rxSec < 0) rxSec = 0; if (txSec < 0) txSec = 0;
        napi_create_double(env, rxSec, &v); napi_set_named_property(env, iface, "rxSec", v);
        napi_create_double(env, txSec, &v); napi_set_named_property(env, iface, "txSec", v);
        napi_create_double(env, (double)a->TransmitLinkSpeed / 1e6, &v); napi_set_named_property(env, iface, "speed", v);
        
        newCache.push_back({ a->IfIndex, rx, tx });
        napi_set_element(env, ifaces, idx++, iface);
    }
    free(addrs); netCache = newCache; netTime = now;
    napi_set_named_property(env, result, "interfaces", ifaces);
    return result;
}


// Process List with detailed info
struct ProcessCpuCache { DWORD pid; ULONGLONG kernelTime, userTime; };
static std::vector<ProcessCpuCache> procCpuCache;
static ULONGLONG procCpuTime = 0;

napi_value GetProcessList(napi_env env, napi_callback_info info) {
    napi_value result, procs; napi_create_object(env, &result); napi_create_array(env, &procs);
    
    // Get system times for CPU calculation
    FILETIME idleTime, kernelTime, userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    ULONGLONG sysKernel = ((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
    ULONGLONG sysUser = ((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;
    ULONGLONG sysTotal = sysKernel + sysUser;
    ULONGLONG now = GetTickCount64();
    double dt = procCpuTime > 0 ? (now - procCpuTime) / 1000.0 : 1.0;
    if (dt < 0.1) dt = 1.0;
    
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        napi_set_named_property(env, result, "processes", procs);
        napi_value v; napi_create_uint32(env, 0, &v); napi_set_named_property(env, result, "count", v);
        return result;
    }
    
    PROCESSENTRY32W pe; pe.dwSize = sizeof(pe);
    struct ProcInfo { DWORD pid; std::wstring name; DWORD threads; SIZE_T memory; DWORD handles; double cpu; };
    std::vector<ProcInfo> list;
    std::vector<ProcessCpuCache> newCpuCache;
    
    if (Process32FirstW(snap, &pe)) do {
        if (pe.th32ProcessID == 0) continue;
        
        ProcInfo pi = { pe.th32ProcessID, pe.szExeFile, pe.cntThreads, 0, 0, 0 };
        
        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
        if (h) {
            // Memory
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc))) {
                pi.memory = pmc.WorkingSetSize;
            }
            
            // Handle count
            DWORD hc = 0;
            GetProcessHandleCount(h, &hc);
            pi.handles = hc;
            
            // CPU usage
            FILETIME createTime, exitTime, procKernel, procUser;
            if (GetProcessTimes(h, &createTime, &exitTime, &procKernel, &procUser)) {
                ULONGLONG pKernel = ((ULONGLONG)procKernel.dwHighDateTime << 32) | procKernel.dwLowDateTime;
                ULONGLONG pUser = ((ULONGLONG)procUser.dwHighDateTime << 32) | procUser.dwLowDateTime;
                
                // Find previous values
                for (auto& c : procCpuCache) {
                    if (c.pid == pe.th32ProcessID) {
                        ULONGLONG kDiff = pKernel - c.kernelTime;
                        ULONGLONG uDiff = pUser - c.userTime;
                        // CPU % = (process time diff) / (elapsed time * num cores) * 100
                        SYSTEM_INFO si; GetNativeSystemInfo(&si);
                        double elapsed = dt * 10000000.0; // Convert to 100ns units
                        pi.cpu = ((double)(kDiff + uDiff) / (elapsed * si.dwNumberOfProcessors)) * 100.0;
                        if (pi.cpu < 0) pi.cpu = 0;
                        if (pi.cpu > 100) pi.cpu = 100;
                        break;
                    }
                }
                newCpuCache.push_back({ pe.th32ProcessID, pKernel, pUser });
            }
            CloseHandle(h);
        }
        
        list.push_back(pi);
    } while (Process32NextW(snap, &pe));
    CloseHandle(snap);
    
    procCpuCache = newCpuCache;
    procCpuTime = now;
    
    uint32_t idx = 0;
    for (auto& p : list) {
        napi_value proc; napi_create_object(env, &proc);
        napi_value v;
        napi_create_uint32(env, p.pid, &v); napi_set_named_property(env, proc, "pid", v);
        napi_create_string_utf8(env, WideToUtf8(p.name.c_str()).c_str(), NAPI_AUTO_LENGTH, &v);
        napi_set_named_property(env, proc, "name", v);
        napi_create_double(env, (double)p.memory, &v); napi_set_named_property(env, proc, "memory", v);
        napi_create_uint32(env, p.threads, &v); napi_set_named_property(env, proc, "threads", v);
        napi_create_uint32(env, p.handles, &v); napi_set_named_property(env, proc, "handles", v);
        napi_create_double(env, p.cpu, &v); napi_set_named_property(env, proc, "cpu", v);
        napi_set_element(env, procs, idx++, proc);
    }
    
    napi_set_named_property(env, result, "processes", procs);
    napi_value v; napi_create_uint32(env, (uint32_t)list.size(), &v); napi_set_named_property(env, result, "count", v);
    return result;
}

// Memory Hardware Info via WMI
napi_value GetMemoryHardware(napi_env env, napi_callback_info info) {
    napi_value result, modules;
    napi_create_object(env, &result);
    napi_create_array(env, &modules);
    
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    bool needUninit = SUCCEEDED(hr);
    
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;
    IEnumWbemClassObject* pEnum = NULL;
    
    do {
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
        if (FAILED(hr)) break;
        
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
        if (FAILED(hr)) break;
        
        hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
        if (FAILED(hr)) break;
        
        // Query physical memory
        hr = pSvc->ExecQuery(_bstr_t(L"WQL"), 
            _bstr_t(L"SELECT BankLabel, Capacity, Speed, SMBIOSMemoryType, FormFactor, Manufacturer, PartNumber FROM Win32_PhysicalMemory"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum);
        if (FAILED(hr)) break;
        
        IWbemClassObject* pObj = NULL;
        ULONG uReturn = 0;
        uint32_t idx = 0;
        
        while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK && uReturn > 0) {
            napi_value mod;
            napi_create_object(env, &mod);
            napi_value v;
            VARIANT vtProp;
            
            // BankLabel
            if (SUCCEEDED(pObj->Get(L"BankLabel", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR) {
                napi_create_string_utf8(env, WideToUtf8(vtProp.bstrVal).c_str(), NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, mod, "bank", v);
                VariantClear(&vtProp);
            }
            
            // Capacity
            if (SUCCEEDED(pObj->Get(L"Capacity", 0, &vtProp, 0, 0))) {
                ULONGLONG cap = 0;
                if (vtProp.vt == VT_BSTR) cap = _wtoi64(vtProp.bstrVal);
                else if (vtProp.vt == VT_I8 || vtProp.vt == VT_UI8) cap = vtProp.ullVal;
                napi_create_double(env, (double)cap, &v);
                napi_set_named_property(env, mod, "capacity", v);
                VariantClear(&vtProp);
            }
            
            // Speed
            if (SUCCEEDED(pObj->Get(L"Speed", 0, &vtProp, 0, 0)) && (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4)) {
                napi_create_uint32(env, vtProp.uintVal, &v);
                napi_set_named_property(env, mod, "speed", v);
                VariantClear(&vtProp);
            }
            
            // SMBIOSMemoryType (26=DDR4, 34=DDR5, 24=DDR3)
            if (SUCCEEDED(pObj->Get(L"SMBIOSMemoryType", 0, &vtProp, 0, 0)) && (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4)) {
                const char* type = "Unknown";
                switch (vtProp.uintVal) {
                    case 24: type = "DDR3"; break;
                    case 26: type = "DDR4"; break;
                    case 34: type = "DDR5"; break;
                }
                napi_create_string_utf8(env, type, NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, mod, "type", v);
                VariantClear(&vtProp);
            }
            
            // FormFactor (8=DIMM, 12=SODIMM)
            if (SUCCEEDED(pObj->Get(L"FormFactor", 0, &vtProp, 0, 0)) && (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4)) {
                const char* form = "Unknown";
                switch (vtProp.uintVal) {
                    case 8: form = "DIMM"; break;
                    case 12: form = "SODIMM"; break;
                }
                napi_create_string_utf8(env, form, NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, mod, "formFactor", v);
                VariantClear(&vtProp);
            }
            
            // Manufacturer
            if (SUCCEEDED(pObj->Get(L"Manufacturer", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR) {
                std::string mfr = WideToUtf8(vtProp.bstrVal);
                // Trim whitespace
                size_t start = mfr.find_first_not_of(" \t");
                size_t end = mfr.find_last_not_of(" \t");
                if (start != std::string::npos) mfr = mfr.substr(start, end - start + 1);
                napi_create_string_utf8(env, mfr.c_str(), NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, mod, "manufacturer", v);
                VariantClear(&vtProp);
            }
            
            // PartNumber
            if (SUCCEEDED(pObj->Get(L"PartNumber", 0, &vtProp, 0, 0)) && vtProp.vt == VT_BSTR) {
                std::string pn = WideToUtf8(vtProp.bstrVal);
                size_t start = pn.find_first_not_of(" \t");
                size_t end = pn.find_last_not_of(" \t");
                if (start != std::string::npos) pn = pn.substr(start, end - start + 1);
                napi_create_string_utf8(env, pn.c_str(), NAPI_AUTO_LENGTH, &v);
                napi_set_named_property(env, mod, "partNumber", v);
                VariantClear(&vtProp);
            }
            
            napi_set_element(env, modules, idx++, mod);
            pObj->Release();
        }
        
        // Get total slots from Win32_PhysicalMemoryArray
        IEnumWbemClassObject* pEnum2 = NULL;
        hr = pSvc->ExecQuery(_bstr_t(L"WQL"), 
            _bstr_t(L"SELECT MemoryDevices FROM Win32_PhysicalMemoryArray"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnum2);
        if (SUCCEEDED(hr)) {
            if (pEnum2->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK && uReturn > 0) {
                VARIANT vtProp;
                if (SUCCEEDED(pObj->Get(L"MemoryDevices", 0, &vtProp, 0, 0)) && (vtProp.vt == VT_I4 || vtProp.vt == VT_UI4)) {
                    napi_value v;
                    napi_create_uint32(env, vtProp.uintVal, &v);
                    napi_set_named_property(env, result, "totalSlots", v);
                    VariantClear(&vtProp);
                }
                pObj->Release();
            }
            pEnum2->Release();
        }
        
        napi_value v;
        napi_create_uint32(env, idx, &v);
        napi_set_named_property(env, result, "usedSlots", v);
        
    } while (0);
    
    if (pEnum) pEnum->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    if (needUninit) CoUninitialize();
    
    napi_set_named_property(env, result, "modules", modules);
    return result;
}

// Module Init
napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor props[] = {
        { "getMemoryInfo", 0, GetMemoryInfo, 0, 0, 0, napi_default, 0 },
        { "getMemoryHardware", 0, GetMemoryHardware, 0, 0, 0, napi_default, 0 },
        { "getCpuUsage", 0, GetCpuUsage, 0, 0, 0, napi_default, 0 },
        { "getPerCoreUsage", 0, GetPerCoreUsage, 0, 0, 0, napi_default, 0 },
        { "getUptime", 0, GetUptime, 0, 0, 0, napi_default, 0 },
        { "getSystemStats", 0, GetSystemStats, 0, 0, 0, napi_default, 0 },
        { "getSystemInfo", 0, GetSystemInfo, 0, 0, 0, napi_default, 0 },
        { "getCpuInfo", 0, GetCpuInfo, 0, 0, 0, napi_default, 0 },
        { "getGpuInfo", 0, GetGpuInfo, 0, 0, 0, napi_default, 0 },
        { "getBatteryInfo", 0, GetBatteryInfo, 0, 0, 0, napi_default, 0 },
        { "getDiskInfo", 0, GetDiskInfo, 0, 0, 0, napi_default, 0 },
        { "getDiskIO", 0, GetDiskIO, 0, 0, 0, napi_default, 0 },
        { "getNetworkStats", 0, GetNetworkStats, 0, 0, 0, napi_default, 0 },
        { "getProcessList", 0, GetProcessList, 0, 0, 0, napi_default, 0 },
    };
    napi_define_properties(env, exports, sizeof(props) / sizeof(props[0]), props);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
