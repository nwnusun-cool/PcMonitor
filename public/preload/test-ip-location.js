/**
 * Test external IP and geolocation APIs
 */
const https = require('https');

function httpGet(url) {
  return new Promise((resolve, reject) => {
    https.get(url, { timeout: 5000 }, (res) => {
      let data = '';
      res.on('data', chunk => data += chunk);
      res.on('end', () => {
        try {
          resolve(JSON.parse(data));
        } catch (e) {
          resolve(data);
        }
      });
    }).on('error', reject).on('timeout', () => reject(new Error('Timeout')));
  });
}

async function testAPIs() {
  console.log('=== Testing IP Geolocation APIs ===\n');

  // 1. ipapi.co (free, no key, detailed)
  console.log('1. Testing ipapi.co...');
  try {
    const data = await httpGet('https://ipapi.co/json/');
    console.log('✓ Success!');
    console.log('  IP:', data.ip);
    console.log('  Country:', data.country_name);
    console.log('  Region:', data.region);
    console.log('  City:', data.city);
    console.log('  ISP:', data.org);
    console.log('  Timezone:', data.timezone);
    console.log('  Lat/Lon:', data.latitude, data.longitude);
  } catch (e) {
    console.log('✗ Failed:', e.message);
  }

  console.log('\n2. Testing ipapi.co/json (retry)...');
  try {
    await new Promise(resolve => setTimeout(resolve, 1000)); // Wait 1s
    const data = await httpGet('https://ipapi.co/json/');
    console.log('✓ Success!');
    console.log('  Full data:', JSON.stringify(data, null, 2));
  } catch (e) {
    console.log('✗ Failed:', e.message);
  }

  console.log('\n3. Testing ipinfo.io (detailed)...');
  try {
    const data = await httpGet('https://ipinfo.io/json');
    console.log('✓ Success!');
    console.log('  Full data:', JSON.stringify(data, null, 2));
  } catch (e) {
    console.log('✗ Failed:', e.message);
  }

  console.log('\n4. Testing ifconfig.me (simple IP only)...');
  try {
    const ip = await httpGet('https://ifconfig.me/ip');
    console.log('✓ Success!');
    console.log('  IP:', ip.trim());
  } catch (e) {
    console.log('✗ Failed:', e.message);
  }

  console.log('\n=== Test Complete ===');
}

testAPIs();
