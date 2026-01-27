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
  uptime: String,
  memoryHardware: Object,
  externalIP: Object
})

// 固定颜色配置 - 轻量化风格，仅用于强调色
const cpuColor = {
  bg: '#ffffff',
  liquid: '#e8f4fd',
  border: '#e5e7eb',
  accent: '#3b82f6',
  glow: 'rgba(59, 130, 246, 0.08)'
}

const memoryColor = {
  bg: '#ffffff',
  liquid: '#f5f3ff',
  border: '#e5e7eb',
  accent: '#8b5cf6',
  glow: 'rgba(139, 92, 246, 0.08)'
}

const diskColor = {
  bg: '#ffffff',
  liquid: '#fef9f3',
  border: '#e5e7eb',
  accent: '#f59e0b',
  glow: 'rgba(245, 158, 11, 0.08)'
}

const networkColor = {
  bg: '#ffffff',
  liquid: '#f0fdf4',
  border: '#e5e7eb',
  accent: '#10b981',
  glow: 'rgba(16, 185, 129, 0.08)'
}
</script>

<template>
  <div class="panel compact">
    <!-- 核心指标卡片 - 4列 液体填充效果 -->
    <div class="metrics-grid">
      <!-- CPU 液体卡片 -->
      <div class="metric-card-liquid">
        <div class="liquid-container" :style="{ 
          borderColor: cpuColor.border 
        }">
          <div 
            class="liquid-wave" 
            :style="{ 
              height: cpuInfo.load || '0%',
              background: cpuColor.liquid
            }"
          >
            <svg viewBox="0 0 500 150" preserveAspectRatio="none" class="wave-svg">
              <path :fill="cpuColor.liquid">
                <animate 
                  attributeName="d" 
                  dur="3s" 
                  repeatCount="indefinite"
                  values="
                    M0,75 C150,90 350,60 500,75 L500,150 L0,150 Z;
                    M0,75 C150,60 350,90 500,75 L500,150 L0,150 Z;
                    M0,75 C150,90 350,60 500,75 L500,150 L0,150 Z
                  "
                />
              </path>
            </svg>
          </div>
          <div class="liquid-container-accent" :style="{ background: cpuColor.accent }"></div>
          <div class="liquid-content">
            <div class="liquid-icon" :style="{ color: cpuColor.accent }">⚡</div>
            <div class="liquid-info">
              <div class="liquid-label">CPU</div>
              <div class="liquid-value" :style="{ color: cpuColor.accent }">
                <span v-if="cpuInfo.load">{{ cpuInfo.load }}</span>
                <span v-else class="skeleton-text"></span>
              </div>
            </div>
          </div>
          <div class="liquid-glow" :style="{ background: cpuColor.glow }"></div>
        </div>
      </div>

      <!-- 内存 液体卡片 -->
      <div class="metric-card-liquid">
        <div class="liquid-container" :style="{ 
          borderColor: memoryColor.border 
        }">
          <div 
            class="liquid-wave" 
            :style="{ 
              height: memoryInfo.usedPercent || '0%',
              background: memoryColor.liquid
            }"
          >
            <svg viewBox="0 0 500 150" preserveAspectRatio="none" class="wave-svg">
              <path :fill="memoryColor.liquid">
                <animate 
                  attributeName="d" 
                  dur="2.5s" 
                  repeatCount="indefinite"
                  values="
                    M0,75 C150,85 350,65 500,75 L500,150 L0,150 Z;
                    M0,75 C150,65 350,85 500,75 L500,150 L0,150 Z;
                    M0,75 C150,85 350,65 500,75 L500,150 L0,150 Z
                  "
                />
              </path>
            </svg>
          </div>
          <div class="liquid-container-accent" :style="{ background: memoryColor.accent }"></div>
          <div class="liquid-content">
            <div class="liquid-icon" :style="{ color: memoryColor.accent }">🎫</div>
            <div class="liquid-info">
              <div class="liquid-label">内存</div>
              <div class="liquid-value" :style="{ color: memoryColor.accent }">
                <span v-if="memoryInfo.usedPercent">{{ memoryInfo.usedPercent }}</span>
                <span v-else class="skeleton-text"></span>
              </div>
            </div>
          </div>
          <div class="liquid-glow" :style="{ background: memoryColor.glow }"></div>
        </div>
      </div>

      <!-- 磁盘 液体卡片 -->
      <div class="metric-card-liquid">
        <div class="liquid-container" :style="{ 
          borderColor: diskColor.border 
        }">
          <div 
            class="liquid-wave" 
            :style="{ 
              height: diskInfo.totalPercent || '0%',
              background: diskColor.liquid
            }"
          >
            <svg viewBox="0 0 500 150" preserveAspectRatio="none" class="wave-svg">
              <path :fill="diskColor.liquid">
                <animate 
                  attributeName="d" 
                  dur="3.5s" 
                  repeatCount="indefinite"
                  values="
                    M0,75 C150,95 350,55 500,75 L500,150 L0,150 Z;
                    M0,75 C150,55 350,95 500,75 L500,150 L0,150 Z;
                    M0,75 C150,95 350,55 500,75 L500,150 L0,150 Z
                  "
                />
              </path>
            </svg>
          </div>
          <div class="liquid-container-accent" :style="{ background: diskColor.accent }"></div>
          <div class="liquid-content">
            <div class="liquid-icon" :style="{ color: diskColor.accent }">💾</div>
            <div class="liquid-info">
              <div class="liquid-label">磁盘</div>
              <div class="liquid-value" :style="{ color: diskColor.accent }">
                <span v-if="diskInfo.totalPercent">{{ diskInfo.totalPercent }}</span>
                <span v-else class="skeleton-text"></span>
              </div>
            </div>
          </div>
          <div class="liquid-glow" :style="{ background: diskColor.glow }"></div>
        </div>
      </div>

      <!-- 网络卡片（双向指示器）-->
      <div class="metric-card-liquid">
        <div class="liquid-container network-card" :style="{ 
          borderColor: networkColor.border 
        }">
          <div class="liquid-container-accent" :style="{ background: networkColor.accent }"></div>
          <div class="liquid-content">
            <div class="liquid-icon" :style="{ color: networkColor.accent }">🌐</div>
            <div class="liquid-info">
              <div class="liquid-label">网络</div>
              <div class="network-speeds">
                <div class="speed-item down">
                  <span class="speed-arrow">↓</span>
                  <span v-if="networkInfo.stats?.[0]" class="speed-value">{{ networkInfo.stats[0].rxSec }}</span>
                  <span v-else class="skeleton-text"></span>
                </div>
                <div class="speed-item up">
                  <span class="speed-arrow">↑</span>
                  <span v-if="networkInfo.stats?.[0]" class="speed-value">{{ networkInfo.stats[0].txSec }}</span>
                  <span v-else class="skeleton-text"></span>
                </div>
              </div>
            </div>
          </div>
          <div class="liquid-glow" :style="{ background: networkColor.glow }"></div>
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
            <span class="value" v-if="memoryInfo.used">{{ memoryInfo.used }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">可用</span>
            <span class="value" v-if="memoryInfo.available">{{ memoryInfo.available }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">类型</span>
            <span class="value" v-if="memoryHardware?.type">{{ memoryHardware.type }} {{ memoryHardware.speed }}MHz</span>
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
            <span class="value" v-if="diskInfo.totalUsed">{{ diskInfo.totalUsed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">可用</span>
            <span class="value" v-if="diskInfo.totalAvailable">{{ diskInfo.totalAvailable }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="compact-item">
            <span class="label">磁盘</span>
            <span class="value" v-if="diskInfo.physical?.length">{{ diskInfo.physical.length }}个 {{ diskInfo.physical[0]?.type || '' }}</span>
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
        <div class="battery-grid">
          <div class="battery-item full">
            <span class="battery-label">
              <span class="status-icon">{{ batteryInfo.isCharging ? '⚡' : '🔋' }}</span>
              {{ batteryInfo.isCharging ? '充电中' : '使用电池' }}
            </span>
            <div class="battery-progress-wrapper">
              <div class="battery-progress-bar" :class="{
                low: parseFloat(batteryInfo.percent) < 20,
                medium: parseFloat(batteryInfo.percent) >= 20 && parseFloat(batteryInfo.percent) < 50,
                charging: batteryInfo.isCharging
              }">
                <div class="battery-progress-fill" :style="{ width: batteryInfo.percent }"></div>
              </div>
              <span class="battery-progress-text">{{ batteryInfo.percent }}</span>
            </div>
          </div>
          <div class="battery-item" v-if="batteryInfo.timeRemaining && batteryInfo.timeRemaining !== 'Unknown'">
            <span class="battery-label">{{ batteryInfo.isCharging ? '充满' : '剩余' }}</span>
            <span class="battery-value">{{ batteryInfo.timeRemaining }}</span>
          </div>
          <div class="battery-item" v-if="batteryInfo.capacity">
            <span class="battery-label">健康度</span>
            <span class="battery-value">{{ batteryInfo.capacity }}</span>
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
            <div class="nic-ips-multi-line">
              <div class="nic-ip-row">
                <span class="ip-tag local-tag">内网</span>
                <span class="ip-address local">{{ networkInfo.interfaces[0].ip4 }}</span>
              </div>
              <div class="nic-ip-row" v-if="externalIP?.ip">
                <span class="ip-tag external-tag">外网</span>
                <span class="ip-address external">{{ externalIP.ip }}</span>
              </div>
            </div>
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

.panel.compact { 
  padding: 12px;
}

.panel.compact h3 { 
  margin: 0 0 8px; 
  font-size: 12px;
  color: #666;
  font-weight: 600;
}

.panel.compact h3:not(:first-child) { 
  margin-top: 10px; 
}

/* 核心指标 - 4列 */
.metrics-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 10px;
  margin-bottom: 10px;
}

/* 液体填充卡片 - 轻量化风格 */
.metric-card-liquid {
  height: 60px;
}

.liquid-container {
  width: 100%;
  height: 100%;
  border-radius: 8px;
  overflow: hidden;
  position: relative;
  border: 1px solid;
  transition: all 0.3s ease;
  box-shadow: 0 1px 3px rgba(0, 0, 0, 0.06);
  display: flex;
  align-items: center;
  padding: 0 12px;
  gap: 10px;
  background: #fff;
}

.liquid-container::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 2px;
  z-index: 2;
}

.liquid-container-accent {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 2px;
  z-index: 2;
}

.liquid-container:hover {
  transform: translateY(-1px);
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
}

.liquid-wave {
  position: absolute;
  bottom: 0;
  left: 0;
  width: 100%;
  height: 100%;
  transition: height 0.8s cubic-bezier(0.4, 0, 0.2, 1);
  z-index: 0;
  opacity: 1;
}

.wave-svg {
  position: absolute;
  top: -50px;
  left: 0;
  width: 100%;
  height: 150px;
  opacity: 0.4;
}

.liquid-content {
  position: relative;
  display: flex;
  align-items: center;
  gap: 10px;
  z-index: 3;
  flex: 1;
}

.liquid-icon {
  font-size: 22px;
  flex-shrink: 0;
  filter: none;
}

.liquid-info {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 2px;
}

.liquid-label {
  font-size: 10px;
  font-weight: 600;
  color: #888;
  text-transform: uppercase;
  letter-spacing: 0.5px;
  line-height: 1;
}

.liquid-value {
  font-size: 20px;
  font-weight: 700;
  line-height: 1;
}

.liquid-glow {
  display: none;
}

/* 网络卡片特殊样式 */
.liquid-container.network-card {
  background: linear-gradient(to bottom, #f0fdf4 0%, #ffffff 50%, #eff6ff 100%);
}

.network-speeds {
  display: flex;
  flex-direction: column;
  gap: 3px;
}

.speed-item {
  display: flex;
  align-items: center;
  gap: 4px;
  font-size: 11px;
  line-height: 1;
}

.speed-arrow {
  font-size: 13px;
  font-weight: 700;
}

.speed-item.down .speed-arrow {
  color: #10b981;
}

.speed-item.up .speed-arrow {
  color: #3b82f6;
}

.speed-value {
  font-size: 11px;
  font-weight: 600;
  color: #333;
}

/* 两列布局 */
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
  border: 1px solid #e5e7eb;
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

.compact-item .value.warning { 
  color: #ef4444;
  font-weight: 700;
}

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
  color: #3b82f6;
  line-height: 1;
}

.process-value.running { color: #10b981; }

.process-label {
  font-size: 9px;
  color: #888;
  text-transform: uppercase;
  font-weight: 600;
}

/* 电池网格 */
.battery-grid {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.battery-item {
  display: flex;
  flex-direction: column;
  gap: 4px;
  padding: 6px 8px;
  background: #f8f9fa;
  border-radius: 6px;
}

.battery-item.full {
  /* 第一行：状态+进度条 */
}

.battery-label {
  font-size: 9px;
  color: #888;
  text-transform: uppercase;
  font-weight: 600;
  display: flex;
  align-items: center;
  gap: 4px;
}

.battery-label .status-icon {
  font-size: 11px;
}

.battery-value {
  font-size: 12px;
  font-weight: 600;
  color: #333;
  display: flex;
  align-items: center;
  gap: 4px;
}

.battery-progress-wrapper {
  display: flex;
  align-items: center;
  gap: 8px;
}

.battery-progress-bar {
  flex: 1;
  height: 8px;
  background: #e5e7eb;
  border-radius: 4px;
  overflow: hidden;
  position: relative;
}

.battery-progress-fill {
  height: 100%;
  background: #10b981;
  border-radius: 4px;
  transition: width 0.5s ease;
  position: relative;
  overflow: hidden;
}

/* 低电量：脉冲警告 */
.battery-progress-bar.low .battery-progress-fill {
  background: #ef4444;
  animation: low-battery-pulse 1.5s ease-in-out infinite;
}

@keyframes low-battery-pulse {
  0%, 100% { 
    opacity: 1;
    box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.4);
  }
  50% { 
    opacity: 0.7;
    box-shadow: 0 0 8px 2px rgba(239, 68, 68, 0.6);
  }
}

/* 中等电量：静态橙色 */
.battery-progress-bar.medium .battery-progress-fill {
  background: #f59e0b;
}

/* 充电中：流动光效 */
.battery-progress-bar.charging .battery-progress-fill {
  background: linear-gradient(90deg, #8b5cf6, #a78bfa, #8b5cf6);
  background-size: 200% 100%;
  animation: charging-flow 3s linear infinite;
}

.battery-progress-bar.charging .battery-progress-fill::before {
  content: '';
  position: absolute;
  top: 0;
  left: -100%;
  width: 100%;
  height: 100%;
  background: linear-gradient(
    90deg,
    transparent 0%,
    rgba(167, 139, 250, 0.3) 40%,
    rgba(196, 181, 253, 0.6) 50%,
    rgba(167, 139, 250, 0.3) 60%,
    transparent 100%
  );
  animation: charging-shine 2.5s ease-in-out infinite;
}

@keyframes charging-flow {
  0% { background-position: 0% 50%; }
  100% { background-position: 200% 50%; }
}

@keyframes charging-shine {
  0% { left: -100%; }
  100% { left: 200%; }
}

/* 正常电量：微妙呼吸 */
.battery-progress-bar:not(.low):not(.medium):not(.charging) .battery-progress-fill {
  animation: normal-breathe 4s ease-in-out infinite;
}

@keyframes normal-breathe {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.85; }
}

.battery-progress-text {
  font-size: 11px;
  font-weight: 700;
  color: #333;
  min-width: 35px;
  text-align: right;
}

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

.net-label { 
  font-size: 9px; 
  color: #888; 
  text-transform: uppercase; 
  font-weight: 600; 
}

.net-value { 
  font-size: 14px; 
  font-weight: 700; 
}

.net-value.down { color: #10b981; }
.net-value.up { color: #3b82f6; }

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

.gpu-meta { 
  font-size: 10px; 
  color: #888; 
  margin-top: 2px; 
}

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
  background: #dbeafe;
  border-radius: 6px;
  flex-shrink: 0;
}

.nic-info { 
  flex: 1; 
  min-width: 0;
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.nic-name {
  font-size: 12px;
  font-weight: 600;
  color: #333;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.nic-info { 
  flex: 1; 
  min-width: 0;
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.nic-name {
  font-size: 12px;
  font-weight: 600;
  color: #333;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.nic-ips-multi-line {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.nic-ip-row {
  display: flex;
  align-items: center;
  gap: 6px;
}

.ip-tag {
  font-size: 8px;
  font-weight: 700;
  text-transform: uppercase;
  padding: 2px 5px;
  border-radius: 3px;
  letter-spacing: 0.3px;
  flex-shrink: 0;
}

.ip-address { 
  font-size: 10px;
  font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
  font-weight: 600;
  letter-spacing: 0.3px;
}

.ip-tag {
  font-size: 8px;
  font-weight: 700;
  text-transform: uppercase;
  padding: 2px 5px;
  border-radius: 3px;
  letter-spacing: 0.3px;
  flex-shrink: 0;
}

.ip-tag.local-tag {
  background: #dbeafe;
  color: #1e40af;
}

.ip-tag.external-tag {
  background: #d1fae5;
  color: #065f46;
}

.ip-address { 
  font-size: 10px;
  font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
  font-weight: 600;
  letter-spacing: 0.3px;
}

.ip-address.local {
  color: #3b82f6;
}

.ip-address.external {
  color: #10b981;
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

.skeleton-text.long { width: 100px; }

.metric-value .skeleton-text { height: 18px; width: 40px; }
.liquid-value .skeleton-text { height: 28px; width: 50px; }

@keyframes shimmer {
  0% { background-position: -200% 0; }
  100% { background-position: 200% 0; }
}
</style>
