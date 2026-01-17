/**
 * 磁盘性能测试 - C++ 原生模块
 */

const sysmon = require('./index.js')

console.log('=== Disk Performance Test (C++ Native) ===\n')

if (!sysmon.isLoaded()) {
  console.log('ERROR: Native module not loaded!')
  console.log('Error:', sysmon.getError())
  console.log('\nPlease build first: npm run build')
  process.exit(1)
}

console.log('Native module loaded successfully!\n')

// 初始化计数器
console.log('Initializing counters...')
sysmon.getDiskIO()

// 等待 1 秒收集数据
setTimeout(() => {
  console.log('\nDisk Performance Metrics:')
  console.log('----------------------------------------')
  
  const io = sysmon.getDiskIO()
  
  console.log('IO Throughput:')
  console.log('  Read:  ', io.readSecFmt)
  console.log('  Write: ', io.writeSecFmt)
  console.log('')
  
  console.log('Performance:')
  console.log('  Active Time:      ', io.activeTimeFmt)
  console.log('  Queue Length:     ', io.queueLengthFmt)
  console.log('  Avg Read Time:    ', io.avgReadTimeFmt)
  console.log('  Avg Write Time:   ', io.avgWriteTimeFmt)
  console.log('  Reads/sec:        ', io.readsPerSecFmt)
  console.log('  Writes/sec:       ', io.writesPerSecFmt)
  
  console.log('\n=== Test Complete ===')
}, 1000)
