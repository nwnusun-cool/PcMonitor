/**
 * 测试原生模块性能
 */
const sysmon = require('./index')

console.log('========== sysmon-native 性能测试 ==========\n')

if (!sysmon.isLoaded()) {
  console.log('❌ 模块加载失败:', sysmon.getError())
  process.exit(1)
}

console.log('✅ 模块加载成功\n')

// 测试 CPU（需要多次采样）
console.log('CPU 使用率测试（每秒采样一次，共 5 次）:')
let count = 0
const cpuTest = setInterval(() => {
  const cpu = sysmon.getCpuUsage()
  console.log(`  [${++count}] 总: ${cpu.load}, 用户: ${cpu.loadUser}, 系统: ${cpu.loadSystem}`)
  if (count >= 5) {
    clearInterval(cpuTest)
    
    // 继续其他测试
    console.log('\n系统信息:', sysmon.getSystemInfo())
    console.log('\nCPU信息:', sysmon.getCpuInfo())
    console.log('\n内存信息:', sysmon.getMemoryInfo())
    console.log('\n运行时间:', sysmon.getUptime())
    console.log('\nGPU信息:', sysmon.getGpuInfo())
    console.log('\n电池信息:', sysmon.getBatteryInfo())
    
    const disk = sysmon.getDiskInfo()
    console.log('\n磁盘信息:', disk.totalSize, '已用:', disk.totalUsed)
    console.log('  物理磁盘:', disk.physical)
    
    const net = sysmon.getNetworkStats()
    console.log('\n网络接口:')
    net.forEach(n => console.log('  ', n.iface, n.ip4, '↓', n.rxSec, '↑', n.txSec))
    
    const proc = sysmon.getProcessList()
    console.log('\n进程数:', proc.count)
    
    console.log('\n========== 测试完成 ==========')
  }
}, 1000)

// 初始化第一次采样
sysmon.getCpuUsage()
