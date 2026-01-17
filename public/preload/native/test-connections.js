/**
 * Test network connections API
 */
const { execSync } = require('child_process');

console.log('=== Testing Network Connections ===\n');

// Test 1: PowerShell Get-NetTCPConnection
console.log('1. Testing Get-NetTCPConnection (PowerShell)...');
try {
  const result = execSync(
    'powershell -Command "Get-NetTCPConnection | Where-Object {$_.State -eq \'Established\'} | Select-Object -First 5 LocalAddress,LocalPort,RemoteAddress,RemotePort,State,OwningProcess | ConvertTo-Json"',
    { encoding: 'utf8', timeout: 5000 }
  );
  const data = JSON.parse(result);
  const connections = Array.isArray(data) ? data : [data];
  console.log('✓ Success! Found', connections.length, 'connections');
  connections.forEach((conn, i) => {
    console.log(`  [${i+1}] ${conn.LocalAddress}:${conn.LocalPort} -> ${conn.RemoteAddress}:${conn.RemotePort} (PID: ${conn.OwningProcess})`);
  });
} catch (e) {
  console.log('✗ Failed:', e.message);
}

// Test 2: netstat command
console.log('\n2. Testing netstat command...');
try {
  const result = execSync('netstat -ano -p TCP', { encoding: 'utf8', timeout: 5000 });
  const lines = result.split('\n').filter(l => l.includes('ESTABLISHED')).slice(0, 5);
  console.log('✓ Success! Sample connections:');
  lines.forEach(line => console.log('  ' + line.trim()));
} catch (e) {
  console.log('✗ Failed:', e.message);
}

console.log('\n=== Test Complete ===');
console.log('\nFor C++ implementation:');
console.log('- Use GetExtendedTcpTable (iphlpapi.h)');
console.log('- Get process name from PID using OpenProcess + GetModuleFileNameEx');
console.log('- Support both TCP and UDP');
console.log('- Filter by state (ESTABLISHED, LISTENING, etc.)');
