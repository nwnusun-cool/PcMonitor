<script setup>
import { ref, onMounted, watch } from 'vue'
import { setupCanvas, drawMultiLineChart } from '../composables/useChart'

const props = defineProps({
  cpuInfo: Object,
  cpuHistory: Array,
  cpuUserHistory: Array,
  cpuSystemHistory: Array
})

const canvasRef = ref(null)

function redraw() {
  if (canvasRef.value && props.cpuHistory.length > 0) {
    requestAnimationFrame(() => {
      drawMultiLineChart(canvasRef.value, [
        { data: props.cpuHistory, color: '#1a73e8' },
        { data: props.cpuUserHistory, color: '#fbbc04' },
        { data: props.cpuSystemHistory, color: '#ea4335' }
      ])
    })
  }
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
  <div class="panel">
    <!-- 使用率趋势 -->
    <h3>使用率趋势</h3>
    <div class="trend-section">
      <div class="trend-stats">
        <div class="trend-stat-item">
          <span class="trend-dot total"></span>
          <span class="trend-label">总使用率</span>
          <span class="trend-value total">{{ cpuInfo.load }}</span>
        </div>
        <div class="trend-stat-item">
          <span class="trend-dot user"></span>
          <span class="trend-label">用户</span>
          <span class="trend-value user">{{ cpuInfo.loadUser }}</span>
        </div>
        <div class="trend-stat-item">
          <span class="trend-dot system"></span>
          <span class="trend-label">系统</span>
          <span class="trend-value system">{{ cpuInfo.loadSystem }}</span>
        </div>
      </div>
      <div class="trend-chart-wrapper">
        <canvas ref="canvasRef" class="trend-chart-canvas"></canvas>
      </div>
    </div>

    <!-- 处理器概览 -->
    <h3>处理器概览</h3>
    <div class="cpu-brand-card">
      <div class="cpu-brand-icon">⚡</div>
      <div class="cpu-brand-info">
        <div class="cpu-brand-name">{{ cpuInfo.brand }}</div>
        <div class="cpu-brand-meta">{{ cpuInfo.manufacturer }}</div>
      </div>
    </div>

    <!-- 核心与频率 -->
    <h3>核心与频率</h3>
    <div class="cpu-specs-grid">
      <div class="cpu-spec-card">
        <span class="cpu-spec-label">物理核心</span>
        <span class="cpu-spec-value">{{ cpuInfo.physicalCores }}</span>
      </div>
      <div class="cpu-spec-card">
        <span class="cpu-spec-label">逻辑核心</span>
        <span class="cpu-spec-value">{{ cpuInfo.cores }}</span>
      </div>
      <div class="cpu-spec-card">
        <span class="cpu-spec-label">基础频率</span>
        <span class="cpu-spec-value">{{ cpuInfo.speed }}</span>
      </div>
      <div class="cpu-spec-card">
        <span class="cpu-spec-label">当前频率</span>
        <span class="cpu-spec-value highlight">{{ cpuInfo.currentSpeed }}</span>
      </div>
      <div class="cpu-spec-card">
        <span class="cpu-spec-label">虚拟化</span>
        <span class="cpu-spec-value">{{ cpuInfo.virtualization }}</span>
      </div>
    </div>

    <!-- 缓存信息 -->
    <h3 v-if="cpuInfo.cache?.l2 || cpuInfo.cache?.l3">缓存信息</h3>
    <div class="cpu-cache-grid" v-if="cpuInfo.cache?.l2 || cpuInfo.cache?.l3">
      <div class="cpu-cache-card" v-if="cpuInfo.cache?.l1d">
        <span class="cpu-cache-label">L1 数据</span>
        <span class="cpu-cache-value">{{ cpuInfo.cache.l1d }}</span>
      </div>
      <div class="cpu-cache-card" v-if="cpuInfo.cache?.l1i">
        <span class="cpu-cache-label">L1 指令</span>
        <span class="cpu-cache-value">{{ cpuInfo.cache.l1i }}</span>
      </div>
      <div class="cpu-cache-card" v-if="cpuInfo.cache?.l2">
        <span class="cpu-cache-label">L2 缓存</span>
        <span class="cpu-cache-value">{{ cpuInfo.cache.l2 }}</span>
      </div>
      <div class="cpu-cache-card" v-if="cpuInfo.cache?.l3">
        <span class="cpu-cache-label">L3 缓存</span>
        <span class="cpu-cache-value">{{ cpuInfo.cache.l3 }}</span>
      </div>
    </div>

    <!-- 温度 -->
    <div v-if="cpuInfo.temperature" class="cpu-temp-section">
      <h3>温度监控</h3>
      <div class="temp-display">
        <span class="temp-icon">🌡️</span>
        <span class="temp-value">{{ cpuInfo.temperature }}</span>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.cpu-brand-card {
  display: flex;
  align-items: center;
  gap: 14px;
  background: #fff;
  padding: 16px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 16px;
}

.cpu-brand-icon {
  font-size: 32px;
  width: 56px;
  height: 56px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #e8f0fe, #d2e3fc);
  border-radius: 12px;
}

.cpu-brand-info { flex: 1; }

.cpu-brand-name {
  font-size: 15px;
  font-weight: 700;
  color: #333;
  margin-bottom: 4px;
}

.cpu-brand-meta {
  font-size: 12px;
  color: #666;
}

.cpu-specs-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 10px;
  margin-bottom: 16px;
}

.cpu-spec-card {
  background: #fff;
  padding: 14px 12px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 70px;
  transition: all 0.2s;
}

.cpu-spec-card:hover { border-color: #1a73e8; }

.cpu-spec-label {
  font-size: 10px;
  color: #888;
  margin-bottom: 8px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.cpu-spec-value {
  font-size: 18px;
  font-weight: 700;
  color: #333;
  line-height: 1;
}

.cpu-spec-value.highlight { color: #1a73e8; }

.cpu-cache-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 10px;
  margin-bottom: 16px;
}

.cpu-cache-card {
  background: #fff;
  padding: 12px 10px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 65px;
}

.cpu-cache-label {
  font-size: 10px;
  color: #888;
  margin-bottom: 8px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.3px;
}

.cpu-cache-value {
  font-size: 14px;
  font-weight: 700;
  color: #9334e6;
  line-height: 1;
}

.cpu-temp-section { margin-top: 16px; }

.temp-display {
  background: #fff;
  padding: 16px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  display: flex;
  align-items: center;
  gap: 12px;
}

.temp-icon { font-size: 28px; }

.temp-value {
  font-size: 24px;
  font-weight: 700;
  color: #ea4335;
}
</style>
