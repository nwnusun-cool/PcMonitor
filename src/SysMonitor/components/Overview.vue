<script setup>
defineProps({
  cpuInfo: Object,
  memoryInfo: Object,
  diskInfo: Object,
  networkInfo: Object,
  systemInfo: Object,
  batteryInfo: Object,
  processInfo: Object,
  uptime: String
})
</script>

<template>
  <div class="panel">
    <!-- 核心指标卡片 -->
    <div class="metrics-grid">
      <div class="metric-card cpu">
        <div class="metric-icon">⚡</div>
        <div class="metric-content">
          <div class="metric-label">CPU使用率</div>
          <div class="metric-value">{{ cpuInfo.load }}</div>
          <div class="metric-detail">{{ cpuInfo.brand }}</div>
        </div>
      </div>
      
      <div class="metric-card memory">
        <div class="metric-icon">💾</div>
        <div class="metric-content">
          <div class="metric-label">内存使用率</div>
          <div class="metric-value">{{ memoryInfo.usedPercent }}</div>
          <div class="metric-detail">{{ memoryInfo.used }} / {{ memoryInfo.total }}</div>
        </div>
      </div>
      
      <div class="metric-card disk">
        <div class="metric-icon">💿</div>
        <div class="metric-content">
          <div class="metric-label">磁盘使用</div>
          <div class="metric-value">{{ diskInfo.totalPercent || '0%' }}</div>
          <div class="metric-detail">{{ diskInfo.totalUsed || '0 B' }} / {{ diskInfo.totalSize || '0 B' }}</div>
        </div>
      </div>
      
      <div class="metric-card network">
        <div class="metric-icon">🌐</div>
        <div class="metric-content">
          <div class="metric-label">网络流量</div>
          <div class="metric-value">↓ {{ networkInfo.stats?.[0]?.rxSec || '0 B/s' }}</div>
          <div class="metric-detail">↑ {{ networkInfo.stats?.[0]?.txSec || '0 B/s' }}</div>
        </div>
      </div>
    </div>

    <!-- 系统信息 -->
    <h3>系统信息</h3>
    <div class="info-grid">
      <div class="info-item">
        <span class="label">操作系统</span>
        <span class="value">{{ systemInfo.platform }} {{ systemInfo.arch }}</span>
      </div>
      <div class="info-item">
        <span class="label">主机名</span>
        <span class="value">{{ systemInfo.hostname }}</span>
      </div>
      <div class="info-item">
        <span class="label">运行时间</span>
        <span class="value">{{ uptime }}</span>
      </div>
      <div class="info-item">
        <span class="label">设备型号</span>
        <span class="value">{{ systemInfo.manufacturer }} {{ systemInfo.model }}</span>
      </div>
    </div>

    <!-- CPU信息 -->
    <h3>处理器</h3>
    <div class="info-grid">
      <div class="info-item">
        <span class="label">型号</span>
        <span class="value">{{ cpuInfo.brand }}</span>
      </div>
      <div class="info-item">
        <span class="label">核心数</span>
        <span class="value">{{ cpuInfo.physicalCores }} 物理 / {{ cpuInfo.cores }} 逻辑</span>
      </div>
      <div class="info-item">
        <span class="label">当前频率</span>
        <span class="value">{{ cpuInfo.currentSpeed }}</span>
      </div>
      <div class="info-item" v-if="cpuInfo.temperature">
        <span class="label">温度</span>
        <span class="value">{{ cpuInfo.temperature }}</span>
      </div>
    </div>

    <!-- 内存信息 -->
    <h3>内存</h3>
    <div class="info-grid">
      <div class="info-item">
        <span class="label">总内存</span>
        <span class="value">{{ memoryInfo.total }}</span>
      </div>
      <div class="info-item">
        <span class="label">已使用</span>
        <span class="value">{{ memoryInfo.used }}</span>
      </div>
      <div class="info-item">
        <span class="label">可用</span>
        <span class="value">{{ memoryInfo.available }}</span>
      </div>
      <div class="info-item">
        <span class="label">使用率</span>
        <span class="value">{{ memoryInfo.usedPercent }}</span>
      </div>
    </div>

    <!-- 电池信息 -->
    <div v-if="batteryInfo.hasBattery">
      <h3>电池</h3>
      <div class="battery-status">
        <div class="battery-icon" :class="{ charging: batteryInfo.isCharging }">
          <div class="battery-level" :style="{ width: batteryInfo.percent }"></div>
        </div>
        <div class="battery-info">
          <div class="battery-percent">{{ batteryInfo.percent }}</div>
          <div class="battery-state">{{ batteryInfo.isCharging ? '正在充电' : '使用电池' }}</div>
        </div>
      </div>
    </div>

    <!-- 进程统计 -->
    <h3>进程统计</h3>
    <div class="process-stats">
      <div class="stat-item">
        <div class="stat-label">总进程数</div>
        <div class="stat-value">{{ processInfo.all }}</div>
      </div>
      <div class="stat-item">
        <div class="stat-label">运行中</div>
        <div class="stat-value running">{{ processInfo.running }}</div>
      </div>
      <div class="stat-item">
        <div class="stat-label">休眠中</div>
        <div class="stat-value">{{ processInfo.all - processInfo.running }}</div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.metrics-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 12px;
  margin-bottom: 20px;
}

.metric-card {
  background: #fff;
  padding: 16px;
  border-radius: 10px;
  border: 1px solid #e0e0e0;
  display: flex;
  align-items: center;
  gap: 14px;
  transition: all 0.3s;
  position: relative;
  overflow: hidden;
}

.metric-card::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 4px;
}

.metric-card.cpu::before { background: linear-gradient(90deg, #1a73e8, #4285f4); }
.metric-card.memory::before { background: linear-gradient(90deg, #34a853, #5bb974); }
.metric-card.disk::before { background: linear-gradient(90deg, #9334e6, #a855f7); }
.metric-card.network::before { background: linear-gradient(90deg, #ea4335, #f87171); }

.metric-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 16px rgba(0,0,0,0.12);
}

.metric-icon {
  font-size: 32px;
  width: 50px;
  height: 50px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #f5f5f5;
  border-radius: 10px;
  flex-shrink: 0;
}

.metric-content { flex: 1; min-width: 0; }

.metric-label {
  font-size: 11px;
  color: #666;
  text-transform: uppercase;
  letter-spacing: 0.5px;
  font-weight: 600;
  margin-bottom: 4px;
}

.metric-value {
  font-size: 24px;
  font-weight: 700;
  color: #333;
  line-height: 1.2;
  margin-bottom: 2px;
}

.metric-card.cpu .metric-value { color: #1a73e8; }
.metric-card.memory .metric-value { color: #34a853; }
.metric-card.disk .metric-value { color: #9334e6; }
.metric-card.network .metric-value { color: #ea4335; }

.metric-detail {
  font-size: 10px;
  color: #999;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

/* 电池状态 */
.battery-status {
  display: flex;
  align-items: center;
  gap: 16px;
  background: #fff;
  padding: 16px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
}

.battery-icon {
  width: 80px;
  height: 40px;
  border: 3px solid #666;
  border-radius: 6px;
  position: relative;
  background: #f5f5f5;
}

.battery-icon::after {
  content: '';
  position: absolute;
  right: -8px;
  top: 50%;
  transform: translateY(-50%);
  width: 6px;
  height: 16px;
  background: #666;
  border-radius: 0 3px 3px 0;
}

.battery-level {
  height: 100%;
  background: linear-gradient(90deg, #34a853, #5bb974);
  border-radius: 3px;
  transition: width 0.3s;
}

.battery-icon.charging .battery-level {
  background: linear-gradient(90deg, #fbbc04, #fbbf24);
}

.battery-info { flex: 1; }

.battery-percent {
  font-size: 28px;
  font-weight: 700;
  color: #34a853;
  line-height: 1;
  margin-bottom: 4px;
}

.battery-icon.charging + .battery-info .battery-percent { color: #fbbc04; }

.battery-state {
  font-size: 12px;
  color: #666;
  font-weight: 500;
}

/* 进程统计 */
.process-stats {
  display: flex;
  gap: 12px;
}

.stat-item {
  flex: 1;
  background: #fff;
  padding: 16px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  text-align: center;
  transition: all 0.2s;
}

.stat-item:hover {
  border-color: #1a73e8;
  box-shadow: 0 2px 8px rgba(26, 115, 232, 0.15);
}

.stat-label {
  font-size: 11px;
  color: #666;
  text-transform: uppercase;
  letter-spacing: 0.5px;
  font-weight: 600;
  margin-bottom: 8px;
}

.stat-value {
  font-size: 32px;
  font-weight: 700;
  color: #1a73e8;
  line-height: 1;
}

.stat-value.running { color: #34a853; }
</style>
