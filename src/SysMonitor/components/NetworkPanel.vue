<script setup>
import { ref, onMounted, watch } from 'vue'
import { setupCanvas, drawNetworkChart } from '../composables/useChart'

const props = defineProps({
  networkInfo: Object,
  networkDownHistory: Array,
  networkUpHistory: Array
})

const canvasRef = ref(null)

function redraw() {
  if (canvasRef.value && props.networkDownHistory.length > 0) {
    requestAnimationFrame(() => {
      drawNetworkChart(canvasRef.value, [
        { data: props.networkDownHistory, color: '#34a853' },
        { data: props.networkUpHistory, color: '#1a73e8' }
      ])
    })
  }
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
    <!-- 网络流量趋势 -->
    <h3>网络流量趋势</h3>
    <div class="network-traffic-section">
      <div class="traffic-stats">
        <div class="traffic-stat-item">
          <span class="traffic-label">下载</span>
          <span class="traffic-value download">{{ networkInfo.stats?.[0]?.rxSec || '0 B/s' }}</span>
        </div>
        <div class="traffic-stat-item">
          <span class="traffic-label">上传</span>
          <span class="traffic-value upload">{{ networkInfo.stats?.[0]?.txSec || '0 B/s' }}</span>
        </div>
      </div>
      <div class="network-chart-wrapper">
        <canvas ref="canvasRef" class="network-chart-canvas"></canvas>
      </div>
    </div>

    <!-- 流量统计概览 -->
    <h3>流量统计</h3>
    <div class="network-overview">
      <div class="network-stat">
        <span class="network-stat-label">总下载</span>
        <span class="network-stat-value download">{{ networkInfo.stats?.[0]?.rxBytes || '0 B' }}</span>
      </div>
      <div class="network-stat">
        <span class="network-stat-label">总上传</span>
        <span class="network-stat-value upload">{{ networkInfo.stats?.[0]?.txBytes || '0 B' }}</span>
      </div>
      <div class="network-stat">
        <span class="network-stat-label">下载速度</span>
        <span class="network-stat-value">{{ networkInfo.stats?.[0]?.rxSec || '0 B/s' }}</span>
      </div>
      <div class="network-stat">
        <span class="network-stat-label">上传速度</span>
        <span class="network-stat-value">{{ networkInfo.stats?.[0]?.txSec || '0 B/s' }}</span>
      </div>
    </div>

    <!-- 网络接口 -->
    <h3>网络接口</h3>
    <div class="network-interfaces">
      <div v-for="iface in networkInfo.interfaces" :key="iface.iface" class="interface-item">
        <div class="interface-icon">{{ iface.type === 'wireless' ? '📶' : '🔌' }}</div>
        <div class="interface-info">
          <div class="interface-name">{{ iface.iface }}</div>
          <div class="interface-meta">{{ iface.type }} · {{ iface.speed }}</div>
        </div>
        <div class="interface-details">
          <div class="interface-ip">{{ iface.ip4 }}</div>
          <div class="interface-mac">{{ iface.mac }}</div>
        </div>
      </div>
    </div>

    <!-- 各接口流量 -->
    <h3>接口流量</h3>
    <div class="interface-traffic-list">
      <div v-for="stat in networkInfo.stats" :key="stat.iface" class="interface-traffic-item">
        <span class="interface-traffic-name">{{ stat.iface }}</span>
        <div class="interface-traffic-stats">
          <span class="interface-traffic-down">↓ {{ stat.rxSec }}</span>
          <span class="interface-traffic-up">↑ {{ stat.txSec }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.network-traffic-section {
  background: #fff;
  padding: 16px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 16px;
}

.traffic-stats {
  display: flex;
  gap: 24px;
  margin-bottom: 12px;
}

.traffic-stat-item {
  display: flex;
  align-items: center;
  gap: 8px;
}

.traffic-label {
  font-size: 12px;
  color: #666;
  font-weight: 500;
}

.traffic-value {
  font-size: 16px;
  font-weight: 700;
}

.traffic-value.download { color: #34a853; }
.traffic-value.upload { color: #1a73e8; }

.network-chart-wrapper { height: 120px; }

.network-chart-canvas {
  width: 100%;
  height: 100%;
  display: block;
}

.network-overview {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 12px;
  margin-bottom: 16px;
}

.network-stat {
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

.network-stat-label {
  font-size: 11px;
  color: #888;
  margin-bottom: 10px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.network-stat-value {
  font-size: 18px;
  font-weight: 700;
  color: #333;
  line-height: 1;
}

.network-stat-value.download { color: #34a853; }
.network-stat-value.upload { color: #1a73e8; }

.network-interfaces {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-bottom: 16px;
}

.interface-item {
  display: flex;
  align-items: center;
  gap: 12px;
  background: #fff;
  padding: 12px 14px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  transition: all 0.2s;
}

.interface-item:hover { border-color: #1a73e8; }

.interface-icon {
  font-size: 24px;
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #f5f5f5;
  border-radius: 8px;
}

.interface-info {
  flex: 1;
  min-width: 0;
}

.interface-name {
  font-size: 13px;
  font-weight: 600;
  color: #333;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.interface-meta {
  font-size: 11px;
  color: #999;
  margin-top: 2px;
}

.interface-details { text-align: right; }

.interface-ip {
  font-size: 12px;
  font-weight: 600;
  color: #1a73e8;
}

.interface-mac {
  font-size: 10px;
  color: #999;
  margin-top: 2px;
}

.interface-traffic-list {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.interface-traffic-item {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 10px 12px;
  background: #fff;
  border-radius: 6px;
  border: 1px solid #e0e0e0;
}

.interface-traffic-name {
  font-size: 12px;
  font-weight: 500;
  color: #333;
  flex: 1;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.interface-traffic-stats {
  display: flex;
  gap: 16px;
}

.interface-traffic-down {
  font-size: 12px;
  font-weight: 600;
  color: #34a853;
  min-width: 80px;
  text-align: right;
}

.interface-traffic-up {
  font-size: 12px;
  font-weight: 600;
  color: #1a73e8;
  min-width: 80px;
  text-align: right;
}
</style>
