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

const loading = ref(true)
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
const cpuUserHistory = ref([])
const cpuSystemHistory = ref([])
const networkDownHistory = ref([])
const networkUpHistory = ref([])

// 组件引用
const cpuPanelRef = ref(null)
const memoryPanelRef = ref(null)
const networkPanelRef = ref(null)

let refreshTimer = null
let processRefreshCount = 0

const tabs = [
  { key: 'overview', label: '概览', icon: '📊' },
  { key: 'cpu', label: 'CPU', icon: '⚡' },
  { key: 'memory', label: '内存', icon: '💾' },
  { key: 'disk', label: '磁盘', icon: '💿' },
  { key: 'network', label: '网络', icon: '🌐' },
  { key: 'gpu', label: 'GPU', icon: '🎮' },
  { key: 'process', label: '进程', icon: '📋' }
]

async function loadData() {
  loading.value = false
  
  try {
    uptime.value = window.services.getUptime()
    
    window.services.getMemoryInfo().then(mem => {
      memoryInfo.value = mem
      memoryHistory.value.push(parseFloat(mem.usedPercent) || 0)
    })
    
    window.services.getSystemInfo().then(sys => {
      systemInfo.value = sys
    })
    
    window.services.getBatteryInfo().then(battery => {
      batteryInfo.value = battery
    })
    
    window.services.getNetworkInfo().then(net => {
      networkInfo.value = net
      const primaryNetStats = net.stats?.[0]
      networkDownHistory.value.push(primaryNetStats?.rxSecBytes || 0)
      networkUpHistory.value.push(primaryNetStats?.txSecBytes || 0)
    })
    
    window.services.getCpuInfo().then(cpu => {
      cpuInfo.value = cpu
      cpuHistory.value.push(parseFloat(cpu.load) || 0)
      cpuUserHistory.value.push(parseFloat(cpu.loadUser) || 0)
      cpuSystemHistory.value.push(parseFloat(cpu.loadSystem) || 0)
    })
    
    window.services.getDiskInfo().then(disk => {
      diskInfo.value = disk
    })
    
    window.services.getGpuInfo().then(gpu => {
      gpuInfo.value = gpu
    })
    
    setTimeout(() => {
      window.services.getProcessInfo().then(proc => {
        processInfo.value = proc
      })
    }, 500)
    
  } catch (e) {
    console.error('加载数据失败:', e)
  }
}

async function refreshDynamic() {
  try {
    const currentTab = activeTab.value
    
    const cpuPromise = window.services.getCpuInfo()
    const memPromise = window.services.getMemoryInfo()
    
    let netPromise = null
    if (currentTab === 'network' || currentTab === 'overview') {
      netPromise = window.services.getNetworkInfo()
    }
    
    processRefreshCount++
    let procPromise = null
    if (currentTab === 'process' && processRefreshCount >= 5) {
      processRefreshCount = 0
      procPromise = window.services.getProcessInfo()
    }
    
    const [cpu, mem] = await Promise.all([cpuPromise, memPromise])
    
    cpuInfo.value = cpu
    memoryInfo.value = mem
    uptime.value = window.services.getUptime()

    cpuHistory.value.push(parseFloat(cpu.load) || 0)
    memoryHistory.value.push(parseFloat(mem.usedPercent) || 0)
    cpuUserHistory.value.push(parseFloat(cpu.loadUser) || 0)
    cpuSystemHistory.value.push(parseFloat(cpu.loadSystem) || 0)
    
    if (cpuHistory.value.length > maxDataPoints) cpuHistory.value.shift()
    if (memoryHistory.value.length > maxDataPoints) memoryHistory.value.shift()
    if (cpuUserHistory.value.length > maxDataPoints) cpuUserHistory.value.shift()
    if (cpuSystemHistory.value.length > maxDataPoints) cpuSystemHistory.value.shift()
    
    if (netPromise) {
      netPromise.then(net => {
        networkInfo.value = net
        const primaryNetStats = net.stats?.[0]
        networkDownHistory.value.push(primaryNetStats?.rxSecBytes ?? 0)
        networkUpHistory.value.push(primaryNetStats?.txSecBytes ?? 0)
        if (networkDownHistory.value.length > maxDataPoints) networkDownHistory.value.shift()
        if (networkUpHistory.value.length > maxDataPoints) networkUpHistory.value.shift()
      })
    }
    
    if (procPromise) {
      procPromise.then(proc => {
        processInfo.value = proc
      })
    }
  } catch (e) {
    console.error('刷新数据失败:', e)
  }
}

onMounted(() => {
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
      <div class="sidebar-header">系统监控</div>
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
      <div v-if="loading" class="loading">加载中...</div>

      <div v-else class="content">
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
          :cpuUserHistory="cpuUserHistory"
          :cpuSystemHistory="cpuSystemHistory"
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
}

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

.loading {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  font-size: 13px;
  color: #999;
}

.content {
  animation: fadeIn 0.3s ease-in;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(8px); }
  to { opacity: 1; transform: translateY(0); }
}
</style>
