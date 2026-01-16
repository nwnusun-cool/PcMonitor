<script setup>
import { ref, onMounted, watch } from 'vue'
import { setupCanvas, drawChartWithHighlight, getDataIndexAtPosition } from '../composables/useChart'

const props = defineProps({
  memoryInfo: Object,
  memoryHistory: Array
})

const canvasRef = ref(null)
const tooltipVisible = ref(false)
const tooltipX = ref(0)
const tooltipY = ref(0)
const tooltipValue = ref('')
const highlightIndex = ref(-1)

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

    <!-- 内存 + 交换空间 -->
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
            <span class="mem-label">使用率</span>
            <span class="mem-value percent" v-if="memoryInfo.usedPercent">{{ memoryInfo.usedPercent }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
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

/* 两栏布局 */
.two-col {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 10px;
}
.col-section {
  background: #fff;
  padding: 10px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
}
.col-section h3 { margin: 0 0 8px !important; }

/* 内存网格 */
.mem-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 6px;
}
.mem-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 8px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.mem-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; }
.mem-value { font-size: 13px; font-weight: 700; color: #333; margin-top: 4px; }
.mem-value.used { color: #ea4335; }
.mem-value.available { color: #34a853; }
.mem-value.percent { color: #34a853; }

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
.swap-value { font-size: 13px; font-weight: 700; color: #333; margin-top: 4px; }
.swap-value.used { color: #9334e6; }

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
