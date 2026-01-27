/**
 * Clean dist directory - remove unnecessary files for production
 */
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { dirname } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const distPath = path.join(__dirname, '../dist');

let removedCount = 0;
let failedCount = 0;
const removedFiles = [];

// Recursively find and remove files matching patterns
function cleanDirectory(dir) {
  if (!fs.existsSync(dir)) return;
  
  const files = fs.readdirSync(dir);
  
  files.forEach(file => {
    const fullPath = path.join(dir, file);
    const stats = fs.statSync(fullPath);
    
    if (stats.isDirectory()) {
      cleanDirectory(fullPath);
      // Try to remove empty directories
      try {
        const remainingFiles = fs.readdirSync(fullPath);
        if (remainingFiles.length === 0) {
          fs.rmdirSync(fullPath);
          console.log(`✓ Removed empty dir: ${path.relative(distPath, fullPath)}`);
        }
      } catch (e) {
        // Ignore if can't remove directory
      }
    } else {
      // Check if file should be removed
      const shouldRemove = 
        file.endsWith('.map') ||           // Source maps
        file.endsWith('.js.gz') ||         // Gzipped JS
        file.endsWith('.css.gz') ||        // Gzipped CSS
        file.endsWith('.html.gz') ||       // Gzipped HTML
        file.endsWith('.pdb') ||           // MSVC debug symbols
        file.endsWith('.iobj') ||          // MSVC incremental linking
        file.endsWith('.ipdb') ||          // MSVC incremental PDB
        file.endsWith('.exp') ||           // MSVC export file
        file.endsWith('.lib') ||           // MSVC static library
        file.endsWith('.ilk') ||           // MSVC incremental link
        file.startsWith('test-') ||        // Test files
        file.endsWith('.vcxproj') ||       // Visual Studio project
        file.endsWith('.vcxproj.filters') || // VS project filters
        file.endsWith('.sln') ||           // Visual Studio solution
        file.endsWith('.gypi') ||          // node-gyp config
        file === 'binding.gyp' ||          // node-gyp binding
        file === 'config.gypi';            // node-gyp config
      
      if (shouldRemove) {
        try {
          fs.unlinkSync(fullPath);
          const relativePath = path.relative(distPath, fullPath);
          console.log(`✓ Removed: ${relativePath}`);
          removedFiles.push(relativePath);
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
console.log('Removing: .map, .gz, .pdb, .iobj, .ipdb, .exp, .lib, test files, VS project files\n');

cleanDirectory(distPath);

console.log(`\n=== Cleanup Complete ===`);
console.log(`Removed: ${removedCount} files`);
if (failedCount > 0) {
  console.log(`Failed: ${failedCount} files`);
}

if (removedFiles.length > 0) {
  console.log('\nRemoved files:');
  removedFiles.forEach(f => console.log(`  - ${f}`));
}

// Show final size
try {
  const { execSync } = await import('child_process');
  const size = execSync(`powershell -Command "(Get-ChildItem -Path '${distPath}' -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB"`, { encoding: 'utf8' });
  console.log(`\nFinal dist size: ${parseFloat(size).toFixed(2)} MB`);
} catch (e) {
  // Ignore if can't get size
}
