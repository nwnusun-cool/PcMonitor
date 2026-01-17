/**
 * 系统监控服务层 - 使用原生模块
 */
const os = require('node:os')

// 尝试加载原生模块
let native = null
try {
  native = require('./native/index')
  if (!native.isLoaded()) native = null
} catch (e) {
  console.error('原生模块加载失败:', e.message)
}

window.__nativeApiLoaded = native !== null

// 格式化工具
function formatBytes(bytes, decimals = 2) {
  if (!bytes || bytes === 0) return '0 B'
  const k = 1024, sizes = ['B', 'KB', 'MB', 'GB', 'TB']
  const i = Math.floor(Math.log(bytes) / Math.log(k))
  return parseFloat((bytes / Math.pow(k, i)).toFixed(decimals)) + ' ' + sizes[i]
}

// 缓存
const cache = { 
  systemInfo: null, 
  cpuStatic: null, 
  gpuInfo: null, 
  memoryHardware: null,
  externalIP: null,
  externalIPTime: 0
}

window.services = {
  // CPU 负载 (同步, <0.1ms)
  getCpuLoad() {
    if (native) return native.getCpuUsage()
    const cpus = os.cpus()
    return { load: '0%', loadUser: '0%', loadSystem: '0%' }
  },

  // 每核心 CPU 使用率
  getPerCoreUsage() {
    if (native) return native.getPerCoreUsage()
    return []
  },

  // 内存信息 (同步, <0.01ms)
  getMemoryInfo() {
    if (native) return native.getMemoryInfo()
    const total = os.totalmem(), free = os.freemem(), used = total - free
    return { total: formatBytes(total), used: formatBytes(used), free: formatBytes(free),
      available: formatBytes(free), usedPercent: ((used / total) * 100).toFixed(1) + '%' }
  },

  // 内存硬件信息 (缓存)
  getMemoryHardware() {
    if (!cache.memoryHardware && native) {
      cache.memoryHardware = native.getMemoryHardware()
    }
    return cache.memoryHardware || { modules: [], usedSlots: 0, totalSlots: 0 }
  },

  // 运行时间 (同步, <0.001ms)
  getUptime() {
    if (native) return native.getUptime()
    const s = os.uptime()
    return `${Math.floor(s / 86400)}天 ${Math.floor((s % 86400) / 3600)}小时 ${Math.floor((s % 3600) / 60)}分钟`
  },

  // 系统统计 (进程数/线程数/句柄数)
  getSystemStats() {
    if (native) return native.getSystemStats()
    return { processCount: 0, threadCount: 0, handleCount: 0 }
  },

  // 系统信息 (缓存)
  async getSystemInfo() {
    if (cache.systemInfo) return cache.systemInfo
    if (native) {
      cache.systemInfo = native.getSystemInfo()
      return cache.systemInfo
    }
    cache.systemInfo = { hostname: os.hostname(), platform: 'Windows', arch: os.arch() }
    return cache.systemInfo
  },

  // CPU 完整信息
  async getCpuInfo() {
    if (native) return native.getCpuInfo()
    if (!cache.cpuStatic) {
      const cpus = os.cpus(), cpu0 = cpus[0] || {}
      cache.cpuStatic = { brand: cpu0.model || 'Unknown', cores: cpus.length,
        speed: cpu0.speed ? (cpu0.speed / 1000).toFixed(2) + ' GHz' : 'Unknown' }
    }
    const load = this.getCpuLoad()
    return { ...cache.cpuStatic, ...load }
  },

  // 磁盘信息
  async getDiskInfo() {
    if (native) return native.getDiskInfo()
    return { partitions: [], totalSize: '0 B', totalUsed: '0 B', totalAvailable: '0 B', totalPercent: '0%' }
  },

  // 磁盘 IO
  getDiskIO() {
    if (native) return native.getDiskIO()
    return { readSec: 0, writeSec: 0, readSecFmt: '0 B/s', writeSecFmt: '0 B/s' }
  },

  // 网络信息
  async getNetworkInfo() {
    if (native) {
      const stats = native.getNetworkStats()
      return { interfaces: stats, virtualInterfaces: [], allInterfaces: stats, stats, gateway: '' }
    }
    const nets = os.networkInterfaces()
    const interfaces = []
    for (const [name, addrs] of Object.entries(nets)) {
      const ipv4 = addrs.find(a => a.family === 'IPv4' && !a.internal)
      if (ipv4) interfaces.push({ iface: name, ifaceName: name, ip4: ipv4.address, mac: ipv4.mac,
        rxSec: '0 B/s', txSec: '0 B/s', rxSecBytes: 0, txSecBytes: 0 })
    }
    return { interfaces, virtualInterfaces: [], allInterfaces: interfaces, stats: [], gateway: '' }
  },

  // 电池信息
  async getBatteryInfo() {
    if (native) return native.getBatteryInfo()
    return { hasBattery: false }
  },

  // GPU 信息 (缓存)
  async getGpuInfo() {
    if (cache.gpuInfo) return cache.gpuInfo
    if (native) {
      cache.gpuInfo = native.getGpuInfo()
      return cache.gpuInfo
    }
    return { controllers: [], displays: [] }
  },

  // 进程信息
  async getProcessInfo() {
    if (native) {
      const p = native.getProcessList()
      return { 
        all: p.count, 
        list: p.list,
        topCpu: p.topCpu, 
        topMem: p.topMem 
      }
    }
    return { all: 0, list: [], topCpu: [], topMem: [] }
  },

  // 外部 IP 和地理位置 (缓存 5 分钟)
  async getExternalIP() {
    const now = Date.now()
    const cacheTime = 5 * 60 * 1000 // 5 minutes
    
    if (cache.externalIP && (now - cache.externalIPTime) < cacheTime) {
      return cache.externalIP
    }

    try {
      const https = require('https')
      const data = await new Promise((resolve, reject) => {
        const timeout = setTimeout(() => reject(new Error('Timeout')), 5000)
        https.get('https://ipinfo.io/json', { timeout: 5000 }, (res) => {
          let body = ''
          res.on('data', chunk => body += chunk)
          res.on('end', () => {
            clearTimeout(timeout)
            try {
              resolve(JSON.parse(body))
            } catch (e) {
              reject(e)
            }
          })
        }).on('error', (e) => {
          clearTimeout(timeout)
          reject(e)
        })
      })

      const result = {
        ip: data.ip || '',
        city: data.city || '',
        region: data.region || '',
        country: data.country || '',
        location: data.loc || '', // "lat,lon"
        isp: data.org || '',
        timezone: data.timezone || '',
        // Format display strings
        locationText: [data.city, data.region, data.country].filter(Boolean).join(', '),
        ispText: (data.org || '').replace(/^AS\d+\s+/, '') // Remove AS number prefix
      }

      cache.externalIP = result
      cache.externalIPTime = now
      return result
    } catch (e) {
      console.error('获取外部 IP 失败:', e.message)
      return {
        ip: '',
        city: '',
        region: '',
        country: '',
        location: '',
        isp: '',
        timezone: '',
        locationText: '获取失败',
        ispText: '',
        error: e.message
      }
    }
  },

  // 网络连接监控
  getNetworkConnections() {
    if (native) return native.getNetworkConnections()
    return { tcp: [], udp: [], byProcess: [], totalTcp: 0, totalUdp: 0, totalEstablished: 0, totalListening: 0 }
  }
}
