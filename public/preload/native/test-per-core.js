/**
 * 测试每核心 CPU 使用率 - C++ 原生模块
 */

const sysmon = require('./index.js')

async function test() {
  console.log('=== Per-Core CPU Usage Test (C++ Native) ===\n')
  
  if (!sysmon.isLoaded()) {
    console.log('ERROR: Native module not loaded!')
    console.log('Error:', sysmon.getError())
    console.log('\nPlease build first: npm run build')
    return
  }
  
  console.log('Native module loaded successfully!\n')
  
  // First call to initialize
  console.log('Initializing counters...')
  sysmon.getPerCoreUsage()
  
  // Wait a bit for counters to collect data
  await new Promise(r => setTimeout(r, 1000))
  
  // Get per-core usage
  console.log('\nPer-Core CPU Usage:')
  console.log('----------------------------------------')
  const cores = sysmon.getPerCoreUsage()
  cores.forEach(c => {
    const bar = '█'.repeat(Math.floor(c.loadRaw / 5)) + '░'.repeat(20 - Math.floor(c.loadRaw / 5))
    console.log(`Core ${c.core.toString().padStart(2)}: ${bar} ${c.load.padStart(6)}`)
  })
  
  // Get total CPU usage for comparison
  console.log('----------------------------------------')
  const total = sysmon.getCpuUsage()
  console.log(`Total:    ${total.load}`)
  
  console.log('\n=== Test Complete ===')
}

test()

