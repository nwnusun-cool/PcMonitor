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
    tooltipY.value = e.clientY - rect.top - 35
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
  <div class="panel">
    <!-- 使用率趋势 -->
    <h3>使用率趋势</h3>
    <div class="trend-section memory">
      <div class="trend-stats">
        <div class="trend-stat-item">
          <span class="trend-dot memory"></span>
          <span class="trend-label">内存使用率</span>
          <span class="trend-value memory" v-if="memoryInfo.usedPercent">{{ memoryInfo.usedPercent }}</span>
          <span class="skeleton-text" v-else></span>
        </div>
      </div>
      <div class="trend-chart-wrapper">
        <canvas 
          ref="canvasRef" 
          class="trend-chart-canvas"
          @mousemove="handleMouseMove"
          @mouseleave="handleMouseLeave"
        ></canvas>
        <div 
          v-if="tooltipVisible" 
          class="chart-tooltip"
          :style="{ left: tooltipX + 'px', top: tooltipY + 'px' }"
        >
          {{ tooltipValue }}
        </div>
      </div>
    </div>

    <!-- 内存概览 -->
    <h3>内存概览</h3>
    <div class="memory-overview">
      <div class="memory-stat">
        <span class="memory-stat-label">总内存</span>
        <span class="memory-stat-value" v-if="memoryInfo.total">{{ memoryInfo.total }}</span>
        <span class="skeleton-text" v-else></span>
      </div>
      <div class="memory-stat">
        <span class="memory-stat-label">已使用</span>
        <span class="memory-stat-value used" v-if="memoryInfo.used">{{ memoryInfo.used }}</span>
        <span class="skeleton-text" v-else></span>
      </div>
      <div class="memory-stat">
        <span class="memory-stat-label">可用</span>
        <span class="memory-stat-value available" v-if="memoryInfo.available">{{ memoryInfo.available }}</span>
        <span class="skeleton-text" v-else></span>
      </div>
      <div class="memory-stat">
        <span class="memory-stat-label">使用率</span>
        <span class="memory-stat-value percent" v-if="memoryInfo.usedPercent">{{ memoryInfo.usedPercent }}</span>
        <span class="skeleton-text" v-else></span>
      </div>
    </div>

    <!-- 交换空间 -->
    <h3>交换空间</h3>
    <div class="swap-overview">
      <div class="swap-stat">
        <span class="swap-stat-label">总量</span>
        <span class="swap-stat-value" v-if="memoryInfo.swapTotal">{{ memoryInfo.swapTotal }}</span>
        <span class="skeleton-text" v-else></span>
      </div>
      <div class="swap-stat">
        <span class="swap-stat-label">已使用</span>
        <span class="swap-stat-value used" v-if="memoryInfo.swapUsed">{{ memoryInfo.swapUsed }}</span>
        <span class="skeleton-text" v-else></span>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.memory-overview {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 12px;
  margin-bottom: 16px;
}

.memory-stat {
  background: #fff;
  padding: 16px 12px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 80px;
}

.memory-stat-label {
  font-size: 11px;
  color: #888;
  margin-bottom: 10px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.memory-stat-value {
  font-size: 20px;
  font-weight: 700;
  color: #333;
  line-height: 1;
}

.memory-stat-value.used { color: #ea4335; }
.memory-stat-value.available { color: #34a853; }
.memory-stat-value.percent { color: #34a853; }

.swap-overview {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 12px;
}

.swap-stat {
  background: #fff;
  padding: 16px 12px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 80px;
}

.swap-stat-label {
  font-size: 11px;
  color: #888;
  margin-bottom: 10px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.swap-stat-value {
  font-size: 20px;
  font-weight: 700;
  color: #333;
  line-height: 1;
}

.swap-stat-value.used { color: #9334e6; }

/* 骨架屏 */
.skeleton-text {
  display: inline-block;
  height: 16px;
  width: 70px;
  background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%);
  background-size: 200% 100%;
  animation: shimmer 1.5s infinite;
  border-radius: 4px;
}

@keyframes shimmer {
  0% { background-position: -200% 0; }
  100% { background-position: 200% 0; }
}

.chart-tooltip {
  position: absolute;
  background: rgba(0, 0, 0, 0.8);
  color: #fff;
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 12px;
  font-weight: 600;
  pointer-events: none;
  transform: translateX(-50%);
  white-space: nowrap;
  z-index: 10;
}
</style>
