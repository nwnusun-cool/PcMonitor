<script setup>
import { ref, onMounted, watch, computed } from 'vue'
import { setupCanvas, drawNetworkChartWithHighlight, getDataIndexAtPosition, formatIOValue } from '../composables/useChart'

const props = defineProps({
  networkInfo: Object,
  networkDownHistory: Array,
  networkUpHistory: Array,
  externalIP: Object,
  connections: Object
})

const canvasRef = ref(null)
const showVirtual = ref(false)
const expandedCard = ref(null)
const tooltipVisible = ref(false)
const tooltipX = ref(0)
const tooltipY = ref(0)
const tooltipDown = ref('')
const tooltipUp = ref('')
const highlightIndex = ref(-1)
const activeTab = ref('interfaces') // interfaces, connections
const connFilter = ref('all') // all, established, listening
const connSearch = ref('')

function toggleCard(iface) {
  expandedCard.value = expandedCard.value === iface ? null : iface
}

function copyText(text, e) {
  e.stopPropagation()
  if (window.utools) {
    window.utools.copyText(text)
    window.utools.showNotification('已复制: ' + text)
  } else {
    navigator.clipboard.writeText(text)
  }
}

function redraw() {
  if (canvasRef.value && props.networkDownHistory.length > 0) {
    requestAnimationFrame(() => {
      drawNetworkChartWithHighlight(canvasRef.value, [
        { data: props.networkDownHistory, color: '#34a853' },
        { data: props.networkUpHistory, color: '#1a73e8' }
      ], highlightIndex.value)
    })
  }
}

function handleMouseMove(e) {
  if (!canvasRef.value || props.networkDownHistory.length === 0) return
  const rect = canvasRef.value.getBoundingClientRect()
  const mouseX = e.clientX - rect.left
  const index = getDataIndexAtPosition(canvasRef.value, mouseX, props.networkDownHistory.length, true)
  if (index >= 0 && index < props.networkDownHistory.length) {
    highlightIndex.value = index
    tooltipDown.value = formatIOValue(props.networkDownHistory[index]) + '/s'
    tooltipUp.value = formatIOValue(props.networkUpHistory[index]) + '/s'
    tooltipX.value = e.clientX - rect.left
    tooltipY.value = e.clientY - rect.top - 40
    tooltipVisible.value = true
    redraw()
  } else {
    handleMouseLeave()
  }
}

function handleMouseLeave() {
  tooltipVisible.value = false
  highlightIndex.value = -1
  redraw()
}

// Filter connections
const filteredConnections = computed(() => {
  if (!props.connections || !props.connections.tcp) return []
  
  let conns = props.connections.tcp
  
  // Filter by state
  if (connFilter.value === 'established') {
    conns = conns.filter(c => c.state === 'ESTABLISHED')
  } else if (connFilter.value === 'listening') {
    conns = conns.filter(c => c.state === 'LISTENING')
  }
  
  // Search filter
  if (connSearch.value) {
    const search = connSearch.value.toLowerCase()
    conns = conns.filter(c => 
      (c.process || '').toLowerCase().includes(search) ||
      c.localAddress.includes(search) ||
      c.remoteAddress.includes(search) ||
      c.localPort.toString().includes(search) ||
      c.remotePort.toString().includes(search)
    )
  }
  
  return conns.slice(0, 50) // Limit to 50 for performance
})

const connectionStats = computed(() => {
  if (!props.connections) return { total: 0, established: 0, listening: 0 }
  return {
    total: props.connections.totalTcp || 0,
    established: props.connections.totalEstablished || 0,
    listening: props.connections.totalListening || 0
  }
})

onMounted(() => {
  setTimeout(() => {
    if (canvasRef.value) {
      const rect = canvasRef.value.getBoundingClientRect()
      if (rect.width > 0 && rect.height > 0) {
        setupCanvas(canvasRef.value)
        redraw()
      }
    }
  }, 50)
})

watch(() => props.networkDownHistory.length, redraw)
defineExpose({ redraw, canvasRef })
</script>

<template>
  <div class="panel compact">
    <!-- 外部 IP 信息 -->
    <div class="external-ip-section" v-if="externalIP">
      <div class="external-ip-card">
        <div class="ip-icon">🌍</div>
        <div class="ip-content">
          <div class="ip-main">
            <span class="ip-label">外部 IP</span>
            <span class="ip-value" @click="copyText(externalIP.ip, $event)" v-if="externalIP.ip">
              {{ externalIP.ip }}
            </span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="ip-details" v-if="externalIP.locationText">
            <span class="ip-location">📍 {{ externalIP.locationText }}</span>
            <span class="ip-isp" v-if="externalIP.ispText">🏢 {{ externalIP.ispText }}</span>
          </div>
        </div>
      </div>
    </div>

    <!-- 流量趋势 -->
    <h3>网络流量</h3>
    <div class="chart-section">
      <div class="chart-stats">
        <div class="stat-item">
          <span class="stat-label">下载</span>
          <span class="stat-value down" v-if="networkInfo.stats?.[0]">{{ networkInfo.stats[0].rxSec }}</span>
          <span class="skeleton-text" v-else></span>
        </div>
        <div class="stat-item">
          <span class="stat-label">上传</span>
          <span class="stat-value up" v-if="networkInfo.stats?.[0]">{{ networkInfo.stats[0].txSec }}</span>
          <span class="skeleton-text" v-else></span>
        </div>
      </div>
      <div class="chart-wrapper">
        <canvas 
          ref="canvasRef" 
          class="chart-canvas"
          @mousemove="handleMouseMove"
          @mouseleave="handleMouseLeave"
        ></canvas>
        <div 
          v-if="tooltipVisible" 
          class="chart-tooltip"
          :style="{ left: tooltipX + 'px', top: tooltipY + 'px' }"
        >
          <div class="tooltip-row"><span class="tooltip-dot down"></span>↓ {{ tooltipDown }}</div>
          <div class="tooltip-row"><span class="tooltip-dot up"></span>↑ {{ tooltipUp }}</div>
        </div>
      </div>
    </div>

    <!-- 网络接口 -->
    <div class="section-header">
      <div class="tabs">
        <button :class="['tab', { active: activeTab === 'interfaces' }]" @click="activeTab = 'interfaces'">
          网络接口
        </button>
        <button :class="['tab', { active: activeTab === 'connections' }]" @click="activeTab = 'connections'">
          连接监控
          <span class="badge-count" v-if="connectionStats.total">{{ connectionStats.total }}</span>
        </button>
      </div>
      <label class="toggle-virtual" v-if="activeTab === 'interfaces' && networkInfo.virtualInterfaces?.length">
        <input type="checkbox" v-model="showVirtual" />
        <span>虚拟 ({{ networkInfo.virtualInterfaces.length }})</span>
      </label>
    </div>

    <!-- 网络接口列表 -->
    <div v-if="activeTab === 'interfaces' && networkInfo.interfaces?.length" class="interfaces">
      <div v-for="iface in networkInfo.interfaces" :key="iface.iface" 
           class="iface-card" 
           :class="{ 'is-default': iface.isDefault, 'expanded': expandedCard === iface.iface }"
           @click="toggleCard(iface.iface)">
        <div class="iface-header">
          <div class="iface-icon-wrap">
            <span class="iface-icon">{{ iface.type === 'wireless' ? '📶' : '🔌' }}</span>
            <span class="status-dot" :class="iface.isUp ? 'online' : 'offline'"></span>
          </div>
          <div class="iface-info">
            <div class="iface-name">
              {{ iface.ifaceName }}
              <span class="badge" v-if="iface.isDefault">主</span>
            </div>
            <div class="iface-meta">{{ iface.ip4 }}</div>
            <div class="iface-util" v-if="iface.utilizationFmt">{{ iface.utilizationFmt }}</div>
          </div>
          <div class="iface-traffic">
            <span class="traffic-down">↓{{ iface.rxSec || '0 B/s' }}</span>
            <span class="traffic-up">↑{{ iface.txSec || '0 B/s' }}</span>
          </div>
          <span class="expand-icon">{{ expandedCard === iface.iface ? '▲' : '▼' }}</span>
        </div>
        
        <div class="iface-detail" :class="{ 'show': expandedCard === iface.iface }">
          <div class="detail-grid">
            <div class="detail-item">
              <span class="detail-label">IPv4</span>
              <span class="detail-value" @click="copyText(iface.ip4, $event)">{{ iface.ip4 }}</span>
            </div>
            <div class="detail-item" v-if="iface.ip6">
              <span class="detail-label">IPv6</span>
              <span class="detail-value" @click="copyText(iface.ip6, $event)">{{ iface.ip6 }}</span>
            </div>
            <div class="detail-item" v-if="iface.subnet">
              <span class="detail-label">子网掩码</span>
              <span class="detail-value" @click="copyText(iface.subnet, $event)">{{ iface.subnet }}</span>
            </div>
            <div class="detail-item" v-if="iface.gateway">
              <span class="detail-label">网关</span>
              <span class="detail-value" @click="copyText(iface.gateway, $event)">{{ iface.gateway }}</span>
            </div>
            <div class="detail-item" v-if="iface.dns && iface.dns.length > 0">
              <span class="detail-label">DNS</span>
              <span class="detail-value" @click="copyText(iface.dns[0], $event)">{{ iface.dns[0] }}</span>
            </div>
            <div class="detail-item" v-if="iface.dns && iface.dns.length > 1">
              <span class="detail-label">DNS 2</span>
              <span class="detail-value" @click="copyText(iface.dns[1], $event)">{{ iface.dns[1] }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">MAC</span>
              <span class="detail-value" @click="copyText(iface.mac, $event)">{{ iface.mac }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">DHCP</span>
              <span class="detail-value">{{ iface.dhcp ? '启用' : '禁用' }}</span>
            </div>
            <div class="detail-item" v-if="iface.speed">
              <span class="detail-label">链路速度</span>
              <span class="detail-value">{{ iface.speed }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">接收包</span>
              <span class="detail-value">{{ iface.rxPackets?.toLocaleString() || '0' }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">发送包</span>
              <span class="detail-value">{{ iface.txPackets?.toLocaleString() || '0' }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">总接收</span>
              <span class="detail-value">{{ iface.rxBytes || '0 B' }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">总发送</span>
              <span class="detail-value">{{ iface.txBytes || '0 B' }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div v-else-if="activeTab === 'interfaces'" class="skeleton-list">
      <div class="skeleton-card" v-for="i in 2" :key="i"></div>
    </div>

    <div v-if="activeTab === 'interfaces' && showVirtual && networkInfo.virtualInterfaces?.length" class="virtual-section">
      <div class="virtual-list">
        <div v-for="iface in networkInfo.virtualInterfaces" :key="iface.iface" class="virtual-item">
          <span class="virtual-icon">🔗</span>
          <span class="virtual-name">{{ iface.ifaceName }}</span>
          <span class="virtual-ip">{{ iface.ip4 || '-' }}</span>
        </div>
      </div>
    </div>

    <!-- 连接监控 -->
    <div v-if="activeTab === 'connections'" class="connections-view">
      <div class="conn-toolbar">
        <div class="conn-stats">
          <span class="stat-badge">总计: {{ connectionStats.total }}</span>
          <span class="stat-badge established">活动: {{ connectionStats.established }}</span>
          <span class="stat-badge listening">监听: {{ connectionStats.listening }}</span>
        </div>
        <div class="conn-filters">
          <select v-model="connFilter" class="filter-select">
            <option value="all">全部</option>
            <option value="established">活动连接</option>
            <option value="listening">监听端口</option>
          </select>
          <input v-model="connSearch" type="text" placeholder="搜索..." class="search-input" />
        </div>
      </div>

      <div class="conn-list">
        <div v-for="conn in filteredConnections" :key="`${conn.protocol}-${conn.localPort}-${conn.remotePort}`" class="conn-item">
          <div class="conn-process">
            <span class="process-name">{{ conn.process || 'Unknown' }}</span>
            <span class="process-pid">PID: {{ conn.pid }}</span>
          </div>
          <div class="conn-details">
            <span class="conn-protocol">{{ conn.protocol }}</span>
            <span class="conn-address">{{ conn.localAddress }}:{{ conn.localPort }}</span>
            <span class="conn-arrow" v-if="conn.state === 'ESTABLISHED'">→</span>
            <span class="conn-address" v-if="conn.state === 'ESTABLISHED'">{{ conn.remoteAddress }}:{{ conn.remotePort }}</span>
            <span class="conn-state" :class="conn.state.toLowerCase()">{{ conn.state }}</span>
          </div>
        </div>
        <div v-if="filteredConnections.length === 0" class="conn-empty">
          <span>{{ connSearch ? '未找到匹配的连接' : '暂无连接' }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.panel.compact h3 { margin: 0 0 8px; font-size: 12px; }
.panel.compact h3:not(:first-child) { margin-top: 10px; }

/* 外部 IP 卡片 */
.external-ip-section { margin-bottom: 10px; }
.external-ip-card {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  padding: 12px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  gap: 10px;
  color: #fff;
}
.ip-icon { font-size: 24px; }
.ip-content { flex: 1; min-width: 0; }
.ip-main { display: flex; align-items: center; gap: 8px; margin-bottom: 4px; }
.ip-label { font-size: 10px; opacity: 0.9; font-weight: 600; text-transform: uppercase; }
.ip-value { 
  font-size: 14px; 
  font-weight: 700; 
  font-family: monospace; 
  cursor: pointer;
  padding: 2px 6px;
  border-radius: 4px;
  transition: background 0.2s;
}
.ip-value:hover { background: rgba(255, 255, 255, 0.2); }
.ip-details { display: flex; flex-direction: column; gap: 2px; font-size: 10px; opacity: 0.95; }
.ip-location, .ip-isp { display: flex; align-items: center; gap: 4px; }

/* 图表区块 */
.chart-section {
  background: #fff;
  padding: 12px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 10px;
}
.chart-stats { display: flex; gap: 20px; margin-bottom: 8px; }
.chart-stats .stat-item { display: flex; align-items: center; gap: 6px; }
.chart-stats .stat-label { font-size: 11px; color: #666; font-weight: 500; }
.chart-stats .stat-value { font-size: 14px; font-weight: 700; }
.chart-stats .stat-value.down { color: #34a853; }
.chart-stats .stat-value.up { color: #1a73e8; }
.chart-wrapper { height: 100px; position: relative; }
.chart-canvas { width: 100%; height: 100%; display: block; }

/* 区块头 */
.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 8px; }
.section-header h3 { margin: 0 !important; }
.tabs { display: flex; gap: 4px; }
.tab {
  padding: 4px 10px;
  font-size: 11px;
  font-weight: 600;
  border: none;
  background: #e8e8e8;
  color: #666;
  border-radius: 5px;
  cursor: pointer;
  transition: all 0.2s;
  display: flex;
  align-items: center;
  gap: 4px;
}
.tab:hover { background: #d0d0d0; color: #333; }
.tab.active { 
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: #fff;
  box-shadow: 0 2px 4px rgba(102, 126, 234, 0.3);
}
.badge-count {
  font-size: 9px;
  padding: 1px 5px;
  border-radius: 10px;
  background: rgba(0, 0, 0, 0.15);
  font-weight: 700;
}
.tab.active .badge-count { 
  background: rgba(255, 255, 255, 0.3);
  color: #fff;
}
.toggle-virtual { display: flex; align-items: center; gap: 4px; font-size: 10px; color: #666; cursor: pointer; }
.toggle-virtual input { cursor: pointer; width: 12px; height: 12px; }

/* 网卡列表 */
.interfaces { display: flex; flex-direction: column; gap: 6px; }
.iface-card { background: #fff; border-radius: 8px; border: 1px solid #e0e0e0; overflow: hidden; cursor: pointer; }
.iface-card:hover { border-color: #1a73e8; }
.iface-card.is-default { border-color: #1a73e8; border-width: 2px; }

.iface-header { display: flex; align-items: center; gap: 8px; padding: 8px 10px; }
.iface-icon-wrap { position: relative; }
.iface-icon { font-size: 18px; width: 32px; height: 32px; display: flex; align-items: center; justify-content: center; background: #f5f5f5; border-radius: 6px; }
.status-dot { position: absolute; bottom: -1px; right: -1px; width: 8px; height: 8px; border-radius: 50%; border: 2px solid #fff; }
.status-dot.online { background: #34a853; }
.status-dot.offline { background: #999; }

.iface-info { flex: 1; min-width: 0; }
.iface-name { font-size: 11px; font-weight: 600; color: #333; display: flex; align-items: center; gap: 4px; }
.badge { font-size: 9px; padding: 1px 4px; border-radius: 3px; background: #e8f0fe; color: #1a73e8; font-weight: 500; }
.iface-meta { font-size: 10px; color: #888; margin-top: 1px; }
.iface-util { font-size: 9px; color: #666; margin-top: 1px; font-weight: 600; }

.iface-traffic { display: flex; flex-direction: column; align-items: flex-end; gap: 1px; font-size: 10px; font-weight: 600; }
.traffic-down { color: #34a853; }
.traffic-up { color: #1a73e8; }
.expand-icon { font-size: 9px; color: #999; }

.iface-detail { 
  border-top: 1px solid #e8e8e8; 
  padding: 0 10px; 
  background: #fafafa; 
  max-height: 0; 
  overflow: hidden; 
  transition: max-height 0.2s, padding 0.2s;
}
.iface-detail.show { max-height: 300px; padding: 8px 10px; }
.detail-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 6px; }
.detail-item { display: flex; flex-direction: column; gap: 1px; }
.detail-label { font-size: 9px; color: #888; text-transform: uppercase; font-weight: 600; }
.detail-value { font-size: 10px; color: #333; font-family: monospace; cursor: pointer; }
.detail-value:hover { color: #1a73e8; }

/* 虚拟网卡 */
.virtual-section { margin-top: 8px; }
.virtual-list { background: #f8f9fa; border-radius: 6px; padding: 4px; }
.virtual-item { display: flex; align-items: center; gap: 6px; padding: 5px 6px; font-size: 10px; color: #666; border-bottom: 1px solid #e8e8e8; }
.virtual-item:last-child { border-bottom: none; }
.virtual-icon { font-size: 11px; }
.virtual-name { flex: 1; font-weight: 500; }
.virtual-ip { color: #888; }

/* 连接监控 */
.connections-view { display: flex; flex-direction: column; gap: 8px; }
.conn-toolbar {
  background: #fff;
  padding: 8px;
  border-radius: 6px;
  border: 1px solid #e0e0e0;
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 8px;
}
.conn-stats { display: flex; gap: 6px; flex-wrap: wrap; }
.stat-badge {
  font-size: 10px;
  padding: 3px 8px;
  border-radius: 12px;
  background: #f5f5f5;
  color: #666;
  font-weight: 600;
}
.stat-badge.established { background: #e8f5e9; color: #2e7d32; }
.stat-badge.listening { background: #e3f2fd; color: #1565c0; }
.conn-filters { display: flex; gap: 6px; }
.filter-select, .search-input {
  font-size: 10px;
  padding: 4px 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
  outline: none;
}
.filter-select:focus, .search-input:focus { border-color: #1a73e8; }
.search-input { width: 120px; }

.conn-list {
  background: #fff;
  border-radius: 6px;
  border: 1px solid #e0e0e0;
  max-height: 400px;
  overflow-y: auto;
}
.conn-item {
  padding: 8px 10px;
  border-bottom: 1px solid #f0f0f0;
  transition: background 0.2s;
}
.conn-item:hover { background: #f8f9fa; }
.conn-item:last-child { border-bottom: none; }
.conn-process {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-bottom: 4px;
}
.process-name {
  font-size: 11px;
  font-weight: 600;
  color: #333;
}
.process-pid {
  font-size: 9px;
  color: #888;
  background: #f5f5f5;
  padding: 2px 6px;
  border-radius: 10px;
}
.conn-details {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 10px;
  font-family: monospace;
}
.conn-protocol {
  font-weight: 700;
  color: #666;
  background: #f0f0f0;
  padding: 2px 6px;
  border-radius: 3px;
}
.conn-address { color: #555; }
.conn-arrow { color: #999; }
.conn-state {
  font-size: 9px;
  padding: 2px 6px;
  border-radius: 10px;
  font-weight: 600;
  text-transform: uppercase;
}
.conn-state.established { background: #e8f5e9; color: #2e7d32; }
.conn-state.listening { background: #e3f2fd; color: #1565c0; }
.conn-state.time_wait { background: #fff3e0; color: #e65100; }
.conn-empty {
  padding: 40px 20px;
  text-align: center;
  color: #999;
  font-size: 11px;
}

/* 骨架屏 */
.skeleton-text { display: inline-block; height: 12px; width: 50px; background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%); background-size: 200% 100%; animation: shimmer 1.5s infinite; border-radius: 3px; }
.skeleton-list { display: flex; flex-direction: column; gap: 6px; }
.skeleton-card { height: 50px; background: linear-gradient(90deg, #f5f5f5 25%, #e8e8e8 50%, #f5f5f5 75%); background-size: 200% 100%; animation: shimmer 1.5s infinite; border-radius: 8px; }

/* Tooltip */
.chart-tooltip {
  position: absolute;
  background: rgba(0, 0, 0, 0.85);
  color: #fff;
  padding: 5px 8px;
  border-radius: 5px;
  font-size: 11px;
  font-weight: 600;
  pointer-events: none;
  transform: translateX(-50%);
  white-space: nowrap;
  z-index: 10;
}
.tooltip-row { display: flex; align-items: center; gap: 5px; line-height: 1.5; }
.tooltip-dot { width: 6px; height: 6px; border-radius: 50%; }
.tooltip-dot.down { background: #34a853; }
.tooltip-dot.up { background: #1a73e8; }

@keyframes shimmer { 0% { background-position: -200% 0; } 100% { background-position: 200% 0; } }
</style>
