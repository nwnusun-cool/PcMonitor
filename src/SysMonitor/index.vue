<script setup>
import { ref, onMounted, onUnmounted, watch } from 'vue'
import Overview from './components/Overview.vue'
import CpuPanel from './components/CpuPanel.vue'
import MemoryPanel from './components/MemoryPanel.vue'
import DiskPanel from './components/DiskPanel.vue'
import NetworkPanel from './components/NetworkPanel.vue'
import GpuPanel from './components/GpuPanel.vue'
import ProcessPanel from './components/ProcessPanel.vue'
import { maxDataPoints } from './composables/useChart'

const loading = ref(false)
const activeTab = ref('overview')
const systemInfo = ref({})
const cpuInfo = ref({})
const memoryInfo = ref({})
const diskInfo = ref({ 
  partitions: [], 
  physical: [], 
  totalSize: '0 B',
  totalUsed: '0 B',
  totalAvailable: '0 B',
  totalPercent: '0%'
})
const networkInfo = ref({ interfaces: [], stats: [] })
const gpuInfo = ref({ controllers: [], displays: [] })
const batteryInfo = ref({})
const processInfo = ref({ topCpu: [], topMem: [], all: 0, running: 0, blocked: 0, sleeping: 0 })
const uptime = ref('')

// 图表数据
const cpuHistory = ref([])
const memoryHistory = ref([])
const networkDownHistory = ref([])
const networkUpHistory = ref([])

// 组件引用
const cpuPanelRef = ref(null)
const memoryPanelRef = ref(null)
const networkPanelRef = ref(null)

let refreshTimer = null

// 检查原生 API 状态
const apiStatus = ref({ loaded: false, error: null })

const tabs = [
  { key: 'overview', label: '概览', icon: '📊' },
  { key: 'cpu', label: 'CPU', icon: '⚡' },
  { key: 'memory', label: '内存', icon: '💾' },
  { key: 'disk', label: '磁盘', icon: '💿' },
  { key: 'network', label: '网络', icon: '🌐' },
  { key: 'gpu', label: 'GPU', icon: '🎮' },
  { key: 'process', label: '进程', icon: '📋' }
]

// 分层加载：快速数据优先显示
async function loadData() {
  try {
    const t0 = Date.now()
    
    // 第一层：快速数据（立即显示）
    uptime.value = window.services.getUptime()
    console.log(`[${Date.now() - t0}ms] uptime done`)
    
    // 兼容：getCpuLoad 可能是同步或异步
    const memResult = window.services.getMemoryInfo()
    const mem = memResult.then ? await memResult : memResult
    console.log(`[${Date.now() - t0}ms] memory done`)
    
    // 兼容：getCpuLoad 可能不存在，用 getCpuInfo 替代
    let cpuLoad
    if (typeof window.services.getCpuLoad === 'function') {
      const cpuResult = window.services.getCpuLoad()
      cpuLoad = cpuResult.then ? await cpuResult : cpuResult
    } else {
      cpuLoad = await window.services.getCpuInfo()
    }
    console.log(`[${Date.now() - t0}ms] cpu done`)
    
    memoryInfo.value = mem
    cpuInfo.value = cpuLoad
    memoryHistory.value.push(parseFloat(mem.usedPercent) || 0)
    cpuHistory.value.push(parseFloat(cpuLoad.load) || 0)
    
    // 快速数据加载完成，立即显示界面
    loading.value = false
    console.log(`[${Date.now() - t0}ms] 快速数据完成，UI应该已更新`)
    
    // 第二层：中速数据（后台加载）
    window.services.getNetworkInfo().then(net => {
      console.log(`[${Date.now() - t0}ms] network done`)
      networkInfo.value = net
      const primaryNetStats = net.stats?.[0]
      networkDownHistory.value.push(primaryNetStats?.rxSecBytes || 0)
      networkUpHistory.value.push(primaryNetStats?.txSecBytes || 0)
    })
    
    window.services.getBatteryInfo().then(battery => {
      console.log(`[${Date.now() - t0}ms] battery done`)
      batteryInfo.value = battery
    })
    
    // 第三层：慢速数据（后台加载，不阻塞）
    window.services.getSystemInfo().then(sys => {
      console.log(`[${Date.now() - t0}ms] systemInfo done`)
      systemInfo.value = sys
    })
    
    window.services.getCpuInfo().then(cpu => {
      console.log(`[${Date.now() - t0}ms] cpuInfo done`)
      cpuInfo.value = cpu
    })
    
    window.services.getDiskInfo().then(disk => {
      console.log(`[${Date.now() - t0}ms] disk done`)
      diskInfo.value = disk
    })
    
    window.services.getGpuInfo().then(gpu => {
      console.log(`[${Date.now() - t0}ms] gpu done`)
      gpuInfo.value = gpu
    })
    
    window.services.getProcessInfo().then(proc => {
      console.log(`[${Date.now() - t0}ms] process done`)
      processInfo.value = proc
    })
    
  } catch (e) {
    console.error('加载数据失败:', e)
    loading.value = false
  }
}

async function refreshDynamic() {
  try {
    const currentTab = activeTab.value
    
    // 兼容处理：如果 getCpuLoad 不存在则用 getCpuInfo
    const cpuLoadFn = window.services.getCpuLoad || window.services.getCpuInfo
    
    // 快速数据：每次都刷新
    const [cpuLoad, mem] = await Promise.all([
      cpuLoadFn(),
      window.services.getMemoryInfo()
    ])
    
    // 合并 CPU 负载到现有 cpuInfo
    cpuInfo.value = { ...cpuInfo.value, ...cpuLoad }
    memoryInfo.value = mem
    uptime.value = window.services.getUptime()

    cpuHistory.value.push(parseFloat(cpuLoad.load) || 0)
    memoryHistory.value.push(parseFloat(mem.usedPercent) || 0)
    
    if (cpuHistory.value.length > maxDataPoints) cpuHistory.value.shift()
    if (memoryHistory.value.length > maxDataPoints) memoryHistory.value.shift()
    
    // 网络：仅在相关 tab 时刷新
    if (currentTab === 'network' || currentTab === 'overview') {
      window.services.getNetworkInfo().then(net => {
        networkInfo.value = net
        const primaryNetStats = net.stats?.[0]
        networkDownHistory.value.push(primaryNetStats?.rxSecBytes ?? 0)
        networkUpHistory.value.push(primaryNetStats?.txSecBytes ?? 0)
        if (networkDownHistory.value.length > maxDataPoints) networkDownHistory.value.shift()
        if (networkUpHistory.value.length > maxDataPoints) networkUpHistory.value.shift()
      })
    }
    
    // 进程：仅在进程 tab 时刷新（服务层已有 5 秒缓存）
    if (currentTab === 'process') {
      window.services.getProcessInfo().then(proc => {
        processInfo.value = proc
      })
    }
    
    // 磁盘：仅在磁盘 tab 时刷新（服务层已有 10 秒缓存）
    if (currentTab === 'disk') {
      window.services.getDiskInfo().then(disk => {
        diskInfo.value = disk
      })
    }
  } catch (e) {
    console.error('刷新数据失败:', e)
  }
}

onMounted(() => {
  // 检查原生 API 状态
  apiStatus.value = {
    loaded: window.__nativeApiLoaded || false,
    error: window.__nativeApiError || null
  }
  
  loadData()
  refreshTimer = setInterval(refreshDynamic, 1000)
})

onUnmounted(() => {
  if (refreshTimer) clearInterval(refreshTimer)
})
</script>

<template>
  <div class="sys-monitor">
    <div class="sidebar">
      <div class="sidebar-header">
        系统监控
        <span class="api-status" :class="apiStatus.loaded ? 'fast' : 'slow'" :title="apiStatus.error || '原生API已加载'">
          {{ apiStatus.loaded ? '⚡' : '🐢' }}
        </span>
        <button class="refresh-btn" @click="loadData" title="刷新数据">🔄</button>
      </div>
      <div class="menu">
        <button
          v-for="tab in tabs"
          :key="tab.key"
          :class="['menu-item', { active: activeTab === tab.key }]"
          @click="activeTab = tab.key"
        >
          <span class="menu-icon">{{ tab.icon }}</span>
          <span class="menu-label">{{ tab.label }}</span>
        </button>
      </div>
    </div>

    <div class="main-content">
      <div class="content">
        <Overview 
          v-if="activeTab === 'overview'"
          :cpuInfo="cpuInfo"
          :memoryInfo="memoryInfo"
          :diskInfo="diskInfo"
          :networkInfo="networkInfo"
          :systemInfo="systemInfo"
          :batteryInfo="batteryInfo"
          :processInfo="processInfo"
          :uptime="uptime"
        />

        <CpuPanel 
          v-if="activeTab === 'cpu'"
          ref="cpuPanelRef"
          :cpuInfo="cpuInfo"
          :cpuHistory="cpuHistory"
        />

        <MemoryPanel 
          v-if="activeTab === 'memory'"
          ref="memoryPanelRef"
          :memoryInfo="memoryInfo"
          :memoryHistory="memoryHistory"
        />

        <DiskPanel 
          v-if="activeTab === 'disk'"
          :diskInfo="diskInfo"
        />

        <NetworkPanel 
          v-if="activeTab === 'network'"
          ref="networkPanelRef"
          :networkInfo="networkInfo"
          :networkDownHistory="networkDownHistory"
          :networkUpHistory="networkUpHistory"
        />

        <GpuPanel 
          v-if="activeTab === 'gpu'"
          :gpuInfo="gpuInfo"
        />

        <ProcessPanel 
          v-if="activeTab === 'process'"
          :processInfo="processInfo"
        />
      </div>
    </div>
  </div>
</template>

<style scoped>
* {
  box-sizing: border-box;
}

.sys-monitor {
  display: flex;
  height: 600px;
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Microsoft YaHei', sans-serif;
  background: #f5f5f5;
  color: #333;
  overflow: hidden;
}

.sidebar {
  width: 140px;
  background: #fff;
  border-right: 1px solid #e0e0e0;
  display: flex;
  flex-direction: column;
  flex-shrink: 0;
}

.sidebar-header {
  padding: 14px 12px;
  font-size: 13px;
  font-weight: 700;
  color: #1a73e8;
  border-bottom: 1px solid #e0e0e0;
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.refresh-btn {
  background: none;
  border: none;
  font-size: 14px;
  cursor: pointer;
  padding: 4px;
  border-radius: 4px;
  transition: background 0.2s;
}

.refresh-btn:hover {
  background: #e8f0fe;
}

.api-status {
  font-size: 12px;
  cursor: help;
}

.api-status.fast { color: #34a853; }
.api-status.slow { color: #ea4335; }

.menu {
  flex: 1;
  padding: 6px 4px;
  overflow-y: auto;
}

.menu::-webkit-scrollbar {
  width: 3px;
}

.menu::-webkit-scrollbar-thumb {
  background: #d0d0d0;
  border-radius: 2px;
}

.menu-item {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 9px 10px;
  margin-bottom: 2px;
  border: none;
  background: transparent;
  color: #666;
  cursor: pointer;
  transition: all 0.2s;
  text-align: left;
  font-size: 12px;
  border-radius: 6px;
  position: relative;
}

.menu-item::before {
  content: '';
  position: absolute;
  left: 0;
  top: 50%;
  transform: translateY(-50%);
  width: 3px;
  height: 0;
  background: #1a73e8;
  border-radius: 0 2px 2px 0;
  transition: height 0.2s;
}

.menu-item:hover {
  background: #f0f0f0;
  color: #333;
}

.menu-item.active {
  background: #e8f0fe;
  color: #1a73e8;
  font-weight: 500;
}

.menu-item.active::before {
  height: 60%;
}

.menu-icon {
  font-size: 15px;
  width: 18px;
  text-align: center;
}

.menu-label {
  flex: 1;
}

.main-content {
  flex: 1;
  height: 600px;
  overflow-y: auto;
  padding: 14px 16px;
  background: #f5f5f5;
}

.main-content::-webkit-scrollbar {
  width: 5px;
}

.main-content::-webkit-scrollbar-track {
  background: #f0f0f0;
}

.main-content::-webkit-scrollbar-thumb {
  background: #c0c0c0;
  border-radius: 3px;
}

.content {
  animation: fadeIn 0.3s ease-in;
}

@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}
</style>
