<script setup>
import { ref, computed } from 'vue'

const props = defineProps({
  processInfo: Object,
  systemStats: Object
})

const sortBy = ref('memory')
const sortOrder = ref('desc')
const searchQuery = ref('')

const sortedProcesses = computed(() => {
  let list = props.processInfo?.topMem || []
  if (searchQuery.value) {
    const q = searchQuery.value.toLowerCase()
    list = list.filter(p => p.name.toLowerCase().includes(q) || String(p.pid).includes(q))
  }
  const key = sortBy.value === 'memory' ? 'memoryRaw' : sortBy.value === 'cpu' ? 'cpuRaw' : sortBy.value
  list = [...list].sort((a, b) => {
    const aVal = a[key] || 0, bVal = b[key] || 0
    return sortOrder.value === 'desc' ? bVal - aVal : aVal - bVal
  })
  return list.slice(0, 15)
})

function toggleSort(field) {
  if (sortBy.value === field) {
    sortOrder.value = sortOrder.value === 'desc' ? 'asc' : 'desc'
  } else {
    sortBy.value = field
    sortOrder.value = 'desc'
  }
}

function getSortIcon(field) {
  if (sortBy.value !== field) return ''
  return sortOrder.value === 'desc' ? '↓' : '↑'
}
</script>

<template>
  <div class="panel compact">
    <!-- 统计 + 排行 -->
    <div class="top-row">
      <div class="stats-section">
        <h3>系统统计</h3>
        <div class="stats-grid">
          <div class="stat-item">
            <span class="stat-value">{{ processInfo?.all || systemStats?.processCount || 0 }}</span>
            <span class="stat-label">进程</span>
          </div>
          <div class="stat-item">
            <span class="stat-value">{{ (systemStats?.threadCount || 0).toLocaleString() }}</span>
            <span class="stat-label">线程</span>
          </div>
          <div class="stat-item">
            <span class="stat-value">{{ (systemStats?.handleCount || 0).toLocaleString() }}</span>
            <span class="stat-label">句柄</span>
          </div>
        </div>
      </div>
      <div class="top-section">
        <h3>资源占用</h3>
        <div class="top-grid">
          <div class="top-item cpu">
            <span class="top-icon">⚡</span>
            <div class="top-info">
              <span class="top-name" v-if="processInfo?.topCpu?.[0]">{{ processInfo.topCpu[0].name }}</span>
              <span class="skeleton-text" v-else></span>
              <span class="top-value cpu" v-if="processInfo?.topCpu?.[0]">{{ processInfo.topCpu[0].cpu }}</span>
            </div>
          </div>
          <div class="top-item mem">
            <span class="top-icon">💾</span>
            <div class="top-info">
              <span class="top-name" v-if="processInfo?.topMem?.[0]">{{ processInfo.topMem[0].name }}</span>
              <span class="skeleton-text" v-else></span>
              <span class="top-value mem" v-if="processInfo?.topMem?.[0]">{{ processInfo.topMem[0].memory }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- 进程列表 -->
    <div class="list-header">
      <h3>进程列表</h3>
      <input type="text" v-model="searchQuery" placeholder="搜索..." class="search-input" />
    </div>

    <div class="table-wrapper">
      <table class="process-table">
        <thead>
          <tr>
            <th class="col-name">进程</th>
            <th class="col-pid" @click="toggleSort('pid')">PID {{ getSortIcon('pid') }}</th>
            <th class="col-cpu" @click="toggleSort('cpu')">CPU {{ getSortIcon('cpu') }}</th>
            <th class="col-mem" @click="toggleSort('memory')">内存 {{ getSortIcon('memory') }}</th>
            <th class="col-threads" @click="toggleSort('threads')">线程 {{ getSortIcon('threads') }}</th>
            <th class="col-handles" @click="toggleSort('handles')">句柄 {{ getSortIcon('handles') }}</th>
          </tr>
        </thead>
        <tbody v-if="sortedProcesses.length > 0">
          <tr v-for="proc in sortedProcesses" :key="proc.pid">
            <td class="col-name" :title="proc.name">{{ proc.name }}</td>
            <td class="col-pid">{{ proc.pid }}</td>
            <td class="col-cpu">
              <div class="bar-cell">
                <div class="mini-bar cpu" :style="{ width: Math.min(proc.cpuRaw * 5, 100) + '%' }"></div>
                <span>{{ proc.cpu }}</span>
              </div>
            </td>
            <td class="col-mem">
              <div class="bar-cell">
                <div class="mini-bar mem" :style="{ width: Math.min(proc.memPercentRaw * 10, 100) + '%' }"></div>
                <span>{{ proc.memory }}</span>
              </div>
            </td>
            <td class="col-threads">{{ proc.threads }}</td>
            <td class="col-handles">{{ proc.handles?.toLocaleString() }}</td>
          </tr>
        </tbody>
        <tbody v-else>
          <tr v-for="i in 6" :key="i">
            <td><span class="skeleton-text"></span></td>
            <td><span class="skeleton-text short"></span></td>
            <td><span class="skeleton-text short"></span></td>
            <td><span class="skeleton-text"></span></td>
            <td><span class="skeleton-text short"></span></td>
            <td><span class="skeleton-text short"></span></td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>


<style scoped>
@import '../styles/common.css';

.panel.compact h3 { margin: 0 0 8px; font-size: 12px; }

.top-row { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-bottom: 10px; }

.stats-section { background: #fff; padding: 10px; border-radius: 8px; border: 1px solid #e0e0e0; }
.stats-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 6px; }
.stat-item { display: flex; flex-direction: column; align-items: center; padding: 8px 4px; background: #f8f9fa; border-radius: 6px; }
.stat-value { font-size: 18px; font-weight: 700; color: #1a73e8; line-height: 1; }
.stat-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; margin-top: 4px; }

.top-section { background: #fff; padding: 10px; border-radius: 8px; border: 1px solid #e0e0e0; }
.top-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 6px; }
.top-item { display: flex; align-items: center; gap: 8px; padding: 8px; background: #f8f9fa; border-radius: 6px; border-left: 3px solid; }
.top-item.cpu { border-left-color: #1a73e8; }
.top-item.mem { border-left-color: #34a853; }
.top-icon { font-size: 16px; }
.top-info { flex: 1; min-width: 0; display: flex; flex-direction: column; gap: 2px; }
.top-name { font-size: 11px; font-weight: 600; color: #333; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.top-value { font-size: 13px; font-weight: 700; }
.top-value.cpu { color: #1a73e8; }
.top-value.mem { color: #34a853; }

.list-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
.list-header h3 { margin: 0 !important; }
.search-input { padding: 5px 10px; border: 1px solid #e0e0e0; border-radius: 5px; font-size: 11px; width: 120px; outline: none; }
.search-input:focus { border-color: #1a73e8; }

.table-wrapper { background: #fff; border-radius: 8px; border: 1px solid #e0e0e0; overflow: hidden; }
.process-table { width: 100%; border-collapse: collapse; font-size: 11px; }
.process-table th, .process-table td { padding: 8px 10px; text-align: left; }
.process-table th { background: #f8f9fa; color: #666; font-weight: 600; font-size: 9px; text-transform: uppercase; border-bottom: 1px solid #e0e0e0; cursor: pointer; user-select: none; }
.process-table th:hover { background: #f0f0f0; }
.process-table td { border-bottom: 1px solid #f0f0f0; }
.process-table tr:last-child td { border-bottom: none; }
.process-table tbody tr:hover td { background: #f8f9fa; }

.col-name { width: 28%; max-width: 120px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; font-weight: 500; }
.col-pid { width: 10%; color: #666; font-family: monospace; font-size: 10px; }
.col-cpu, .col-mem { width: 18%; }
.col-threads, .col-handles { width: 13%; color: #666; text-align: right; }

.bar-cell { display: flex; align-items: center; gap: 5px; }
.mini-bar { height: 3px; border-radius: 2px; min-width: 2px; max-width: 40px; }
.mini-bar.cpu { background: #1a73e8; }
.mini-bar.mem { background: #34a853; }

.skeleton-text { display: inline-block; height: 10px; width: 50px; background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%); background-size: 200% 100%; animation: shimmer 1.5s infinite; border-radius: 3px; }
.skeleton-text.short { width: 30px; }

@keyframes shimmer { 0% { background-position: -200% 0; } 100% { background-position: 200% 0; } }
</style>
