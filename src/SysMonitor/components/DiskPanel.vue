<script setup>
import { ref, onMounted, watch } from 'vue'
import { setupCanvas, drawNetworkChartWithHighlight, getDataIndexAtPosition, formatIOValue } from '../composables/useChart'

const props = defineProps({
  diskInfo: Object,
  diskIO: Object,
  diskReadHistory: Array,
  diskWriteHistory: Array
})

const canvasRef = ref(null)
const tooltipVisible = ref(false)
const tooltipX = ref(0)
const tooltipY = ref(0)
const tooltipRead = ref('')
const tooltipWrite = ref('')
const highlightIndex = ref(-1)

function redraw() {
  if (canvasRef.value && props.diskReadHistory?.length > 0) {
    requestAnimationFrame(() => {
      drawNetworkChartWithHighlight(canvasRef.value, [
        { data: props.diskReadHistory, color: '#34a853' },
        { data: props.diskWriteHistory, color: '#ea4335' }
      ], highlightIndex.value)
    })
  }
}

function handleMouseMove(e) {
  if (!canvasRef.value || !props.diskReadHistory?.length) return
  const rect = canvasRef.value.getBoundingClientRect()
  const mouseX = e.clientX - rect.left
  const index = getDataIndexAtPosition(canvasRef.value, mouseX, props.diskReadHistory.length, true)
  if (index >= 0 && index < props.diskReadHistory.length) {
    highlightIndex.value = index
    tooltipRead.value = formatIOValue(props.diskReadHistory[index]) + '/s'
    tooltipWrite.value = formatIOValue(props.diskWriteHistory[index]) + '/s'
    tooltipX.value = e.clientX - rect.left
    tooltipY.value = e.clientY - rect.top - 40
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

function openDisk(mount) {
  if (mount && window.utools) {
    const path = mount.endsWith('\\') || mount.endsWith('/') ? mount : mount + '\\'
    window.utools.shellOpenPath(path)
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

watch(() => props.diskReadHistory?.length, redraw)
defineExpose({ redraw, canvasRef })
</script>

<template>
  <div class="panel compact">
    <!-- IO 趋势 -->
    <h3>磁盘 IO</h3>
    <div class="io-section">
      <div class="io-stats">
        <div class="io-stat-item">
          <span class="io-label">读取</span>
          <span class="io-value io-read" v-if="diskIO?.readSecFmt">{{ diskIO.readSecFmt }}</span>
          <span class="skeleton-text" v-else></span>
        </div>
        <div class="io-stat-item">
          <span class="io-label">写入</span>
          <span class="io-value io-write" v-if="diskIO?.writeSecFmt">{{ diskIO.writeSecFmt }}</span>
          <span class="skeleton-text" v-else></span>
        </div>
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
        >
          <div class="tooltip-row"><span class="tooltip-dot dot-read"></span>读 {{ tooltipRead }}</div>
          <div class="tooltip-row"><span class="tooltip-dot dot-write"></span>写 {{ tooltipWrite }}</div>
        </div>
      </div>
    </div>

    <!-- 存储 + 性能指标 -->
    <div class="two-col">
      <div class="col-section">
        <h3>存储概览</h3>
        <div class="storage-grid">
          <div class="storage-item">
            <span class="storage-label">总量</span>
            <span class="storage-value" v-if="diskInfo.totalSize && diskInfo.totalSize !== '0 B'">{{ diskInfo.totalSize }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="storage-item">
            <span class="storage-label">已用</span>
            <span class="storage-value used" v-if="diskInfo.totalUsed && diskInfo.totalUsed !== '0 B'">{{ diskInfo.totalUsed }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="storage-item">
            <span class="storage-label">可用</span>
            <span class="storage-value available" v-if="diskInfo.totalAvailable && diskInfo.totalAvailable !== '0 B'">{{ diskInfo.totalAvailable }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="storage-item">
            <span class="storage-label">使用率</span>
            <span class="storage-value percent" v-if="diskInfo.totalPercent && diskInfo.totalPercent !== '0%'">{{ diskInfo.totalPercent }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
      <div class="col-section">
        <h3>性能指标</h3>
        <div class="perf-grid">
          <div class="perf-item">
            <span class="perf-label">活动时间</span>
            <span class="perf-value" v-if="diskIO?.activeTimeFmt">{{ diskIO.activeTimeFmt }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="perf-item">
            <span class="perf-label">队列长度</span>
            <span class="perf-value" v-if="diskIO?.queueLengthFmt">{{ diskIO.queueLengthFmt }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="perf-item">
            <span class="perf-label">读响应</span>
            <span class="perf-value" v-if="diskIO?.avgReadTimeFmt">{{ diskIO.avgReadTimeFmt }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="perf-item">
            <span class="perf-label">写响应</span>
            <span class="perf-value" v-if="diskIO?.avgWriteTimeFmt">{{ diskIO.avgWriteTimeFmt }}</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="perf-item">
            <span class="perf-label">读次数</span>
            <span class="perf-value" v-if="diskIO?.readsPerSecFmt">{{ diskIO.readsPerSecFmt }}/s</span>
            <span class="skeleton-text" v-else></span>
          </div>
          <div class="perf-item">
            <span class="perf-label">写次数</span>
            <span class="perf-value" v-if="diskIO?.writesPerSecFmt">{{ diskIO.writesPerSecFmt }}/s</span>
            <span class="skeleton-text" v-else></span>
          </div>
        </div>
      </div>
    </div>

    <!-- 物理磁盘 -->
    <div class="physical-section">
      <h3>物理磁盘</h3>
      <div class="physical-list" v-if="diskInfo.physical?.length">
        <div v-for="(disk, index) in diskInfo.physical" :key="index" class="physical-item">
          <span class="physical-icon">💿</span>
          <span class="physical-name">{{ disk.name }}</span>
          <span class="physical-meta">{{ disk.type }}</span>
          <span class="physical-size">{{ disk.size }}</span>
        </div>
      </div>
      <div class="physical-list" v-else>
        <div class="physical-item"><span class="skeleton-text long"></span></div>
      </div>
    </div>

    <!-- 磁盘分区 -->
    <h3>磁盘分区</h3>
    <div class="partitions-grid" v-if="diskInfo.partitions?.length">
      <div 
        v-for="partition in diskInfo.partitions" 
        :key="partition.mount" 
        class="partition-item"
        @click="openDisk(partition.mount)"
        :title="'打开 ' + partition.mount"
      >
        <div class="partition-header">
          <span class="partition-mount">{{ partition.mount }}</span>
          <span class="partition-percent">{{ partition.usedPercent }}</span>
        </div>
        <div class="partition-bar">
          <div class="partition-fill" :style="{ width: partition.usedPercent }"></div>
        </div>
        <div class="partition-footer">
          <span class="partition-type">{{ partition.type }}</span>
          <span class="partition-usage">{{ partition.used }}/{{ partition.size }}</span>
        </div>
      </div>
    </div>
    <div class="partitions-grid" v-else>
      <div class="partition-item skeleton" v-for="i in 3" :key="i">
        <div class="partition-header"><span class="skeleton-text short"></span></div>
        <div class="partition-bar"><div class="partition-fill" style="width:50%"></div></div>
        <div class="partition-footer"><span class="skeleton-text short"></span></div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.panel.compact h3 { margin: 0 0 8px; font-size: 12px; }
.panel.compact h3:not(:first-child) { margin-top: 10px; }

/* IO 区块 - 全宽 */
.io-section {
  background: #fff;
  padding: 12px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 10px;
}
.io-stats { display: flex; gap: 20px; margin-bottom: 8px; }
.io-stat-item { display: flex; align-items: center; gap: 6px; }
.io-label { font-size: 11px; color: #666; font-weight: 500; }
.io-value { font-size: 14px; font-weight: 700; }
.io-value.io-read { color: #34a853; }
.io-value.io-write { color: #ea4335; }
.chart-wrapper { height: 100px; position: relative; }
.chart-canvas { width: 100%; height: 100%; display: block; }

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

/* 存储网格 */
.storage-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 6px;
}
.storage-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 6px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.storage-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; }
.storage-value { font-size: 13px; font-weight: 700; color: #333; margin-top: 2px; }
.storage-value.used { color: #ea4335; }
.storage-value.available { color: #34a853; }
.storage-value.percent { color: #9334e6; }

/* 性能指标网格 */
.perf-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 6px;
}
.perf-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 6px 4px;
  background: #f8f9fa;
  border-radius: 6px;
}
.perf-label { font-size: 9px; color: #888; font-weight: 600; text-transform: uppercase; }
.perf-value { font-size: 11px; font-weight: 700; color: #1a73e8; margin-top: 2px; }

/* 物理磁盘区块 */
.physical-section {
  background: #fff;
  padding: 10px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 10px;
}
.physical-section h3 { margin: 0 0 8px !important; }

/* 物理磁盘 */
.physical-list { display: flex; flex-direction: column; gap: 4px; }
.physical-item {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 8px;
  background: #f8f9fa;
  border-radius: 6px;
  font-size: 11px;
}
.physical-icon { font-size: 14px; }
.physical-name { flex: 1; color: #333; font-weight: 500; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.physical-meta { color: #888; font-size: 10px; }
.physical-size { color: #9334e6; font-weight: 700; }

/* 分区网格 */
.partitions-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(130px, 1fr));
  gap: 8px;
}
.partition-item {
  background: #fff;
  padding: 8px 10px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  cursor: pointer;
  transition: all 0.2s;
}
.partition-item:hover {
  border-color: #9334e6;
  transform: translateY(-1px);
  box-shadow: 0 2px 8px rgba(147, 52, 230, 0.12);
}
.partition-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 5px; }
.partition-mount { font-size: 12px; font-weight: 600; color: #333; }
.partition-percent { font-size: 11px; font-weight: 700; color: #9334e6; }
.partition-bar { height: 4px; background: #e0e0e0; border-radius: 2px; overflow: hidden; margin-bottom: 5px; }
.partition-fill { height: 100%; background: linear-gradient(90deg, #9334e6, #a855f7); border-radius: 2px; }
.partition-footer { display: flex; justify-content: space-between; align-items: center; }
.partition-type { font-size: 9px; color: #999; text-transform: uppercase; font-weight: 600; }
.partition-usage { font-size: 10px; color: #666; }

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
.partition-item.skeleton .partition-fill {
  background: linear-gradient(90deg, #e0e0e0 25%, #d0d0d0 50%, #e0e0e0 75%);
  background-size: 200% 100%;
  animation: shimmer 1.5s infinite;
}

/* Tooltip */
.chart-tooltip {
  position: absolute;
  background: rgba(0, 0, 0, 0.85);
  color: #fff;
  padding: 5px 8px;
  border-radius: 5px;
  font-size: 11px;
  font-weight: 600;
  pointer-events: none;
  transform: translateX(-50%);
  white-space: nowrap;
  z-index: 10;
}
.tooltip-row { display: flex; align-items: center; gap: 5px; line-height: 1.5; }
.tooltip-dot { width: 6px; height: 6px; border-radius: 50%; }
.tooltip-dot.dot-read { background: #34a853; }
.tooltip-dot.dot-write { background: #ea4335; }

@keyframes shimmer { 0% { background-position: -200% 0; } 100% { background-position: 200% 0; } }
</style>
