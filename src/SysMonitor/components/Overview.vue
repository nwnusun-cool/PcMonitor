<script setup>
const props = defineProps({
  cpuInfo: Object,
  memoryInfo: Object,
  diskInfo: Object,
  networkInfo: Object,
  systemInfo: Object,
  batteryInfo: Object,
  processInfo: Object,
  systemStats: Object,
  gpuInfo: Object,
  uptime: String
})
</script>

<template>
  <div class="panel compact">
    <!-- 核心指标卡片 - 4列 -->
    <div class="metrics-grid">
      <div class="metric-card cpu">
        <div class="metric-icon">⚡</div>
        <div class="metric-content">
          <div class="metric-label">CPU</div>
          <div class="metric-value">
            <span v-if="cpuInfo.load">{{ cpuInfo.load }}</span>
            <span v-else class="skeleton-text"></span>
          </div>
        </div>
      </div>
      <div class="metric-card memory">
        <div class="metric-icon">💾</div>
        <div class="metric-content">
          <div class="metric-label">内存</div>
          <div class="metric-value">
            <span v-if="memoryInfo.usedPercent">{{ memoryInfo.usedPercent }}</span>
            <span v-else class="skeleton-text"></span>
          </div>
        </div>
      </div>
      <div class="metric-card disk">
        <div class="metric-icon">💿</div>
        <div class="metric-content">
          <div class="metric-label">磁盘</div>
          <div class="metric-value">
            <span v-if="diskInfo.totalPercent">{{ diskInfo.totalPercent }}</span>
            <span v-else class="skeleton-text"></span>
          </div>
        </div>
      </div>
      <div class="metric-card network">
        <div class="metric-icon">🌐</div>
        <div class="metric-content">
          <div class="metric-label">网络</div>
          <div class="metric-value small">
            <span v-if="networkInfo.stats?.[0]">↓{{ networkInfo.stats[0].rxSec }}</span>
            <span v-else class="skeleton-text"></span>
          </div>
        </div>
      </div>
    </div>

    <!-- 系统 + 处理器 -->
    <div class="two-col">
      <div class="col-section">
        <h3>系统</h3>
        <div class="compact-grid">
          <div class="compact-item">
            <span class="label">系统</span>
            <span class="value" v-if="systemInfo.platform">{{ systemInfo.platform }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">主机</span>
            <span class="value" v-if="systemInfo.hostname">{{ systemInfo.hostname }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">运行</span>
            <span class="value" v-if="uptime">{{ uptime }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">型号</span>
            <span class="value" v-if="systemInfo.model">{{ systemInfo.model }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
      <div class="col-section">
        <h3>处理器</h3>
        <div class="compact-grid">
          <div class="compact-item full">
            <span class="label">型号</span>
            <span class="value" v-if="cpuInfo.brand">{{ cpuInfo.brand }}</span>
            <span class="skeleton-text long" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">核心</span>
            <span class="value" v-if="cpuInfo.physicalCores">{{ cpuInfo.physicalCores }}C/{{ cpuInfo.cores }}T</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">频率</span>
            <span class="value" v-if="cpuInfo.currentSpeed">{{ cpuInfo.currentSpeed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
    </div>

    <!-- 内存 + 存储 -->
    <div class="two-col">
      <div class="col-section">
        <h3>内存</h3>
        <div class="compact-grid four">
          <div class="compact-item">
            <span class="label">总量</span>
            <span class="value" v-if="memoryInfo.total">{{ memoryInfo.total }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">已用</span>
            <span class="value used" v-if="memoryInfo.used">{{ memoryInfo.used }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">可用</span>
            <span class="value available" v-if="memoryInfo.available">{{ memoryInfo.available }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">使用率</span>
            <span class="value percent" v-if="memoryInfo.usedPercent">{{ memoryInfo.usedPercent }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
      <div class="col-section">
        <h3>存储</h3>
        <div class="compact-grid four">
          <div class="compact-item">
            <span class="label">总量</span>
            <span class="value" v-if="diskInfo.totalSize">{{ diskInfo.totalSize }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">已用</span>
            <span class="value used" v-if="diskInfo.totalUsed">{{ diskInfo.totalUsed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">可用</span>
            <span class="value available" v-if="diskInfo.totalAvailable">{{ diskInfo.totalAvailable }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">使用率</span>
            <span class="value percent" v-if="diskInfo.totalPercent">{{ diskInfo.totalPercent }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
    </div>

    <!-- 进程 + 电池/网络 -->
    <div class="two-col">
      <div class="col-section">
        <h3>进程</h3>
        <div class="process-row">
          <div class="process-item">
            <span class="process-value" v-if="systemStats?.processCount">{{ systemStats.processCount }}</span>
            <span class="skeleton-text" v-else></span>
            <span class="process-label">进程</span>
          </div>
          <div class="process-item">
            <span class="process-value running" v-if="systemStats?.threadCount">{{ systemStats.threadCount.toLocaleString() }}</span>
            <span class="skeleton-text" v-else></span>
            <span class="process-label">线程</span>
          </div>
          <div class="process-item">
            <span class="process-value" v-if="systemStats?.handleCount">{{ systemStats.handleCount.toLocaleString() }}</span>
            <span class="skeleton-text" v-else></span>
            <span class="process-label">句柄</span>
          </div>
        </div>
      </div>
      <div class="col-section" v-if="batteryInfo.hasBattery">
        <h3>电池</h3>
        <div class="battery-row">
          <div class="battery-icon" :class="{ charging: batteryInfo.isCharging }">
            <div class="battery-level" :style="{ width: batteryInfo.percent }"></div>
          </div>
          <div class="battery-text">
            <span class="battery-percent">{{ batteryInfo.percent }}</span>
            <span class="battery-state">{{ batteryInfo.isCharging ? '充电中' : '电池' }}</span>
          </div>
        </div>
      </div>
      <div class="col-section" v-else>
        <h3>网络</h3>
        <div class="network-row">
          <div class="net-item">
            <span class="net-label">下载</span>
            <span class="net-value down" v-if="networkInfo.stats?.[0]">{{ networkInfo.stats[0].rxSec }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="net-item">
            <span class="net-label">上传</span>
            <span class="net-value up" v-if="networkInfo.stats?.[0]">{{ networkInfo.stats[0].txSec }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
    </div>

    <!-- GPU + 主网卡 -->
    <div class="two-col">
      <div class="col-section">
        <h3>显卡</h3>
        <div class="gpu-row" v-if="gpuInfo?.controllers?.[0]">
          <span class="gpu-icon">🎮</span>
          <div class="gpu-info">
            <div class="gpu-name">{{ gpuInfo.controllers[0].model }}</div>
            <div class="gpu-meta">{{ gpuInfo.controllers[0].vram }} · {{ gpuInfo.controllers[0].vendor }}</div>
          </div>
        </div>
        <div class="gpu-row" v-else>
          <span class="gpu-icon">🎮</span>
          <div class="gpu-info">
            <div class="skeleton-text long"></div>
            <div class="skeleton-text"></div>
          </div>
        </div>
      </div>
      <div class="col-section">
        <h3>主网卡</h3>
        <div class="nic-row" v-if="networkInfo?.interfaces?.[0]">
          <span class="nic-icon">{{ networkInfo.interfaces[0].type === 'wireless' ? '📶' : '🔌' }}</span>
          <div class="nic-info">
            <div class="nic-name">{{ networkInfo.interfaces[0].ifaceName }}</div>
            <div class="nic-ip">{{ networkInfo.interfaces[0].ip4 }}</div>
          </div>
        </div>
        <div class="nic-row" v-else>
          <span class="nic-icon">🔌</span>
          <div class="nic-info">
            <div class="skeleton-text"></div>
            <div class="skeleton-text long"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.panel.compact { padding: 12px; }
.panel.compact h3 { margin: 0 0 8px; font-size: 12px; }
.panel.compact h3:not(:first-child) { margin-top: 12px; }

/* 核心指标 - 4列 */
.metrics-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 8px;
  margin-bottom: 12px;
}

.metric-card {
  background: #fff;
  padding: 10px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  display: flex;
  align-items: center;
  gap: 8px;
  position: relative;
  overflow: hidden;
}
.metric-card::before {
  content: '';
  position: absolute;
  top: 0; left: 0; right: 0;
  height: 3px;
}
.metric-card.cpu::before { background: #1a73e8; }
.metric-card.memory::before { background: #34a853; }
.metric-card.disk::before { background: #9334e6; }
.metric-card.network::before { background: #ea4335; }

.metric-icon {
  font-size: 22px;
  width: 36px;
  height: 36px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #f5f5f5;
  border-radius: 8px;
  flex-shrink: 0;
}
.metric-content { flex: 1; min-width: 0; }
.metric-label { font-size: 10px; color: #888; font-weight: 600; text-transform: uppercase; }
.metric-value { font-size: 18px; font-weight: 700; line-height: 1.2; }
.metric-value.small { font-size: 14px; }
.metric-card.cpu .metric-value { color: #1a73e8; }
.metric-card.memory .metric-value { color: #34a853; }
.metric-card.disk .metric-value { color: #9334e6; }
.metric-card.network .metric-value { color: #ea4335; }

/* 两列布局 */
.two-col {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 12px;
  margin-bottom: 12px;
}
.col-section {
  background: #fff;
  padding: 10px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
}
.col-section h3 { margin: 0 0 8px !important; }

/* 紧凑网格 */
.compact-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 6px;
}
.compact-grid.four { grid-template-columns: repeat(4, 1fr); }
.compact-item {
  display: flex;
  flex-direction: column;
  gap: 2px;
}
.compact-item.full { grid-column: span 2; }
.compact-item .label {
  font-size: 9px;
  color: #888;
  text-transform: uppercase;
  font-weight: 600;
}
.compact-item .value {
  font-size: 12px;
  color: #333;
  font-weight: 600;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
.compact-item .value.used { color: #ea4335; }
.compact-item .value.available { color: #34a853; }
.compact-item .value.percent { color: #1a73e8; }

/* 进程行 */
.process-row {
  display: flex;
  gap: 8px;
}
.process-item {
  flex: 1;
  text-align: center;
  padding: 6px;
  background: #f8f9fa;
  border-radius: 6px;
}
.process-value {
  display: block;
  font-size: 20px;
  font-weight: 700;
  color: #1a73e8;
  line-height: 1;
}
.process-value.running { color: #34a853; }
.process-label {
  font-size: 9px;
  color: #888;
  text-transform: uppercase;
  font-weight: 600;
}

/* 电池 */
.battery-row {
  display: flex;
  align-items: center;
  gap: 10px;
}
.battery-icon {
  width: 50px;
  height: 24px;
  border: 2px solid #666;
  border-radius: 4px;
  position: relative;
  background: #f5f5f5;
}
.battery-icon::after {
  content: '';
  position: absolute;
  right: -5px;
  top: 50%;
  transform: translateY(-50%);
  width: 4px;
  height: 10px;
  background: #666;
  border-radius: 0 2px 2px 0;
}
.battery-level {
  height: 100%;
  background: #34a853;
  border-radius: 2px;
}
.battery-icon.charging .battery-level { background: #fbbc04; }
.battery-text { display: flex; flex-direction: column; }
.battery-percent { font-size: 16px; font-weight: 700; color: #34a853; line-height: 1; }
.battery-icon.charging + .battery-text .battery-percent { color: #fbbc04; }
.battery-state { font-size: 10px; color: #888; }

/* 网络行 */
.network-row {
  display: flex;
  gap: 12px;
}
.net-item {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 2px;
}
.net-label { font-size: 9px; color: #888; text-transform: uppercase; font-weight: 600; }
.net-value { font-size: 14px; font-weight: 700; }
.net-value.down { color: #34a853; }
.net-value.up { color: #1a73e8; }

/* GPU 行 */
.gpu-row {
  display: flex;
  align-items: center;
  gap: 10px;
}
.gpu-icon {
  font-size: 20px;
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #fef3e2;
  border-radius: 6px;
  flex-shrink: 0;
}
.gpu-info { flex: 1; min-width: 0; }
.gpu-name {
  font-size: 12px;
  font-weight: 600;
  color: #333;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
.gpu-meta { font-size: 10px; color: #888; margin-top: 2px; }

/* 网卡行 */
.nic-row {
  display: flex;
  align-items: center;
  gap: 10px;
}
.nic-icon {
  font-size: 20px;
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: #e8f0fe;
  border-radius: 6px;
  flex-shrink: 0;
}
.nic-info { flex: 1; min-width: 0; }
.nic-name {
  font-size: 12px;
  font-weight: 600;
  color: #333;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}
.nic-ip { font-size: 11px; color: #1a73e8; font-family: monospace; margin-top: 2px; }

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
.skeleton-text.long { width: 100px; }
.metric-value .skeleton-text { height: 18px; width: 40px; }

@keyframes shimmer {
  0% { background-position: -200% 0; }
  100% { background-position: 200% 0; }
}
</style>
