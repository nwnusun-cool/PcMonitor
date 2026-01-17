/**
 * Test C++ network connections module
 */

let native;
try {
  native = require('./build/Release/sysmon.node');
  console.log('Native module loaded successfully!\n');
} catch (e) {
  console.error('Failed to load native module:', e.message);
  process.exit(1);
}

console.log('=== Network Connections Test (C++ Native) ===\n');

const data = native.getNetworkConnections();

console.log(`TCP Connections: ${data.tcp.length}`);
console.log(`UDP Connections: ${data.udp.length}`);
console.log(`Total: ${data.tcp.length + data.udp.length}\n`);

// Show top 10 ESTABLISHED TCP connections
const established = data.tcp.filter(c => c.state === 'ESTABLISHED').slice(0, 10);
console.log('Top 10 ESTABLISHED TCP Connections:');
console.log('='.repeat(80));
established.forEach((conn, i) => {
  console.log(`[${i+1}] ${conn.process || 'Unknown'} (PID: ${conn.pid})`);
  console.log(`    ${conn.localAddress}:${conn.localPort} -> ${conn.remoteAddress}:${conn.remotePort}`);
  console.log(`    State: ${conn.state}`);
});

// Show top 10 LISTENING TCP connections
console.log('\nTop 10 LISTENING TCP Connections:');
console.log('='.repeat(80));
const listening = data.tcp.filter(c => c.state === 'LISTENING').slice(0, 10);
listening.forEach((conn, i) => {
  console.log(`[${i+1}] ${conn.process || 'Unknown'} (PID: ${conn.pid})`);
  console.log(`    ${conn.localAddress}:${conn.localPort}`);
});

// Show top 10 UDP connections
console.log('\nTop 10 UDP Connections:');
console.log('='.repeat(80));
data.udp.slice(0, 10).forEach((conn, i) => {
  console.log(`[${i+1}] ${conn.process || 'Unknown'} (PID: ${conn.pid})`);
  console.log(`    ${conn.localAddress}:${conn.localPort}`);
});

// Group by process
const byProcess = {};
[...data.tcp, ...data.udp].forEach(conn => {
  const key = `${conn.process} (${conn.pid})`;
  if (!byProcess[key]) byProcess[key] = 0;
  byProcess[key]++;
});

console.log('\nTop 10 Processes by Connection Count:');
console.log('='.repeat(80));
Object.entries(byProcess)
  .sort((a, b) => b[1] - a[1])
  .slice(0, 10)
  .forEach(([proc, count], i) => {
    console.log(`[${i+1}] ${proc}: ${count} connections`);
  });

console.log('\n=== Test Complete ===');
