const os = require('node:os')
const si = require('systeminformation')

// 格式化字节
function formatBytes(bytes, decimals = 2) {
  if (bytes === 0) return '0 B'
  if (!bytes || isNaN(bytes)) return '0 B'
  const k = 1024
  const sizes = ['B', 'KB', 'MB', 'GB', 'TB']
  const i = Math.floor(Math.log(bytes) / Math.log(k))
  return parseFloat((bytes / Math.pow(k, i)).toFixed(decimals)) + ' ' + sizes[i]
}

// 格式化运行时间
function formatUptime(seconds) {
  const days = Math.floor(seconds / 86400)
  const hours = Math.floor((seconds % 86400) / 3600)
  const minutes = Math.floor((seconds % 3600) / 60)
  return `${days}天 ${hours}小时 ${minutes}分钟`
}

// 缓存静态 CPU 信息（不会变化的数据）
let cachedCpuStatic = null

window.services = {
  // 获取基础系统信息
  async getSystemInfo() {
    const [system, bios, baseboard, osInfo] = await Promise.all([
      si.system(),
      si.bios(),
      si.baseboard(),
      si.osInfo()
    ])
    
    // 处理 Windows 版本显示
    let platformName = osInfo.distro || os.platform()
    // 如果是乱码或包含特殊字符，使用英文版本名称
    if (/[\uFFFD]/.test(platformName) || !/^[\x00-\x7F\u4e00-\u9fa5]+$/.test(platformName)) {
      // 根据版本号判断 Windows 版本
      const release = osInfo.release || os.release()
      const build = parseInt(osInfo.build) || 0
      if (build >= 22000) {
        platformName = 'Windows 11'
      } else if (build >= 10240) {
        platformName = 'Windows 10'
      } else {
        platformName = 'Windows'
      }
    }
    
    return {
      hostname: os.hostname(),
      platform: platformName,
      arch: osInfo.arch || os.arch(),
      release: osInfo.release || os.release(),
      manufacturer: system.manufacturer,
      model: system.model,
      biosVendor: bios.vendor,
      biosVersion: bios.version,
      baseboardManufacturer: baseboard.manufacturer,
      baseboardModel: baseboard.model
    }
  },

  // 获取 CPU 信息（优化：缓存静态数据）
  async getCpuInfo() {
    // 格式化缓存大小
    const formatCache = (bytes) => {
      if (!bytes || bytes === 0) return null
      if (bytes < 1024) return bytes + ' B'
      if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(0) + ' KB'
      return (bytes / (1024 * 1024)).toFixed(0) + ' MB'
    }
    
    // 首次加载时获取静态 CPU 信息
    if (!cachedCpuStatic) {
      const cpu = await si.cpu()
      
      // 判断虚拟化支持：检查 flags 中是否包含虚拟化相关标志
      // Intel: vmx, VT-x | AMD: svm, AMD-V
      const flags = (cpu.flags || '').toLowerCase()
      const hasVirtualization = cpu.virtualization === true || 
        flags.includes('vmx') || 
        flags.includes('svm') ||
        flags.includes('hypervisor') ||
        // 现代 CPU（2010年后）基本都支持虚拟化
        (cpu.cores >= 2)
      
      cachedCpuStatic = {
        manufacturer: cpu.manufacturer || '未知',
        brand: cpu.brand || '未知处理器',
        cores: cpu.cores || 0,
        physicalCores: cpu.physicalCores || 0,
        speed: cpu.speed ? cpu.speed + ' GHz' : '未知',
        speedMin: cpu.speedMin ? cpu.speedMin + ' GHz' : null,
        speedMax: cpu.speedMax ? cpu.speedMax + ' GHz' : null,
        cache: {
          l1d: formatCache(cpu.cache?.l1d),
          l1i: formatCache(cpu.cache?.l1i),
          l2: formatCache(cpu.cache?.l2),
          l3: formatCache(cpu.cache?.l3)
        },
        virtualization: hasVirtualization ? '支持' : '不支持',
        flags: cpu.flags || ''
      }
    }
    
    // 只获取动态数据（频率、温度、负载）
    const [cpuSpeed, cpuTemp, currentLoad] = await Promise.all([
      si.cpuCurrentSpeed(),
      si.cpuTemperature(),
      si.currentLoad()
    ])
    
    return {
      ...cachedCpuStatic,
      currentSpeed: cpuSpeed.avg ? cpuSpeed.avg + ' GHz' : '未知',
      temperature: cpuTemp.main ? cpuTemp.main + '°C' : null,
      load: currentLoad.currentLoad.toFixed(1) + '%',
      loadUser: currentLoad.currentLoadUser.toFixed(1) + '%',
      loadSystem: currentLoad.currentLoadSystem.toFixed(1) + '%'
    }
  },

  // 获取内存信息
  async getMemoryInfo() {
    const mem = await si.mem()
    return {
      total: formatBytes(mem.total),
      used: formatBytes(mem.used),
      free: formatBytes(mem.free),
      available: formatBytes(mem.available),
      usedPercent: ((mem.used / mem.total) * 100).toFixed(1) + '%',
      swapTotal: formatBytes(mem.swaptotal),
      swapUsed: formatBytes(mem.swapused),
      swapFree: formatBytes(mem.swapfree)
    }
  },

  // 获取磁盘信息
  async getDiskInfo() {
    try {
      const [disks, diskLayout] = await Promise.all([
        si.fsSize(),
        si.diskLayout()
      ])
      
      // 计算总存储统计
      let totalSize = 0
      let totalUsed = 0
      
      const partitions = disks.map(disk => {
        totalSize += disk.size || 0
        totalUsed += disk.used || 0
        return {
          fs: disk.fs || '未知',
          type: disk.type || '未知',
          mount: disk.mount || '未知',
          size: formatBytes(disk.size || 0),
          used: formatBytes(disk.used || 0),
          available: formatBytes(disk.available || 0),
          usedPercent: disk.use ? disk.use.toFixed(1) + '%' : '0%',
          sizeBytes: disk.size || 0,
          usedBytes: disk.used || 0
        }
      })
      
      const physical = diskLayout.map(disk => ({
        name: disk.name || '未知磁盘',
        type: disk.type || 'Unknown',
        vendor: disk.vendor || '未知厂商',
        size: formatBytes(disk.size || 0),
        interfaceType: disk.interfaceType || '未知'
      }))
      
      return {
        partitions,
        physical,
        totalSize: formatBytes(totalSize),
        totalUsed: formatBytes(totalUsed),
        totalAvailable: formatBytes(totalSize - totalUsed),
        totalPercent: totalSize > 0 ? ((totalUsed / totalSize) * 100).toFixed(1) + '%' : '0%'
      }
    } catch (e) {
      console.error('获取磁盘信息失败:', e)
      return {
        partitions: [],
        physical: [],
        totalSize: '0 B',
        totalUsed: '0 B',
        totalAvailable: '0 B',
        totalPercent: '0%'
      }
    }
  },

  // 获取网络信息
  async getNetworkInfo() {
    const [interfaces, stats] = await Promise.all([
      si.networkInterfaces(),
      si.networkStats()
    ])
    
    // 修复网络接口名称乱码问题
    const fixInterfaceName = (name) => {
      if (!name) return '未知'
      // 尝试修复编码问题
      try {
        // 如果包含乱码字符，尝试使用 ifaceName 或其他字段
        if (/[\uFFFD\u0000-\u001F]/.test(name)) {
          return name.replace(/[\uFFFD\u0000-\u001F]/g, '').trim() || '网络接口'
        }
        return name
      } catch (e) {
        return '网络接口'
      }
    }
    
    return {
      interfaces: interfaces.filter(i => i.ip4).map(i => ({
        iface: fixInterfaceName(i.iface),
        ifaceName: i.ifaceName || i.iface,
        ip4: i.ip4,
        ip6: i.ip6,
        mac: i.mac,
        type: i.type,
        speed: i.speed ? i.speed + ' Mbps' : '未知'
      })),
      stats: stats.map(s => ({
        iface: fixInterfaceName(s.iface),
        rxBytes: formatBytes(s.rx_bytes || 0),
        txBytes: formatBytes(s.tx_bytes || 0),
        rxSec: formatBytes(s.rx_sec || 0) + '/s',
        txSec: formatBytes(s.tx_sec || 0) + '/s',
        rxSecBytes: s.rx_sec || 0,
        txSecBytes: s.tx_sec || 0
      }))
    }
  },

  // 获取 GPU 信息
  async getGpuInfo() {
    const graphics = await si.graphics()
    
    // 过滤掉虚拟显卡（如向日葵等远程软件的虚拟显卡）
    const controllers = graphics.controllers
      .filter(gpu => gpu.vram > 0 || gpu.bus === 'PCI')
      .map(gpu => ({
        vendor: gpu.vendor || '未知厂商',
        model: gpu.model || '未知型号',
        vram: gpu.vram ? gpu.vram + ' MB' : '共享内存',
        vramDynamic: gpu.vramDynamic,
        bus: gpu.bus || '未知',
        subDeviceId: gpu.subDeviceId || null
      }))
    
    // 显示器信息
    const displays = graphics.displays.map(d => ({
      model: d.model || '显示器',
      main: d.main,
      builtin: d.builtin,
      connection: d.connection || '未知',
      resolutionX: d.resolutionX || 0,
      resolutionY: d.resolutionY || 0,
      currentResX: d.currentResX || 0,
      currentResY: d.currentResY || 0,
      refreshRate: d.currentRefreshRate ? d.currentRefreshRate + ' Hz' : '未知',
      pixelDepth: d.pixelDepth ? d.pixelDepth + ' bit' : '未知',
      sizeX: d.sizeX || 0,
      sizeY: d.sizeY || 0
    }))
    
    return { controllers, displays }
  },

  // 获取电池信息
  async getBatteryInfo() {
    const battery = await si.battery()
    if (!battery.hasBattery) {
      return { hasBattery: false }
    }
    return {
      hasBattery: true,
      percent: battery.percent + '%',
      isCharging: battery.isCharging,
      timeRemaining: battery.timeRemaining ? battery.timeRemaining + ' 分钟' : '计算中',
      voltage: battery.voltage ? battery.voltage + ' V' : '未知',
      manufacturer: battery.manufacturer || '未知'
    }
  },

  // 获取进程信息
  async getProcessInfo() {
    const processes = await si.processes()
    
    // 过滤掉 System Idle Process
    const filteredList = processes.list.filter(p => p.pid !== 0 && p.name !== 'System Idle Process')
    
    const topCpu = filteredList
      .sort((a, b) => b.cpu - a.cpu)
      .slice(0, 10)
      .map(p => ({
        name: p.name,
        pid: p.pid,
        cpu: p.cpu.toFixed(1) + '%',
        cpuRaw: p.cpu,
        mem: p.mem.toFixed(1) + '%',
        memRaw: p.mem
      }))
    
    const topMem = filteredList
      .sort((a, b) => b.mem - a.mem)
      .slice(0, 10)
      .map(p => ({
        name: p.name,
        pid: p.pid,
        cpu: p.cpu.toFixed(1) + '%',
        cpuRaw: p.cpu,
        mem: p.mem.toFixed(1) + '%',
        memRaw: p.mem
      }))
    
    return {
      all: processes.all,
      running: processes.running,
      blocked: processes.blocked,
      sleeping: processes.sleeping,
      topCpu,
      topMem
    }
  },

  // 获取系统运行时间
  getUptime() {
    return formatUptime(os.uptime())
  }
}
