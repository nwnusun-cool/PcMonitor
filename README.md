# PcMonitor

一款基于 uTools 的 Windows 系统资源监控插件，实时查看 CPU、内存、磁盘、网络、GPU 等硬件信息。

![Vue](https://img.shields.io/badge/Vue-3.5-4FC08D?logo=vue.js)
![Vite](https://img.shields.io/badge/Vite-6.0-646CFF?logo=vite)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows)
![License](https://img.shields.io/badge/License-MIT-green)

## ✨ 功能特性

- 📊 **系统概览** - 一目了然查看系统核心指标
- ⚡ **CPU 监控** - 实时负载、频率、温度、缓存信息
- 💾 **内存监控** - 内存使用率、交换空间状态
- 💿 **磁盘信息** - 分区详情、物理磁盘、存储统计
- 🌐 **网络监控** - 实时网速、网卡信息、流量统计
- 🎮 **GPU 信息** - 显卡型号、显存、显示器参数
- 📋 **进程管理** - CPU/内存占用 Top 10 进程
- 🔋 **电池状态** - 笔记本电池电量与充电状态

## 📸 截图预览

> 启动插件后，输入关键词即可打开系统监控面板

**支持的关键词：**
- `系统监控`
- `资源监控`
- `硬件信息`
- `system monitor`
- `sysinfo`

## 🚀 快速开始

### 环境要求

- [Node.js](https://nodejs.org/) >= 18
- [uTools](https://u.tools/) 已安装

### 安装依赖

```bash
# 安装主项目依赖
npm install

# 安装 preload 依赖
cd public/preload
npm install
cd ../..
```

### 开发调试

```bash
# 启动开发服务器
npm run dev
```

然后在 uTools 中进入开发者模式，加载 `public/plugin.json` 即可调试。

### 构建发布

```bash
npm run build
```

构建产物在 `dist` 目录，将 `dist` 目录内容与 `public` 目录合并后打包为 `.upx` 文件即可发布。

## 📁 项目结构

```
PcMonitor/
├── public/
│   ├── plugin.json          # uTools 插件配置
│   ├── logo.png             # 插件图标
│   └── preload/
│       ├── services.js      # Node.js 系统信息服务
│       └── package.json     # preload 依赖
├── src/
│   ├── App.vue              # 应用入口
│   ├── main.js              # Vue 入口
│   └── SysMonitor/          # 系统监控模块
│       ├── index.vue        # 主界面
│       ├── components/      # 功能面板组件
│       │   ├── Overview.vue
│       │   ├── CpuPanel.vue
│       │   ├── MemoryPanel.vue
│       │   ├── DiskPanel.vue
│       │   ├── NetworkPanel.vue
│       │   ├── GpuPanel.vue
│       │   └── ProcessPanel.vue
│       ├── composables/     # 组合式函数
│       └── styles/          # 样式文件
├── package.json
└── vite.config.js
```

## 🛠️ 技术栈

- **前端框架**: Vue 3 + Composition API
- **构建工具**: Vite 6
- **系统信息**: [systeminformation](https://github.com/sebhildebrandt/systeminformation)
- **运行平台**: uTools

## 📝 开发说明

### Preload 服务

`public/preload/services.js` 提供了以下系统信息获取方法：

| 方法 | 说明 |
|------|------|
| `getSystemInfo()` | 获取操作系统、主机信息 |
| `getCpuInfo()` | 获取 CPU 详细信息 |
| `getMemoryInfo()` | 获取内存使用情况 |
| `getDiskInfo()` | 获取磁盘分区信息 |
| `getNetworkInfo()` | 获取网络接口和流量 |
| `getGpuInfo()` | 获取显卡和显示器信息 |
| `getBatteryInfo()` | 获取电池状态 |
| `getProcessInfo()` | 获取进程列表 |
| `getUptime()` | 获取系统运行时间 |

### 数据刷新

- 核心指标（CPU、内存、网络）每秒刷新
- 进程列表每 5 秒刷新
- 静态信息（系统、磁盘、GPU）仅首次加载

## 📄 License

[MIT](LICENSE)

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

Made with ❤️ for uTools
