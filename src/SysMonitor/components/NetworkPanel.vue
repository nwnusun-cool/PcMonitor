<script setup>
import { ref, onMounted, watch } from 'vue'
import { setupCanvas, drawNetworkChartWithHighlight, getDataIndexAtPosition, formatIOValue } from '../composables/useChart'

const props = defineProps({
  networkInfo: Object,
  networkDownHistory: Array,
  networkUpHistory: Array
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
    tooltipY.value = e.clientY - rect.top - 45
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
  <div class="panel">
    <h3>网络流量趋势</h3>
    <div class="network-traffic-section">
      <div class="traffic-stats">
        <div class="traffic-stat-item">
          <span class="traffic-label">下载</span>
          <span class="traffic-value download" v-if="networkInfo.stats?.[0]">{{ networkInfo.stats[0].rxSec }}</span>
          <span class="skeleton-text" v-else></span>
        </div>
        <div class="traffic-stat-item">
          <span class="traffic-label">上传</span>
          <span class="traffic-value upload" v-if="networkInfo.stats?.[0]">{{ networkInfo.stats[0].txSec }}</span>
          <span class="skeleton-text" v-else></span>
        </div>
      </div>
      <div class="network-chart-wrapper">
        <canvas 
          ref="canvasRef" 
          class="network-chart-canvas"
          @mousemove="handleMouseMove"
          @mouseleave="handleMouseLeave"
        ></canvas>
        <div 
          v-if="tooltipVisible" 
          class="chart-tooltip"
          :style="{ left: tooltipX + 'px', top: tooltipY + 'px' }"
        >
          <div class="tooltip-row"><span class="tooltip-dot download"></span>↓ {{ tooltipDown }}</div>
          <div class="tooltip-row"><span class="tooltip-dot upload"></span>↑ {{ tooltipUp }}</div>
        </div>
      </div>
    </div>

    <div class="section-header">
      <h3>网络接口</h3>
      <label class="toggle-virtual" v-if="networkInfo.virtualInterfaces?.length">
        <input type="checkbox" v-model="showVirtual" />
        <span>显示虚拟网卡 ({{ networkInfo.virtualInterfaces.length }})</span>
      </label>
    </div>

    <div class="network-interfaces" v-if="networkInfo.interfaces?.length">
      <div v-for="iface in networkInfo.interfaces" :key="iface.iface" 
           class="interface-card" 
           :class="{ 'is-default': iface.isDefault, 'expanded': expandedCard === iface.iface }"
           @click="toggleCard(iface.iface)">
        <div class="interface-header">
          <div class="interface-icon-wrap">
            <span class="interface-icon">{{ iface.type === 'wireless' ? '📶' : '🔌' }}</span>
            <span class="status-dot" :class="iface.isUp ? 'online' : 'offline'"></span>
          </div>
          <div class="interface-title">
            <div class="interface-name">
              {{ iface.ifaceName }}
              <span class="badge default" v-if="iface.isDefault">主网卡</span>
            </div>
            <div class="interface-meta">{{ iface.ip4 }} · {{ iface.type }}</div>
          </div>
          <div class="interface-right">
            <div class="interface-traffic-mini">
              <span class="mini-down">↓{{ iface.rxSec || '0 B/s' }}</span>
              <span class="mini-up">↑{{ iface.txSec || '0 B/s' }}</span>
            </div>
            <span class="expand-icon">{{ expandedCard === iface.iface ? '▲' : '▼' }}</span>
          </div>
        </div>
        
        <div class="interface-detail" :class="{ 'show': expandedCard === iface.iface }">
          <div class="detail-grid">
            <div class="detail-item">
              <span class="detail-label">IPv4</span>
              <span class="detail-value copyable" @click="copyText(iface.ip4, $event)">{{ iface.ip4 }}<span class="subnet" v-if="iface.ip4subnet">/{{ iface.ip4subnet }}</span></span>
            </div>
            <div class="detail-item" v-if="iface.gateway">
              <span class="detail-label">网关</span>
              <span class="detail-value copyable" @click="copyText(iface.gateway, $event)">{{ iface.gateway }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">MAC</span>
              <span class="detail-value copyable" @click="copyText(iface.mac, $event)">{{ iface.mac }}</span>
            </div>
            <div class="detail-item" v-if="iface.ip6">
              <span class="detail-label">IPv6</span>
              <span class="detail-value ip6 copyable" @click="copyText(iface.ip6, $event)">{{ iface.ip6 }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="skeleton-list" v-else>
      <div class="skeleton-card" v-for="i in 2" :key="i"></div>
    </div>

    <div v-if="showVirtual && networkInfo.virtualInterfaces?.length" class="virtual-section">
      <div class="virtual-list">
        <div v-for="iface in networkInfo.virtualInterfaces" :key="iface.iface" class="virtual-item">
          <span class="virtual-icon">🔗</span>
          <span class="virtual-name">{{ iface.ifaceName }}</span>
          <span class="virtual-ip">{{ iface.ip4 || '无 IP' }}</span>
          <span class="virtual-mac">{{ iface.mac }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.section-header { display: flex; align-items: center; justify-content: space-between; margin-bottom: 12px; }
.section-header h3 { margin: 0; }
.toggle-virtual { display: flex; align-items: center; gap: 6px; font-size: 12px; color: #666; cursor: pointer; }
.toggle-virtual input { cursor: pointer; }

.network-traffic-section { background: #fff; padding: 16px; border-radius: 8px; border: 1px solid #e0e0e0; margin-bottom: 16px; }
.traffic-stats { display: flex; gap: 24px; margin-bottom: 12px; }
.traffic-stat-item { display: flex; align-items: center; gap: 8px; }
.traffic-label { font-size: 12px; color: #666; font-weight: 500; }
.traffic-value { font-size: 16px; font-weight: 700; }
.traffic-value.download { color: #34a853; }
.traffic-value.upload { color: #1a73e8; }
.network-chart-wrapper { height: 120px; }
.network-chart-canvas { width: 100%; height: 100%; display: block; }

.network-interfaces { display: flex; flex-direction: column; gap: 8px; }
.interface-card { background: #fff; border-radius: 10px; border: 1px solid #e0e0e0; overflow: hidden; cursor: pointer; will-change: transform; }
.interface-card:hover { border-color: #1a73e8; }
.interface-card.is-default { border-color: #1a73e8; border-width: 2px; }
.interface-card.expanded { box-shadow: 0 2px 12px rgba(26, 115, 232, 0.15); }

.interface-header { display: flex; align-items: center; gap: 12px; padding: 12px 14px; }
.interface-icon-wrap { position: relative; }
.interface-icon { font-size: 24px; width: 40px; height: 40px; display: flex; align-items: center; justify-content: center; background: #f5f5f5; border-radius: 8px; }
.status-dot { position: absolute; bottom: -2px; right: -2px; width: 10px; height: 10px; border-radius: 50%; border: 2px solid #fff; }
.status-dot.online { background: #34a853; }
.status-dot.offline { background: #999; }

.interface-title { flex: 1; min-width: 0; }
.interface-name { font-size: 13px; font-weight: 600; color: #333; display: flex; align-items: center; gap: 6px; }
.badge { font-size: 10px; padding: 2px 6px; border-radius: 4px; font-weight: 500; }
.badge.default { background: #e8f0fe; color: #1a73e8; }
.interface-meta { font-size: 11px; color: #888; margin-top: 2px; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }

.interface-right { display: flex; align-items: center; gap: 12px; }
.interface-traffic-mini { display: flex; flex-direction: column; align-items: flex-end; gap: 2px; font-size: 11px; font-weight: 600; }
.mini-down { color: #34a853; }
.mini-up { color: #1a73e8; }
.expand-icon { font-size: 10px; color: #999; }

.interface-detail { 
  border-top: 1px solid #e8e8e8; 
  padding: 0 14px; 
  background: #fafafa; 
  max-height: 0; 
  overflow: hidden; 
  transition: max-height 0.2s ease-out, padding 0.2s ease-out;
}
.interface-detail.show { 
  max-height: 300px; 
  padding: 12px 14px; 
}
.detail-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 8px 16px; margin-bottom: 12px; }
.detail-item { display: flex; flex-direction: column; gap: 2px; }
.detail-label { font-size: 10px; color: #888; text-transform: uppercase; font-weight: 600; }
.detail-value { font-size: 12px; color: #333; font-family: 'SF Mono', Monaco, monospace; }
.detail-value.copyable { cursor: pointer; }
.detail-value.copyable:hover { color: #1a73e8; }
.detail-value .subnet { color: #888; }
.detail-value.ip6 { font-size: 10px; word-break: break-all; grid-column: span 2; }

.virtual-section { margin-top: 12px; }
.virtual-list { background: #f8f9fa; border-radius: 8px; padding: 8px; }
.virtual-item { display: flex; align-items: center; gap: 10px; padding: 8px 10px; font-size: 12px; color: #666; border-bottom: 1px solid #e8e8e8; }
.virtual-item:last-child { border-bottom: none; }
.virtual-icon { font-size: 14px; }
.virtual-name { flex: 1; font-weight: 500; }
.virtual-ip { color: #888; min-width: 100px; }
.virtual-mac { color: #aaa; font-family: monospace; font-size: 11px; }

.skeleton-text { display: inline-block; height: 14px; width: 70px; background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%); background-size: 200% 100%; animation: shimmer 1.5s infinite; border-radius: 4px; }
.skeleton-list { display: flex; flex-direction: column; gap: 12px; }
.skeleton-card { height: 180px; background: linear-gradient(90deg, #f5f5f5 25%, #e8e8e8 50%, #f5f5f5 75%); background-size: 200% 100%; animation: shimmer 1.5s infinite; border-radius: 10px; }

@keyframes shimmer {
  0% { background-position: -200% 0; }
  100% { background-position: 200% 0; }
}

.chart-tooltip {
  position: absolute;
  background: rgba(0, 0, 0, 0.85);
  color: #fff;
  padding: 6px 10px;
  border-radius: 6px;
  font-size: 12px;
  font-weight: 600;
  pointer-events: none;
  transform: translateX(-50%);
  white-space: nowrap;
  z-index: 10;
}
.tooltip-row { display: flex; align-items: center; gap: 6px; line-height: 1.6; }
.tooltip-dot { width: 8px; height: 8px; border-radius: 50%; }
.tooltip-dot.download { background: #34a853; }
.tooltip-dot.upload { background: #1a73e8; }
</style>
