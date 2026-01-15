<script setup>
defineProps({
  gpuInfo: Object
})
</script>

<template>
  <div class="panel">
    <!-- 显卡信息 -->
    <h3>显卡信息</h3>
    <div v-for="(gpu, index) in gpuInfo.controllers" :key="'gpu-'+index" class="gpu-card">
      <div class="gpu-card-header">
        <div class="gpu-icon">🎮</div>
        <div class="gpu-info">
          <div class="gpu-model">{{ gpu.model }}</div>
          <div class="gpu-vendor">{{ gpu.vendor }} · {{ gpu.bus }}</div>
        </div>
      </div>
      <div class="gpu-specs">
        <div class="gpu-spec-item">
          <span class="gpu-spec-label">显存</span>
          <span class="gpu-spec-value">{{ gpu.vram }}</span>
        </div>
        <div class="gpu-spec-item">
          <span class="gpu-spec-label">总线</span>
          <span class="gpu-spec-value">{{ gpu.bus }}</span>
        </div>
      </div>
    </div>

    <!-- 显示器信息 -->
    <h3 v-if="gpuInfo.displays && gpuInfo.displays.length > 0">显示器</h3>
    <div v-for="(display, index) in gpuInfo.displays" :key="'display-'+index" class="display-card">
      <div class="display-card-header">
        <div class="display-icon">🖥️</div>
        <div class="display-info">
          <div class="display-name">{{ display.main ? '主显示器' : '副显示器' }}</div>
          <div class="display-meta">{{ display.builtin ? '内置显示器' : '外接显示器' }} · {{ display.pixelDepth }}色深</div>
        </div>
      </div>
      <div class="display-specs">
        <div class="display-spec-item">
          <span class="display-spec-label">原生分辨率</span>
          <span class="display-spec-value">{{ display.resolutionX }}×{{ display.resolutionY }}</span>
        </div>
        <div class="display-spec-item">
          <span class="display-spec-label">当前分辨率</span>
          <span class="display-spec-value">{{ display.currentResX }}×{{ display.currentResY }}</span>
        </div>
        <div class="display-spec-item">
          <span class="display-spec-label">刷新率</span>
          <span class="display-spec-value highlight">{{ display.refreshRate }}</span>
        </div>
        <div class="display-spec-item" v-if="display.resolutionX && display.currentResX">
          <span class="display-spec-label">缩放比例</span>
          <span class="display-spec-value">{{ Math.round(display.resolutionX / display.currentResX * 100) }}%</span>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
@import '../styles/common.css';

.gpu-card {
  background: #fff;
  padding: 16px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 12px;
}

.gpu-card-header {
  display: flex;
  align-items: center;
  gap: 14px;
  margin-bottom: 16px;
}

.gpu-icon {
  font-size: 32px;
  width: 56px;
  height: 56px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #fef3e2, #fde7c7);
  border-radius: 12px;
}

.gpu-info { flex: 1; }

.gpu-model {
  font-size: 15px;
  font-weight: 700;
  color: #333;
  margin-bottom: 4px;
}

.gpu-vendor {
  font-size: 12px;
  color: #666;
}

.gpu-specs {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 10px;
}

.gpu-spec-item {
  background: #f8f9fa;
  padding: 12px;
  border-radius: 6px;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.gpu-spec-label {
  font-size: 10px;
  color: #888;
  margin-bottom: 6px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.gpu-spec-value {
  font-size: 16px;
  font-weight: 700;
  color: #f59e0b;
}

.display-card {
  background: #fff;
  padding: 16px;
  border-radius: 8px;
  border: 1px solid #e0e0e0;
  margin-bottom: 12px;
}

.display-card-header {
  display: flex;
  align-items: center;
  gap: 14px;
  margin-bottom: 16px;
}

.display-icon {
  font-size: 32px;
  width: 56px;
  height: 56px;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #e0f2fe, #bae6fd);
  border-radius: 12px;
}

.display-info { flex: 1; }

.display-name {
  font-size: 15px;
  font-weight: 700;
  color: #333;
  margin-bottom: 4px;
}

.display-meta {
  font-size: 12px;
  color: #666;
}

.display-specs {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 10px;
}

.display-spec-item {
  background: #f8f9fa;
  padding: 12px 8px;
  border-radius: 6px;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.display-spec-label {
  font-size: 10px;
  color: #888;
  margin-bottom: 6px;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.3px;
}

.display-spec-value {
  font-size: 14px;
  font-weight: 700;
  color: #333;
}

.display-spec-value.highlight { color: #0ea5e9; }
</style>
