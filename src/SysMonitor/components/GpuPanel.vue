<script setup>
defineProps({
  gpuInfo: Object
})
</script>

<template>
  <div class="panel compact">
    <!-- 显卡列表 -->
    <h3>显卡</h3>
    <div class="gpu-list" v-if="gpuInfo.controllers?.length">
      <div v-for="(gpu, index) in gpuInfo.controllers" :key="index" class="gpu-item">
        <div class="gpu-icon">🎮</div>
        <div class="gpu-main">
          <div class="gpu-name">{{ gpu.model }}</div>
          <div class="gpu-meta">{{ gpu.vendor }}</div>
        </div>
        <div class="gpu-stats">
          <div class="gpu-stat">
            <span class="stat-label">显存</span>
            <span class="stat-value">{{ gpu.vram }}</span>
          </div>
          <div class="gpu-stat">
            <span class="stat-label">总线</span>
            <span class="stat-value">{{ gpu.bus }}</span>
          </div>
        </div>
      </div>
    </div>
    <div class="gpu-list" v-else>
      <div class="gpu-item skeleton">
        <div class="skeleton-icon"></div>
        <div class="gpu-main">
          <div class="skeleton-text long"></div>
          <div class="skeleton-text"></div>
        </div>
      </div>
    </div>

    <!-- 显示器列表 -->
    <h3>显示器</h3>
    <div class="display-grid" v-if="gpuInfo.displays?.length">
      <div v-for="(display, index) in gpuInfo.displays" :key="index" class="display-item">
        <div class="display-header">
          <span class="display-icon">🖥️</span>
          <span class="display-name">{{ display.model || (display.main ? '主显示器' : '显示器 ' + (index + 1)) }}</span>
          <span class="display-badge" v-if="display.main">主</span>
        </div>
        <div class="display-specs">
          <div class="spec-row">
            <span class="spec-label">分辨率</span>
            <span class="spec-value">{{ display.currentResX }}×{{ display.currentResY }}</span>
          </div>
          <div class="spec-row">
            <span class="spec-label">刷新率</span>
            <span class="spec-value highlight">{{ display.refreshRate }}</span>
          </div>
          <div class="spec-row">
            <span class="spec-label">色深</span>
            <span class="spec-value">{{ display.pixelDepth }}</span>
          </div>
        </div>
      </div>
    </div>
    <div class="display-grid" v-else>
      <div class="display-item skeleton">
        <div class="display-header">
          <span class="skeleton-icon small"></span>
          <span class="skeleton-text"></span>
        </div>
        <div class="display-specs">
          <div class="spec-row" v-for="i in 3" :key="i">
            <span class="skeleton-text short"></span>
            <span class="skeleton-text short"></span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.panel.compact h3 { margin-bottom: 10px; }

.gpu-list { display: flex; flex-direction: column; gap: 10px; margin-bottom: 16px; }
.gpu-item { display: flex; align-items: center; gap: 12px; background: #fff; padding: 14px; border-radius: 10px; border: 1px solid #e0e0e0; }
.gpu-item:hover { border-color: #f59e0b; }
.gpu-icon { font-size: 28px; width: 48px; height: 48px; display: flex; align-items: center; justify-content: center; background: linear-gradient(135deg, #fef3e2, #fde7c7); border-radius: 10px; flex-shrink: 0; }
.gpu-main { flex: 1; min-width: 0; }
.gpu-name { font-size: 14px; font-weight: 600; color: #333; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.gpu-meta { font-size: 11px; color: #888; margin-top: 2px; }
.gpu-stats { display: flex; gap: 16px; flex-shrink: 0; }
.gpu-stat { display: flex; flex-direction: column; align-items: center; }
.stat-label { font-size: 10px; color: #888; font-weight: 500; text-transform: uppercase; }
.stat-value { font-size: 14px; font-weight: 700; color: #f59e0b; margin-top: 2px; }

.display-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 10px; }
.display-item { background: #fff; padding: 14px; border-radius: 10px; border: 1px solid #e0e0e0; }
.display-item:hover { border-color: #0ea5e9; }
.display-header { display: flex; align-items: center; gap: 8px; margin-bottom: 12px; padding-bottom: 10px; border-bottom: 1px solid #f0f0f0; }
.display-icon { font-size: 20px; }
.display-name { flex: 1; font-size: 13px; font-weight: 600; color: #333; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.display-badge { font-size: 10px; padding: 2px 6px; background: #e0f2fe; color: #0ea5e9; border-radius: 4px; font-weight: 600; }
.display-specs { display: flex; flex-direction: column; gap: 8px; }
.spec-row { display: flex; justify-content: space-between; align-items: center; }
.spec-label { font-size: 11px; color: #888; }
.spec-value { font-size: 13px; font-weight: 600; color: #333; }
.spec-value.highlight { color: #0ea5e9; }

.skeleton-text { display: inline-block; height: 14px; width: 70px; background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%); background-size: 200% 100%; animation: shimmer 1.5s infinite; border-radius: 4px; }
.skeleton-text.short { width: 45px; }
.skeleton-text.long { width: 120px; }
.skeleton-icon { width: 48px; height: 48px; border-radius: 10px; background: linear-gradient(90deg, #f0f0f0 25%, #e0e0e0 50%, #f0f0f0 75%); background-size: 200% 100%; animation: shimmer 1.5s infinite; flex-shrink: 0; }
.skeleton-icon.small { width: 20px; height: 20px; border-radius: 4px; }
.gpu-item.skeleton .gpu-main { display: flex; flex-direction: column; gap: 6px; }
.display-item.skeleton .display-header { border-bottom: none; }

@keyframes shimmer { 0% { background-position: -200% 0; } 100% { background-position: 200% 0; } }
</style>
