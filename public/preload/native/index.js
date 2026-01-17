/**
 * sysmon-native - Windows native system monitor
 */
let native = null, loadError = null
try { native = require('./build/Release/sysmon.node') } catch (e) { loadError = e.message }

function formatBytes(bytes, decimals = 2) {
  if (!bytes || bytes === 0) return '0 B'
  const k = 1024, sizes = ['B', 'KB', 'MB', 'GB', 'TB']
  const i = Math.floor(Math.log(bytes) / Math.log(k))
  return parseFloat((bytes / Math.pow(k, i)).toFixed(decimals)) + ' ' + sizes[i]
}

function formatUptime(seconds) {
  const d = Math.floor(seconds / 86400), h = Math.floor((seconds % 86400) / 3600), m = Math.floor((seconds % 3600) / 60)
  return `${d}天 ${h}小时 ${m}分钟`
}

module.exports = {
  isLoaded: () => native !== null,
  getError: () => loadError,

  getMemoryInfo() {
    if (!native) return null
    const m = native.getMemoryInfo()
    return {
      total: formatBytes(m.total), used: formatBytes(m.used), free: formatBytes(m.free),
      available: formatBytes(m.free), usedPercent: m.usedPercent.toFixed(1) + '%',
      swapTotal: formatBytes(m.swapTotal || 0),
      swapUsed: formatBytes(m.swapUsed || 0),
      swapFree: formatBytes(m.swapFree || 0),
      totalRaw: m.total, usedRaw: m.used, freeRaw: m.free,
      // Extended info from GetPerformanceInfo
      committed: formatBytes(m.committed || 0),
      commitLimit: formatBytes(m.commitLimit || 0),
      cached: formatBytes(m.cached || 0),
      pagedPool: formatBytes(m.pagedPool || 0),
      nonPagedPool: formatBytes(m.nonPagedPool || 0),
      committedRaw: m.committed || 0,
      cachedRaw: m.cached || 0,
      pagedPoolRaw: m.pagedPool || 0,
      nonPagedPoolRaw: m.nonPagedPool || 0
    }
  },

  // Memory hardware info (via C++ WMI)
  getMemoryHardware() {
    if (!native) return { modules: [], usedSlots: 0, totalSlots: 0, totalCapacity: '0 B', speed: 0, type: 'Unknown' }
    
    const hw = native.getMemoryHardware()
    const modules = (hw.modules || []).map(m => ({
      bank: m.bank || '',
      capacity: formatBytes(m.capacity || 0),
      capacityRaw: m.capacity || 0,
      speed: m.speed || 0,
      type: m.type || 'Unknown',
      formFactor: m.formFactor || 'Unknown',
      manufacturer: m.manufacturer || '',
      partNumber: m.partNumber || ''
    }))
    
    return {
      modules,
      usedSlots: hw.usedSlots || modules.length,
      totalSlots: hw.totalSlots || modules.length,
      totalCapacity: formatBytes(modules.reduce((sum, m) => sum + m.capacityRaw, 0)),
      speed: modules[0]?.speed || 0,
      type: modules[0]?.type || 'Unknown'
    }
  },

  getCpuUsage() {
    if (!native) return null
    const c = native.getCpuUsage()
    return { load: c.load.toFixed(1) + '%', loadRaw: c.load }
  },

  getPerCoreUsage() {
    if (!native) return null
    const cores = native.getPerCoreUsage()
    return cores.map((load, i) => ({
      core: i,
      load: load.toFixed(1) + '%',
      loadRaw: load
    }))
  },

  getUptime() {
    if (!native) return null
    return formatUptime(native.getUptime().seconds)
  },

  getSystemStats() {
    if (!native) return null
    const s = native.getSystemStats()
    return {
      processCount: s.processCount,
      threadCount: s.threadCount,
      handleCount: s.handleCount
    }
  },

  getSystemInfo() {
    if (!native) return null
    const s = native.getSystemInfo()
    return { 
      hostname: s.hostname, platform: s.platform, arch: s.arch, build: s.build,
      manufacturer: s.manufacturer || '未知', model: s.model || '未知'
    }
  },

  getCpuInfo() {
    if (!native) return null
    const c = native.getCpuInfo(), u = native.getCpuUsage()
    return {
      brand: (c.brand || 'Unknown').trim(),
      manufacturer: (c.brand || '').includes('Intel') ? 'Intel' : (c.brand || '').includes('AMD') ? 'AMD' : 'Unknown',
      cores: c.cores,
      physicalCores: c.physicalCores || Math.floor(c.cores / 2),
      speed: c.speed ? c.speed.toFixed(2) + ' GHz' : 'Unknown',
      currentSpeed: c.currentSpeed ? c.currentSpeed.toFixed(2) + ' GHz' : (c.speed ? c.speed.toFixed(2) + ' GHz' : 'Unknown'),
      virtualization: c.virtualization === 'Supported' ? '支持' : '未知',
      load: u.load.toFixed(1) + '%'
    }
  },

  getGpuInfo() {
    if (!native) return null
    const g = native.getGpuInfo()
    return { 
      controllers: (g.controllers || []).map(gpu => ({
        model: gpu.model || 'Unknown',
        vendor: gpu.vendor || (gpu.model?.includes('NVIDIA') ? 'NVIDIA' : gpu.model?.includes('AMD') ? 'AMD' : 
          gpu.model?.includes('Intel') ? 'Intel' : 'Unknown'),
        vram: gpu.vram ? formatBytes(gpu.vram) : '共享内存',
        bus: gpu.bus || 'PCI'
      })), 
      displays: (g.displays || []).map(d => ({
        model: d.model || '显示器',
        main: d.main || false,
        resolutionX: d.resolutionX || 0,
        resolutionY: d.resolutionY || 0,
        currentResX: d.currentResX || d.resolutionX || 0,
        currentResY: d.currentResY || d.resolutionY || 0,
        refreshRate: d.refreshRate ? d.refreshRate + ' Hz' : '未知',
        pixelDepth: d.pixelDepth ? d.pixelDepth + ' bit' : '未知'
      }))
    }
  },

  getBatteryInfo() {
    if (!native) return null
    const b = native.getBatteryInfo()
    if (!b.hasBattery) return { hasBattery: false }
    return { hasBattery: true, percent: b.percent + '%', isCharging: b.isCharging, timeRemaining: 'Unknown' }
  },

  getDiskInfo() {
    if (!native) return null
    const d = native.getDiskInfo()
    let totalSize = 0, totalUsed = 0
    const partitions = (d.partitions || []).map(p => {
      totalSize += p.size; totalUsed += p.used
      return { fs: p.fs, mount: p.mount, type: p.type || 'NTFS',
        size: formatBytes(p.size), used: formatBytes(p.used), available: formatBytes(p.free),
        usedPercent: p.usedPercent.toFixed(1) + '%', sizeBytes: p.size, usedBytes: p.used }
    })
    const physical = (d.physical || []).map(p => ({
      name: p.name || 'Unknown', vendor: p.vendor || '',
      size: formatBytes(p.size || 0), interfaceType: p.interfaceType || 'Unknown'
    }))
    return { partitions, physical, totalSize: formatBytes(totalSize), totalUsed: formatBytes(totalUsed),
      totalAvailable: formatBytes(totalSize - totalUsed),
      totalPercent: totalSize > 0 ? ((totalUsed / totalSize) * 100).toFixed(1) + '%' : '0%' }
  },

  getDiskIO() {
    if (!native) return { 
      readSec: 0, writeSec: 0, readSecFmt: '0 B/s', writeSecFmt: '0 B/s',
      activeTime: 0, queueLength: 0, avgReadTime: 0, avgWriteTime: 0,
      readsPerSec: 0, writesPerSec: 0
    }
    const io = native.getDiskIO()
    return {
      readSec: io.readSec,
      writeSec: io.writeSec,
      readSecFmt: formatBytes(io.readSec) + '/s',
      writeSecFmt: formatBytes(io.writeSec) + '/s',
      // Performance metrics
      activeTime: io.activeTime || 0,
      activeTimeFmt: (io.activeTime || 0).toFixed(1) + '%',
      queueLength: io.queueLength || 0,
      queueLengthFmt: (io.queueLength || 0).toFixed(2),
      avgReadTime: io.avgReadTime || 0,
      avgReadTimeFmt: (io.avgReadTime || 0).toFixed(1) + ' ms',
      avgWriteTime: io.avgWriteTime || 0,
      avgWriteTimeFmt: (io.avgWriteTime || 0).toFixed(1) + ' ms',
      readsPerSec: io.readsPerSec || 0,
      readsPerSecFmt: (io.readsPerSec || 0).toFixed(1),
      writesPerSec: io.writesPerSec || 0,
      writesPerSecFmt: (io.writesPerSec || 0).toFixed(1)
    }
  },

  getNetworkStats() {
    if (!native) return null
    const n = native.getNetworkStats()
    return (n.interfaces || []).map(i => ({ iface: i.iface, ifaceName: i.iface, ip4: i.ip4, mac: i.mac,
      speed: i.speed ? i.speed + ' Mbps' : 'Unknown', rxBytes: formatBytes(i.rxBytes), txBytes: formatBytes(i.txBytes),
      rxSec: formatBytes(i.rxSec) + '/s', txSec: formatBytes(i.txSec) + '/s', rxSecBytes: i.rxSec, txSecBytes: i.txSec }))
  },

  getProcessList() {
    if (!native) return null
    const p = native.getProcessList(), totalMem = native.getMemoryInfo().total
    const list = (p.processes || []).map(proc => ({
      pid: proc.pid,
      name: proc.name,
      memory: formatBytes(proc.memory),
      memoryRaw: proc.memory,
      memPercent: ((proc.memory / totalMem) * 100).toFixed(1) + '%',
      memPercentRaw: (proc.memory / totalMem) * 100,
      threads: proc.threads || 0,
      handles: proc.handles || 0,
      cpu: (proc.cpu || 0).toFixed(1) + '%',
      cpuRaw: proc.cpu || 0
    }))
    
    // Sort by memory for topMem
    const topMem = [...list].sort((a, b) => b.memoryRaw - a.memoryRaw).slice(0, 15)
    // Sort by CPU for topCpu
    const topCpu = [...list].sort((a, b) => b.cpuRaw - a.cpuRaw).slice(0, 15)
    
    return { count: p.count, list, topMem, topCpu }
  }
}
