<script setup>
import { ref, onMounted, watch } from 'vue'
import { setupCanvas, drawChart } from '../composables/useChart'

const props = defineProps({
  memoryInfo: Object,
  memoryHistory: Array
})

const canvasRef = ref(null)

function redraw() {
  if (canvasRef.value && props.memoryHistory.length > 0) {
    requestAnimationFrame(() => {
      drawChart(canvasRef.value, props.memoryHistory, '#34a853')
    })
  }
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
          <span class="trend-value memory">{{ memoryInfo.usedPercent }}</span>
        </div>
      </div>
      <div class="trend-chart-wrapper">
        <canvas ref="canvasRef" class="trend-chart-canvas"></canvas>
      </div>
    </div>

    <!-- 内存概览 -->
    <h3>内存概览</h3>
    <div class="memory-overview">
      <div class="memory-stat">
        <span class="memory-stat-label">总内存</span>
        <span class="memory-stat-value">{{ memoryInfo.total }}</span>
      </div>
      <div class="memory-stat">
        <span class="memory-stat-label">已使用</span>
        <span class="memory-stat-value used">{{ memoryInfo.used }}</span>
      </div>
      <div class="memory-stat">
        <span class="memory-stat-label">可用</span>
        <span class="memory-stat-value available">{{ memoryInfo.available }}</span>
      </div>
      <div class="memory-stat">
        <span class="memory-stat-label">使用率</span>
        <span class="memory-stat-value percent">{{ memoryInfo.usedPercent }}</span>
      </div>
    </div>

    <!-- 交换空间 -->
    <h3>交换空间</h3>
    <div class="swap-overview">
      <div class="swap-stat">
        <span class="swap-stat-label">总量</span>
        <span class="swap-stat-value">{{ memoryInfo.swapTotal }}</span>
      </div>
      <div class="swap-stat">
        <span class="swap-stat-label">已使用</span>
        <span class="swap-stat-value used">{{ memoryInfo.swapUsed }}</span>
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
</style>
