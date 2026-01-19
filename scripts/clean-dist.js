/**
 * Clean dist directory - remove unnecessary files for production
 */
const fs = require('fs');
const path = require('path');

const distPath = path.join(__dirname, '../dist');

let removedCount = 0;
let failedCount = 0;

// Recursively find and remove files matching patterns
function cleanDirectory(dir) {
  if (!fs.existsSync(dir)) return;
  
  const files = fs.readdirSync(dir);
  
  files.forEach(file => {
    const fullPath = path.join(dir, file);
    const stats = fs.statSync(fullPath);
    
    if (stats.isDirectory()) {
      cleanDirectory(fullPath);
    } else {
      // Check if file should be removed
      const shouldRemove = 
        file.endsWith('.map') ||           // Source maps
        file.endsWith('.js.gz') ||         // Gzipped JS
        file.endsWith('.css.gz') ||        // Gzipped CSS
        file.endsWith('.html.gz');         // Gzipped HTML
      
      if (shouldRemove) {
        try {
          fs.unlinkSync(fullPath);
          const relativePath = path.relative(distPath, fullPath);
          console.log(`✓ Removed: ${relativePath}`);
          removedCount++;
        } catch (e) {
          console.error(`✗ Failed to remove ${file}:`, e.message);
          failedCount++;
        }
      }
    }
  });
}

console.log('=== Cleaning dist directory ===');
console.log('Removing: .map, .js.gz, .css.gz, .html.gz files\n');

cleanDirectory(distPath);

console.log(`\n=== Cleanup Complete ===`);
console.log(`Removed: ${removedCount} files`);
if (failedCount > 0) {
  console.log(`Failed: ${failedCount} files`);
}

// Show final size
try {
  const { execSync } = require('child_process');
  const size = execSync(`powershell -Command "(Get-ChildItem -Path '${distPath}' -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB"`, { encoding: 'utf8' });
  console.log(`\nFinal dist size: ${parseFloat(size).toFixed(2)} MB`);
} catch (e) {
  // Ignore if can't get size
}
