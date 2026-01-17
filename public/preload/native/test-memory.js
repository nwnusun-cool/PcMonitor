/**
 * 内存信息测试 - C++ 原生模块
 */

const sysmon = require('./index.js')

console.log('=== Memory Info Test (C++ Native) ===\n')

if (!sysmon.isLoaded()) {
  console.log('ERROR: Native module not loaded!')
  console.log('Error:', sysmon.getError())
  console.log('\nPlease build first: npm run build')
  process.exit(1)
}

console.log('Native module loaded successfully!\n')

// 1. 基本内存信息
console.log('1. Basic Memory Info:')
const mem = sysmon.getMemoryInfo()
console.log('   Total:', mem.total)
console.log('   Used:', mem.used)
console.log('   Free:', mem.free)
console.log('   Usage:', mem.usedPercent)
console.log('   Committed:', mem.committed)
console.log('   Commit Limit:', mem.commitLimit)
console.log('   Cached:', mem.cached)
console.log('   Paged Pool:', mem.pagedPool)
console.log('   Non-Paged Pool:', mem.nonPagedPool)
console.log('')

// 2. 内存硬件信息
console.log('2. Memory Hardware Info:')
const hw = sysmon.getMemoryHardware()
console.log('   Type:', hw.type)
console.log('   Speed:', hw.speed, 'MHz')
console.log('   Slots:', hw.usedSlots, '/', hw.totalSlots, 'used')
console.log('   Total Capacity:', hw.totalCapacity)
console.log('')

console.log('   Modules:')
hw.modules.forEach((m, i) => {
  console.log(`   --- Module ${i + 1} ---`)
  console.log(`   Bank: ${m.bank}`)
  console.log(`   Capacity: ${m.capacity}`)
  console.log(`   Speed: ${m.speed} MHz`)
  console.log(`   Type: ${m.type}`)
  console.log(`   Form: ${m.formFactor}`)
  console.log(`   Manufacturer: ${m.manufacturer}`)
  console.log(`   Part Number: ${m.partNumber}`)
})

console.log('\n=== Test Complete ===')
