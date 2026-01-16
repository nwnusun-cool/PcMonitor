import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [vue()],
  base: './',
  build: {
    // 排除 preload 目录，它会被单独复制
    copyPublicDir: true,
    rollupOptions: {
      external: [/\.node$/]
    }
  },
  publicDir: 'public'
})
