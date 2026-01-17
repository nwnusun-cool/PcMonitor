/**
 * Test getExternalIP service function
 */

// Mock window object
global.window = { __nativeApiLoaded: false };

// Load services
require('./services.js');

async function test() {
  console.log('=== Testing getExternalIP Service ===\n');
  
  console.log('1. First call (should fetch from API)...');
  const start1 = Date.now();
  const result1 = await window.services.getExternalIP();
  const time1 = Date.now() - start1;
  
  console.log('Result:');
  console.log('  IP:', result1.ip);
  console.log('  Location:', result1.locationText);
  console.log('  ISP:', result1.ispText);
  console.log('  Timezone:', result1.timezone);
  console.log('  Time taken:', time1 + 'ms');
  
  console.log('\n2. Second call (should use cache)...');
  const start2 = Date.now();
  const result2 = await window.services.getExternalIP();
  const time2 = Date.now() - start2;
  
  console.log('Result:');
  console.log('  IP:', result2.ip);
  console.log('  Location:', result2.locationText);
  console.log('  ISP:', result2.ispText);
  console.log('  Time taken:', time2 + 'ms');
  console.log('  Cached:', time2 < 10 ? '✓ Yes' : '✗ No');
  
  console.log('\n=== Test Complete ===');
}

test().catch(console.error);
