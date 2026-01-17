/**
 * 测试每核心 CPU 使用率 - C++ 版本
 * 
 * 编译: cl /EHsc test-per-core-cpp.cpp pdh.lib
 * 或者用 g++: g++ test-per-core-cpp.cpp -o test-per-core-cpp.exe -lpdh
 */

#include <windows.h>
#include <pdh.h>
#include <iostream>
#include <vector>
#include <string>

#pragma comment(lib, "pdh.lib")

int main() {
    std::cout << "=== Per-Core CPU Usage Test (C++ PDH) ===" << std::endl << std::endl;
    
    // Get number of logical processors
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    int numCores = si.dwNumberOfProcessors;
    std::cout << "Logical Cores: " << numCores << std::endl << std::endl;
    
    // Open PDH query
    PDH_HQUERY query = NULL;
    if (PdhOpenQuery(NULL, 0, &query) != ERROR_SUCCESS) {
        std::cerr << "Failed to open PDH query" << std::endl;
        return 1;
    }
    
    // Add counter for each core
    std::vector<PDH_HCOUNTER> counters(numCores);
    PDH_HCOUNTER totalCounter = NULL;
    
    // Try Processor Information (modern, matches Task Manager)
    bool useProcessorInfo = true;
    for (int i = 0; i < numCores; i++) {
        wchar_t path[256];
        // Format: \Processor Information(0,0)\% Processor Utility
        // On most systems, it's (0,N) where N is the core index
        swprintf(path, 256, L"\\Processor Information(0,%d)\\%% Processor Utility", i);
        
        if (PdhAddEnglishCounterW(query, path, 0, &counters[i]) != ERROR_SUCCESS) {
            // Fallback to old format
            swprintf(path, 256, L"\\Processor(%d)\\%% Processor Time", i);
            if (PdhAddEnglishCounterW(query, path, 0, &counters[i]) != ERROR_SUCCESS) {
                std::cerr << "Failed to add counter for core " << i << std::endl;
                useProcessorInfo = false;
            }
        }
    }
    
    // Add total counter
    if (PdhAddEnglishCounterW(query, L"\\Processor Information(_Total)\\% Processor Utility", 0, &totalCounter) != ERROR_SUCCESS) {
        PdhAddEnglishCounterW(query, L"\\Processor(_Total)\\% Processor Time", 0, &totalCounter);
    }
    
    // First collection to initialize
    PdhCollectQueryData(query);
    std::cout << "Collecting data... (wait 1 second)" << std::endl;
    Sleep(1000);
    
    // Second collection to get actual values
    if (PdhCollectQueryData(query) != ERROR_SUCCESS) {
        std::cerr << "Failed to collect query data" << std::endl;
        PdhCloseQuery(query);
        return 1;
    }
    
    std::cout << std::endl << "Results:" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    // Get values for each core
    for (int i = 0; i < numCores; i++) {
        PDH_FMT_COUNTERVALUE value;
        if (PdhGetFormattedCounterValue(counters[i], PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
            double usage = value.doubleValue;
            if (usage < 0) usage = 0;
            if (usage > 100) usage = 100;
            std::cout << "Core " << i << ": " << usage << "%" << std::endl;
        } else {
            std::cout << "Core " << i << ": N/A" << std::endl;
        }
    }
    
    // Get total
    if (totalCounter) {
        PDH_FMT_COUNTERVALUE value;
        if (PdhGetFormattedCounterValue(totalCounter, PDH_FMT_DOUBLE, NULL, &value) == ERROR_SUCCESS) {
            double usage = value.doubleValue;
            if (usage < 0) usage = 0;
            if (usage > 100) usage = 100;
            std::cout << "----------------------------------------" << std::endl;
            std::cout << "Total: " << usage << "%" << std::endl;
        }
    }
    
    PdhCloseQuery(query);
    
    std::cout << std::endl << "=== Test Complete ===" << std::endl;
    return 0;
}
