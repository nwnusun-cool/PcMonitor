<script setup>
import { ref, onMounted, watch, computed } from 'vue'
import { setupCanvas, drawChartWithHighlight, getDataIndexAtPosition } from '../composables/useChart'

const props = defineProps({
  memoryInfo: Object,
  memoryHistory: Array,
  memoryHardware: Object
})

const canvasRef = ref(null)
const tooltipVisible = ref(false)
const tooltipX = ref(0)
const tooltipY = ref(0)
const tooltipValue = ref('')
const highlightIndex = ref(-1)

// 内存组成计算
const memoryComposition = computed(() => {
  const info = props.memoryInfo
  if (!info?.totalRaw) return []
  
  const total = info.totalRaw
  const used = info.usedRaw || 0
  const cached = info.cachedRaw || 0
  const free = info.freeRaw || 0
  
  // 简化为：已用、缓存、可用
  const inUse = Math.max(0, used - cached)
  
  return [
    { label: '使用中', value: inUse, percent: (inUse / total * 100).toFixed(1), color: '#ea4335' },
    { label: '缓存', value: cached, percent: (cached / total * 100).toFixed(1), color: '#fbbc04' },
    { label: '可用', value: free, percent: (free / total * 100).toFixed(1), color: '#34a853' }
  ]
})

function redraw() {
  if (canvasRef.value && props.memoryHistory.length > 0) {
    requestAnimationFrame(() => {
      drawChartWithHighlight(canvasRef.value, props.memoryHistory, '#34a853', highlightIndex.value)
    })
  }
}

function handleMouseMove(e) {
  if (!canvasRef.value || props.memoryHistory.length === 0) return
  const rect = canvasRef.value.getBoundingClientRect()
  const mouseX = e.clientX - rect.left
  const index = getDataIndexAtPosition(canvasRef.value, mouseX, props.memoryHistory.length)
  if (index >= 0 && index < props.memoryHistory.length) {
    highlightIndex.value = index
    tooltipValue.value = props.memoryHistory[index].toFixed(1) + '%'
    tooltipX.value = e.clientX - rect.left
    tooltipY.value = e.clientY - rect.top - 30
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
  if (canvasRef.value) {
    setupCanvas(canvasRef.value)
    redraw()
  }
})

watch(() => props.memoryHistory.length, redraw)
defineExpose({ redraw, canvasRef })
</script>

<template>
  <div class="panel compact">
    <!-- 使用率趋势 -->
    <h3>内存使用率</h3>
    <div class="chart-section">
      <div class="chart-stats">
        <span class="chart-label">当前</span>
        <span class="chart-value" v-if="memoryInfo.usedPercent">{{ memoryInfo.usedPercent }}</span>
        <span class="skeleton-text" v-else></span>
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
        >{{ tooltipValue }}</div>
      </div>
    </div>

    <!-- 内存组成可视化 -->
    <div class="composition-section" v-if="memoryComposition.length">
      <h3>内存组成</h3>
      <div class="composition-bar">
        <div 
          v-for="item in memoryComposition" 
          :key="item.label"
          class="composition-segment"
          :style="{ width: item.percent + '%', background: item.color }"
          :title="`${item.label}: ${item.percent}%`"
        ></div>
      </div>
      <div class="composition-legend">
        <div v-for="item in memoryComposition" :key="item.label" class="legend-item">
          <span class="legend-dot" :style="{ background: item.color }"></span>
          <span class="legend-label">{{ item.label }}</span>
          <span class="legend-value">{{ item.percent }}%</span>
        </div>
      </div>
    </div>

    <!-- 物理内存 + 详细信息 -->
    <div class="two-col">
      <div class="col-section">
        <h3>物理内存</h3>
        <div class="mem-grid">
          <div class="mem-item">
            <span class="mem-label">总量</span>
            <span class="mem-value" v-if="memoryInfo.total">{{ memoryInfo.total }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="mem-item">
            <span class="mem-label">已用</span>
            <span class="mem-value used" v-if="memoryInfo.used">{{ memoryInfo.used }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="mem-item">
            <span class="mem-label">可用</span>
            <span class="mem-value available" v-if="memoryInfo.available">{{ memoryInfo.available }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="mem-item">
            <span class="mem-label">缓存</span>
            <span class="mem-value cached" v-if="memoryInfo.cached">{{ memoryInfo.cached }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
      <div class="col-section">
        <h3>详细信息</h3>
        <div class="detail-grid">
          <div class="detail-item">
            <span class="detail-label">已提交</span>
            <span class="detail-value" v-if="memoryInfo.committed">{{ memoryInfo.committed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="detail-item">
            <span class="detail-label">提交限制</span>
            <span class="detail-value" v-if="memoryInfo.commitLimit">{{ memoryInfo.commitLimit }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="detail-item">
            <span class="detail-label">分页池</span>
            <span class="detail-value" v-if="memoryInfo.pagedPool">{{ memoryInfo.pagedPool }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="detail-item">
            <span class="detail-label">非分页池</span>
            <span class="detail-value" v-if="memoryInfo.nonPagedPool">{{ memoryInfo.nonPagedPool }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
    </div>

    <!-- 交换空间 + 硬件信息 -->
    <div class="two-col">
      <div class="col-section">
        <h3>交换空间</h3>
        <div class="swap-grid">
          <div class="swap-item">
            <span class="swap-label">总量</span>
            <span class="swap-value" v-if="memoryInfo.swapTotal">{{ memoryInfo.swapTotal }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="swap-item">
            <span class="swap-label">已用</span>
            <span class="swap-value used" v-if="memoryInfo.swapUsed">{{ memoryInfo.swapUsed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
      <div class="col-section">
        <h3>硬件信息</h3>
        <div class="hardware-info" v-if="memoryHardware?.modules?.length">
          <div class="hw-row">
            <span class="hw-label">类型</span>
            <span class="hw-value">{{ memoryHardware.type }} {{ memoryHardware.speed }} MHz</span>
          </div>
          <div class="hw-row">
            <span class="hw-label">插槽</span>
            <span class="hw-value">{{ memoryHardware.usedSlots }} / {{ memoryHardware.totalSlots }} 已用</span>
          </div>
          <div class="hw-row">
            <span class="hw-label">厂商</span>
            <span class="hw-value">{{ memoryHardware.modules[0]?.manufacturer || 'Unknown' }}</span>
          </div>
          <div class="hw-row">
            <span class="hw-label">规格</span>
            <span class="hw-value">{{ memoryHardware.modules.length }}x {{ memoryHardware.modules[0]?.capacity }}</span>
          </div>
        </div>
        <div class="hardware-skeleton" v-else>
          <div class="skeleton-row" v-for="i in 4" :key="i"></div>
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
.chart-stats { display: flex; align-items: center; gap: 8px; margin-bottom: 8px; }
.chart-label { font-size: 11px; color: #666; font-weight: 500; }
.chart-value { font-size: 16px; font-weight: 700; color: #34a853; }
.chart-wrapper { height: 100px; position: relative; }
.chart-canvas { width: 100%; height: 100%; display: block; }

/* 内存组成 */
.composition-section {
  background: #fff;
  padding: 12px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 10px;
}
.composition-bar {
  display: flex;
  height: 20px;
  border-radius: 4px;
  overflow: hidden;
  margin-bottom: 8px;
}
.composition-segment {
  transition: width 0.3s ease;
}
.composition-legend {
  display: flex;
  gap: 16px;
  justify-content: center;
}
.legend-item {
  display: flex;
  align-items: center;
  gap: 4px;
  font-size: 11px;
}
.legend-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}
.legend-label { color: #666; }
.legend-value { font-weight: 600; color: #333; }

/* 两栏布局 */
.two-col {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
  margin-bottom: 10px;
}
.col-section {
  background: #fff;
  padding: 10px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
}
.col-section h3 { margin: 0 0 8px !important; }

/* 内存网格 */
.mem-grid, .detail-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 6px;
}
.mem-item, .detail-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 8px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.mem-label, .detail-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; }
.mem-value, .detail-value { font-size: 12px; font-weight: 700; color: #333; margin-top: 4px; }
.mem-value.used { color: #ea4335; }
.mem-value.available { color: #34a853; }
.mem-value.cached { color: #fbbc04; }

/* 交换空间网格 */
.swap-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 6px;
}
.swap-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 8px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.swap-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; }
.swap-value { font-size: 12px; font-weight: 700; color: #333; margin-top: 4px; }
.swap-value.used { color: #9334e6; }

/* 硬件信息 */
.hardware-info {
  display: flex;
  flex-direction: column;
  gap: 6px;
}
.hw-row {
  display: flex;
  justify-content: space-between;
  padding: 6px 8px;
  background: #f8f9fa;
  border-radius: 4px;
}
.hw-label { font-size: 10px; color: #888; font-weight: 600; }
.hw-value { font-size: 10px; font-weight: 600; color: #333; }
.hardware-skeleton {
  display: flex;
  flex-direction: column;
  gap: 6px;
}
.skeleton-row {
  height: 24px;
  background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%);
  background-size: 200% 100%;
  animation: shimmer 1.5s infinite;
  border-radius: 4px;
}

/* 骨架屏 */
.skeleton-text {
  display: inline-block;
  height: 12px;
  width: 50px;
  background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%);
  background-size: 200% 100%;
  animation: shimmer 1.5s infinite;
  border-radius: 3px;
}

/* Tooltip */
.chart-tooltip {
  position: absolute;
  background: rgba(0, 0, 0, 0.85);
  color: #fff;
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 11px;
  font-weight: 600;
  pointer-events: none;
  transform: translateX(-50%);
  white-space: nowrap;
  z-index: 10;
}

@keyframes shimmer { 0% { background-position: -200% 0; } 100% { background-position: 200% 0; } }
</style>
