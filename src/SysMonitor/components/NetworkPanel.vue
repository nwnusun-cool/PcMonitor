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
      <h3>网络接口</h3>
      <label class="toggle-virtual" v-if="networkInfo.virtualInterfaces?.length">
        <input type="checkbox" v-model="showVirtual" />
        <span>虚拟 ({{ networkInfo.virtualInterfaces.length }})</span>
      </label>
    </div>

    <div class="interfaces" v-if="networkInfo.interfaces?.length">
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
            <div class="detail-item" v-if="iface.gateway">
              <span class="detail-label">网关</span>
              <span class="detail-value" @click="copyText(iface.gateway, $event)">{{ iface.gateway }}</span>
            </div>
            <div class="detail-item">
              <span class="detail-label">MAC</span>
              <span class="detail-value" @click="copyText(iface.mac, $event)">{{ iface.mac }}</span>
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
          <span class="virtual-ip">{{ iface.ip4 || '-' }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.panel.compact h3 { margin: 0 0 8px; font-size: 12px; }
.panel.compact h3:not(:first-child) { margin-top: 10px; }

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
.iface-detail.show { max-height: 100px; padding: 8px 10px; }
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
