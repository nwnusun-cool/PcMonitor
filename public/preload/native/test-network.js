/**
 * 网络信息测试 - C++ 原生模块
 */

const sysmon = require('./index.js')

console.log('=== Network Info Test (C++ Native) ===\n')

if (!sysmon.isLoaded()) {
  console.log('ERROR: Native module not loaded!')
  console.log('Error:', sysmon.getError())
  console.log('\nPlease build first: npm run build')
  process.exit(1)
}

console.log('Native module loaded successfully!\n')

// 初始化
sysmon.getNetworkStats()

setTimeout(() => {
  const interfaces = sysmon.getNetworkStats()
  
  console.log('Network Interfaces:')
  console.log('='.repeat(60))
  
  interfaces.forEach((iface, i) => {
    console.log(`\n[${i + 1}] ${iface.ifaceName}`)
    console.log('-'.repeat(60))
    console.log('  Type:         ', iface.type)
    console.log('  IPv4:         ', iface.ip4)
    console.log('  IPv6:         ', iface.ip6 || 'N/A')
    console.log('  Subnet:       ', iface.subnet)
    console.log('  MAC:          ', iface.mac)
    console.log('  DHCP:         ', iface.dhcp ? 'Enabled' : 'Disabled')
    console.log('  DNS:          ', iface.dns.join(', ') || 'N/A')
    console.log('')
    console.log('  Link Speed:   ', iface.speed)
    console.log('  Utilization:  ', iface.utilizationFmt)
    console.log('')
    console.log('  Download:     ', iface.rxSec)
    console.log('  Upload:       ', iface.txSec)
    console.log('  Total RX:     ', iface.rxBytes, `(${iface.rxPackets.toLocaleString()} packets)`)
    console.log('  Total TX:     ', iface.txBytes, `(${iface.txPackets.toLocaleString()} packets)`)
  })
  
  console.log('\n' + '='.repeat(60))
  console.log('Test Complete!')
}, 1000)
