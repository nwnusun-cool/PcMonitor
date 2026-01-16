<script setup>
import { ref, onMounted, watch } from 'vue'
import { setupCanvas, drawChartWithHighlight, getDataIndexAtPosition } from '../composables/useChart'

const props = defineProps({
  cpuInfo: Object,
  cpuHistory: Array,
  systemStats: Object
})

const canvasRef = ref(null)
const tooltipVisible = ref(false)
const tooltipX = ref(0)
const tooltipY = ref(0)
const tooltipValue = ref('')
const highlightIndex = ref(-1)

function redraw() {
  if (canvasRef.value && props.cpuHistory.length > 0) {
    requestAnimationFrame(() => {
      drawChartWithHighlight(canvasRef.value, props.cpuHistory, '#1a73e8', highlightIndex.value)
    })
  }
}

function handleMouseMove(e) {
  if (!canvasRef.value || props.cpuHistory.length === 0) return
  const rect = canvasRef.value.getBoundingClientRect()
  const mouseX = e.clientX - rect.left
  const index = getDataIndexAtPosition(canvasRef.value, mouseX, props.cpuHistory.length)
  if (index >= 0 && index < props.cpuHistory.length) {
    highlightIndex.value = index
    tooltipValue.value = props.cpuHistory[index].toFixed(1) + '%'
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

watch(() => props.cpuHistory.length, redraw)
defineExpose({ redraw, canvasRef })
</script>

<template>
  <div class="panel compact">
    <!-- 使用率趋势 -->
    <h3>CPU 使用率</h3>
    <div class="chart-section">
      <div class="chart-stats">
        <span class="chart-label">当前</span>
        <span class="chart-value" v-if="cpuInfo.load">{{ cpuInfo.load }}</span>
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

    <!-- 处理器 + 系统统计 -->
    <div class="two-col">
      <div class="col-section">
        <h3>处理器</h3>
        <div class="cpu-card">
          <span class="cpu-icon">⚡</span>
          <div class="cpu-info">
            <div class="cpu-name" v-if="cpuInfo.brand">{{ cpuInfo.brand }}</div>
            <div class="skeleton-text long" v-else></div>
            <div class="cpu-meta" v-if="cpuInfo.manufacturer">{{ cpuInfo.manufacturer }}</div>
          </div>
        </div>
        <div class="spec-grid">
          <div class="spec-item">
            <span class="spec-label">核心</span>
            <span class="spec-value" v-if="cpuInfo.physicalCores">{{ cpuInfo.physicalCores }}</span>
            <span class="skeleton-text short" v-else></span>
          </div>
          <div class="spec-item">
            <span class="spec-label">线程</span>
            <span class="spec-value" v-if="cpuInfo.cores">{{ cpuInfo.cores }}</span>
            <span class="skeleton-text short" v-else></span>
          </div>
          <div class="spec-item">
            <span class="spec-label">基频</span>
            <span class="spec-value" v-if="cpuInfo.speed">{{ cpuInfo.speed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="spec-item">
            <span class="spec-label">当前</span>
            <span class="spec-value highlight" v-if="cpuInfo.currentSpeed">{{ cpuInfo.currentSpeed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
      <div class="col-section">
        <h3>系统统计</h3>
        <div class="stats-grid">
          <div class="stat-item">
            <span class="stat-value" v-if="systemStats?.processCount">{{ systemStats.processCount }}</span>
            <span class="skeleton-text" v-else></span>
            <span class="stat-label">进程</span>
          </div>
          <div class="stat-item">
            <span class="stat-value" v-if="systemStats?.threadCount">{{ systemStats.threadCount.toLocaleString() }}</span>
            <span class="skeleton-text" v-else></span>
            <span class="stat-label">线程</span>
          </div>
          <div class="stat-item">
            <span class="stat-value" v-if="systemStats?.handleCount">{{ systemStats.handleCount.toLocaleString() }}</span>
            <span class="skeleton-text" v-else></span>
            <span class="stat-label">句柄</span>
          </div>
        </div>
        <!-- 缓存 -->
        <h3 v-if="cpuInfo.cache?.l2 || cpuInfo.cache?.l3">缓存</h3>
        <div class="cache-grid" v-if="cpuInfo.cache?.l2 || cpuInfo.cache?.l3">
          <div class="cache-item" v-if="cpuInfo.cache?.l1d">
            <span class="cache-label">L1d</span>
            <span class="cache-value">{{ cpuInfo.cache.l1d }}</span>
          </div>
          <div class="cache-item" v-if="cpuInfo.cache?.l1i">
            <span class="cache-label">L1i</span>
            <span class="cache-value">{{ cpuInfo.cache.l1i }}</span>
          </div>
          <div class="cache-item" v-if="cpuInfo.cache?.l2">
            <span class="cache-label">L2</span>
            <span class="cache-value">{{ cpuInfo.cache.l2 }}</span>
          </div>
          <div class="cache-item" v-if="cpuInfo.cache?.l3">
            <span class="cache-label">L3</span>
            <span class="cache-value">{{ cpuInfo.cache.l3 }}</span>
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
.chart-value { font-size: 16px; font-weight: 700; color: #1a73e8; }
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

/* CPU 卡片 */
.cpu-card {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 8px;
  background: #f8f9fa;
  border-radius: 6px;
  margin-bottom: 8px;
}
.cpu-icon {
  font-size: 20px;
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #e8f0fe, #d2e3fc);
  border-radius: 6px;
  flex-shrink: 0;
}
.cpu-info { flex: 1; min-width: 0; }
.cpu-name { font-size: 11px; font-weight: 600; color: #333; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.cpu-meta { font-size: 10px; color: #888; margin-top: 1px; }

/* 规格网格 */
.spec-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 6px;
}
.spec-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 6px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.spec-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; }
.spec-value { font-size: 12px; font-weight: 700; color: #333; margin-top: 2px; }
.spec-value.highlight { color: #1a73e8; }

/* 统计网格 */
.stats-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 6px;
  margin-bottom: 8px;
}
.stat-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 8px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.stat-value { font-size: 16px; font-weight: 700; color: #1a73e8; line-height: 1; }
.stat-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; margin-top: 4px; }

/* 缓存网格 */
.cache-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 6px;
}
.cache-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 6px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.cache-label { font-size: 9px; color: #888; font-weight: 600; }
.cache-value { font-size: 11px; font-weight: 700; color: #9334e6; margin-top: 2px; }

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
.skeleton-text.short { width: 30px; }
.skeleton-text.long { width: 80px; }

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
