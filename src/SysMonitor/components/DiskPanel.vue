<script setup>
defineProps({
  diskInfo: Object
})
</script>

<template>
  <div class="panel">
    <!-- 存储概览 -->
    <h3>存储概览</h3>
    <div class="storage-overview">
      <div class="storage-stat">
        <span class="storage-label">总容量</span>
        <span class="storage-value">{{ diskInfo.totalSize }}</span>
      </div>
      <div class="storage-stat">
        <span class="storage-label">已使用</span>
        <span class="storage-value used">{{ diskInfo.totalUsed }}</span>
      </div>
      <div class="storage-stat">
        <span class="storage-label">可用</span>
        <span class="storage-value available">{{ diskInfo.totalAvailable }}</span>
      </div>
      <div class="storage-stat">
        <span class="storage-label">使用率</span>
        <span class="storage-value percent">{{ diskInfo.totalPercent }}</span>
      </div>
    </div>

    <!-- 物理磁盘 -->
    <h3 v-if="diskInfo.physical && diskInfo.physical.length > 0">物理磁盘</h3>
    <div class="physical-disks" v-if="diskInfo.physical && diskInfo.physical.length > 0">
      <div v-for="(disk, index) in diskInfo.physical" :key="index" class="physical-disk-item">
        <div class="physical-disk-icon">💿</div>
        <div class="physical-disk-info">
          <div class="physical-disk-name">{{ disk.name }}</div>
          <div class="physical-disk-meta">{{ disk.vendor }} · {{ disk.type }} · {{ disk.interfaceType }}</div>
        </div>
        <div class="physical-disk-size">{{ disk.size }}</div>
      </div>
    </div>

    <!-- 磁盘分区 -->
    <h3>磁盘分区</h3>
    <div class="partitions-grid">
      <div v-for="partition in diskInfo.partitions" :key="partition.mount" class="partition-item">
        <div class="partition-top">
          <span class="partition-mount">{{ partition.mount }}</span>
          <span class="partition-percent">{{ partition.usedPercent }}</span>
        </div>
        <div class="partition-bar">
          <div class="partition-bar-fill" :style="{ width: partition.usedPercent }"></div>
        </div>
        <div class="partition-bottom">
          <span class="partition-type">{{ partition.type }}</span>
          <span class="partition-usage">{{ partition.used }} / {{ partition.size }}</span>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.storage-overview {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 12px;
  margin-bottom: 16px;
}

.storage-stat {
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

.storage-label {
  font-size: 11px;
  color: #888;
  margin-bottom: 10px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.storage-value {
  font-size: 20px;
  font-weight: 700;
  color: #333;
  line-height: 1;
}

.storage-value.used { color: #ea4335; }
.storage-value.available { color: #34a853; }
.storage-value.percent { color: #9334e6; }

.physical-disks {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-bottom: 16px;
}

.physical-disk-item {
  display: flex;
  align-items: center;
  gap: 12px;
  background: #fff;
  padding: 12px 14px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
}

.physical-disk-item:hover { border-color: #9334e6; }

.physical-disk-icon { font-size: 24px; }

.physical-disk-info {
  flex: 1;
  min-width: 0;
}

.physical-disk-name {
  font-size: 13px;
  font-weight: 600;
  color: #333;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.physical-disk-meta {
  font-size: 11px;
  color: #999;
  margin-top: 2px;
}

.physical-disk-size {
  font-size: 14px;
  font-weight: 700;
  color: #9334e6;
}

.partitions-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
  gap: 10px;
}

.partition-item {
  background: #fff;
  padding: 12px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
}

.partition-item:hover { border-color: #9334e6; }

.partition-top {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 8px;
}

.partition-mount {
  font-size: 13px;
  font-weight: 600;
  color: #333;
}

.partition-percent {
  font-size: 14px;
  font-weight: 700;
  color: #9334e6;
}

.partition-bar {
  height: 6px;
  background: #e0e0e0;
  border-radius: 3px;
  overflow: hidden;
  margin-bottom: 8px;
}

.partition-bar-fill {
  height: 100%;
  background: linear-gradient(90deg, #9334e6, #a855f7);
  border-radius: 3px;
  transition: width 0.3s;
}

.partition-bottom {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.partition-type {
  font-size: 10px;
  color: #999;
  text-transform: uppercase;
  font-weight: 600;
}

.partition-usage {
  font-size: 11px;
  color: #666;
}
</style>
