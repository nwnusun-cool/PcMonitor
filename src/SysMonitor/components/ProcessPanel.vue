<script setup>
defineProps({
  processInfo: Object
})
</script>

<template>
  <div class="panel">
    <!-- 进程统计概览 -->
    <h3>进程统计</h3>
    <div class="process-overview">
      <div class="process-stat-card">
        <span class="process-stat-label">总进程</span>
        <span class="process-stat-value" v-if="processInfo.all">{{ processInfo.all }}</span>
        <span class="skeleton-text center" v-else></span>
      </div>
      <div class="process-stat-card">
        <span class="process-stat-label">CPU Top 1</span>
        <span class="process-stat-value cpu-top" v-if="processInfo.topCpu?.[0]">{{ processInfo.topCpu[0].cpu }}</span>
        <span class="skeleton-text center" v-else></span>
      </div>
      <div class="process-stat-card">
        <span class="process-stat-label">内存 Top 1</span>
        <span class="process-stat-value mem-top" v-if="processInfo.topMem?.[0]">{{ processInfo.topMem[0].mem }}</span>
        <span class="skeleton-text center" v-else></span>
      </div>
      <div class="process-stat-card">
        <span class="process-stat-label">监控中</span>
        <span class="process-stat-value running">20</span>
      </div>
    </div>

    <!-- CPU 占用排行 -->
    <h3>CPU 占用 Top 10</h3>
    <div class="process-table-wrapper">
      <table class="process-table">
        <thead>
          <tr>
            <th class="col-name">进程名</th>
            <th class="col-pid">PID</th>
            <th class="col-cpu">CPU</th>
            <th class="col-mem">内存</th>
          </tr>
        </thead>
        <tbody v-if="processInfo.topCpu && processInfo.topCpu.length > 0">
          <tr v-for="proc in processInfo.topCpu" :key="'cpu-'+proc.pid">
            <td class="col-name">{{ proc.name }}</td>
            <td class="col-pid">{{ proc.pid }}</td>
            <td class="col-cpu">
              <div class="usage-bar-wrapper">
                <div class="usage-bar cpu" :style="{ width: Math.min(proc.cpuRaw * 10, 100) + '%' }"></div>
                <span class="usage-text">{{ proc.cpu }}</span>
              </div>
            </td>
            <td class="col-mem">{{ proc.mem }}</td>
          </tr>
        </tbody>
        <tbody v-else>
          <tr v-for="i in 5" :key="'skeleton-cpu-'+i" class="skeleton-row">
            <td class="col-name"><span class="skeleton-text"></span></td>
            <td class="col-pid"><span class="skeleton-text short"></span></td>
            <td class="col-cpu"><span class="skeleton-text"></span></td>
            <td class="col-mem"><span class="skeleton-text short"></span></td>
          </tr>
        </tbody>
      </table>
    </div>

    <!-- 内存占用排行 -->
    <h3>内存占用 Top 10</h3>
    <div class="process-table-wrapper">
      <table class="process-table">
        <thead>
          <tr>
            <th class="col-name">进程名</th>
            <th class="col-pid">PID</th>
            <th class="col-cpu">CPU</th>
            <th class="col-mem">内存</th>
          </tr>
        </thead>
        <tbody v-if="processInfo.topMem && processInfo.topMem.length > 0">
          <tr v-for="proc in processInfo.topMem" :key="'mem-'+proc.pid">
            <td class="col-name">{{ proc.name }}</td>
            <td class="col-pid">{{ proc.pid }}</td>
            <td class="col-cpu">{{ proc.cpu }}</td>
            <td class="col-mem">
              <div class="usage-bar-wrapper">
                <div class="usage-bar mem" :style="{ width: Math.min(proc.memRaw * 5, 100) + '%' }"></div>
                <span class="usage-text">{{ proc.mem }}</span>
              </div>
            </td>
          </tr>
        </tbody>
        <tbody v-else>
          <tr v-for="i in 5" :key="'skeleton-mem-'+i" class="skeleton-row">
            <td class="col-name"><span class="skeleton-text"></span></td>
            <td class="col-pid"><span class="skeleton-text short"></span></td>
            <td class="col-cpu"><span class="skeleton-text short"></span></td>
            <td class="col-mem"><span class="skeleton-text"></span></td>
          </tr>
        </tbody>
      </table>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.process-overview {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 12px;
  margin-bottom: 16px;
}

.process-stat-card {
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

.process-stat-label {
  font-size: 11px;
  color: #888;
  margin-bottom: 10px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.process-stat-value {
  font-size: 28px;
  font-weight: 700;
  color: #1a73e8;
  line-height: 1;
}

.process-stat-value.running { color: #34a853; }
.process-stat-value.cpu-top { color: #1a73e8; }
.process-stat-value.mem-top { color: #34a853; }

.process-table-wrapper {
  background: #fff;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  overflow: hidden;
  margin-bottom: 16px;
}

.process-table {
  width: 100%;
  border-collapse: collapse;
  font-size: 12px;
}

.process-table th,
.process-table td {
  padding: 10px 12px;
  text-align: left;
}

.process-table th {
  background: #f8f9fa;
  color: #666;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.3px;
  font-size: 10px;
  border-bottom: 1px solid #e0e0e0;
}

.process-table td {
  border-bottom: 1px solid #f0f0f0;
}

.process-table tr:last-child td {
  border-bottom: none;
}

.process-table tr:hover td {
  background: #f8f9fa;
}

.process-table .col-name {
  width: 40%;
  max-width: 180px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  font-weight: 500;
}

.process-table .col-pid {
  width: 15%;
  color: #666;
  font-family: monospace;
}

.process-table .col-cpu,
.process-table .col-mem {
  width: 22%;
}

.usage-bar-wrapper {
  display: flex;
  align-items: center;
  gap: 8px;
}

.usage-bar {
  height: 6px;
  border-radius: 3px;
  min-width: 4px;
  max-width: 60px;
  transition: width 0.3s;
}

.usage-bar.cpu {
  background: linear-gradient(90deg, #1a73e8, #4285f4);
}

.usage-bar.mem {
  background: linear-gradient(90deg, #34a853, #5bb974);
}

.usage-text {
  font-weight: 600;
  min-width: 45px;
}

/* 骨架屏 */
.skeleton-text {
  display: inline-block;
  height: 12px;
  width: 70px;
  background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%);
  background-size: 200% 100%;
  animation: shimmer 1.5s infinite;
  border-radius: 4px;
}
.skeleton-text.short { width: 40px; }
.skeleton-text.center {
  display: block;
  margin: 0 auto;
  height: 24px;
  width: 50px;
}

.skeleton-row td {
  padding: 12px !important;
}

@keyframes shimmer {
  0% { background-position: -200% 0; }
  100% { background-position: 200% 0; }
}
</style>
