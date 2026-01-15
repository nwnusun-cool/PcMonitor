import { ref, nextTick } from 'vue'

// 最大数据点数
export const maxDataPoints = 60

// 设置 Canvas 高清显示
export function setupCanvas(canvas) {
  if (!canvas) return
  const dpr = window.devicePixelRatio || 2
  const rect = canvas.getBoundingClientRect()
  canvas.width = rect.width * dpr
  canvas.height = rect.height * dpr
  canvas.style.width = rect.width + 'px'
  canvas.style.height = rect.height + 'px'
  const ctx = canvas.getContext('2d')
  ctx.scale(dpr, dpr)
}

// 格式化网络速度值
export function formatIOValue(bytes) {
  if (!bytes || isNaN(bytes) || bytes <= 0) return '0'
  if (bytes < 1024) return Math.round(bytes) + 'B'
  const k = 1024
  const sizes = ['B', 'KB', 'MB', 'GB']
  const i = Math.min(Math.floor(Math.log(bytes) / Math.log(k)), sizes.length - 1)
  return parseFloat((bytes / Math.pow(k, i)).toFixed(0)) + sizes[i]
}

// 绘制单线图表（百分比）
export function drawChart(canvas, data, color) {
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  const rect = canvas.getBoundingClientRect()
  const width = rect.width
  const height = rect.height
  
  const padding = { top: 10, right: 10, bottom: 10, left: 50 }
  const chartWidth = width - padding.left - padding.right
  const chartHeight = height - padding.top - padding.bottom
  
  ctx.clearRect(0, 0, width, height)
  
  if (data.length < 1) return
  
  // Y轴网格线和标签
  ctx.strokeStyle = '#e0e0e0'
  ctx.lineWidth = 1
  ctx.fillStyle = '#999'
  ctx.font = '11px sans-serif'
  ctx.textAlign = 'right'
  
  for (let i = 0; i <= 4; i++) {
    const y = padding.top + (chartHeight / 4) * i
    const value = 100 - (i * 25)
    ctx.beginPath()
    ctx.moveTo(padding.left, y)
    ctx.lineTo(padding.left + chartWidth, y)
    ctx.stroke()
    ctx.fillText(value + '%', padding.left - 8, y + 4)
  }
  
  const dataLength = data.length
  const step = chartWidth / (maxDataPoints - 1)
  
  // 绘制折线
  ctx.strokeStyle = color
  ctx.lineWidth = 2
  ctx.beginPath()
  
  let firstPoint = true
  for (let i = 0; i < dataLength; i++) {
    const value = data[i]
    const x = padding.left + chartWidth - (dataLength - 1 - i) * step
    const y = padding.top + chartHeight - (value / 100) * chartHeight
    
    if (firstPoint) {
      ctx.moveTo(x, y)
      firstPoint = false
    } else {
      ctx.lineTo(x, y)
    }
  }
  ctx.stroke()
  
  // 填充区域
  if (dataLength > 0) {
    const lastX = padding.left + chartWidth
    const firstX = padding.left + chartWidth - (dataLength - 1) * step
    ctx.lineTo(lastX, padding.top + chartHeight)
    ctx.lineTo(firstX, padding.top + chartHeight)
    ctx.closePath()
    ctx.fillStyle = color + '15'
    ctx.fill()
  }
  
  // 坐标轴
  ctx.strokeStyle = '#666'
  ctx.lineWidth = 2
  ctx.beginPath()
  ctx.moveTo(padding.left, padding.top)
  ctx.lineTo(padding.left, padding.top + chartHeight)
  ctx.lineTo(padding.left + chartWidth, padding.top + chartHeight)
  ctx.stroke()
}

// 绘制多线图表（百分比）
export function drawMultiLineChart(canvas, lines) {
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  const rect = canvas.getBoundingClientRect()
  const width = rect.width
  const height = rect.height
  
  const padding = { top: 10, right: 10, bottom: 10, left: 50 }
  const chartWidth = width - padding.left - padding.right
  const chartHeight = height - padding.top - padding.bottom
  
  ctx.clearRect(0, 0, width, height)
  
  if (lines[0].data.length < 1) return
  
  // Y轴网格线和标签
  ctx.strokeStyle = '#e0e0e0'
  ctx.lineWidth = 1
  ctx.fillStyle = '#999'
  ctx.font = '11px sans-serif'
  ctx.textAlign = 'right'
  
  for (let i = 0; i <= 4; i++) {
    const y = padding.top + (chartHeight / 4) * i
    const value = 100 - (i * 25)
    ctx.beginPath()
    ctx.moveTo(padding.left, y)
    ctx.lineTo(padding.left + chartWidth, y)
    ctx.stroke()
    ctx.fillText(value + '%', padding.left - 8, y + 4)
  }
  
  const dataLength = lines[0].data.length
  const step = chartWidth / (maxDataPoints - 1)
  
  // 绘制每条线
  lines.forEach(line => {
    const { data, color } = line
    ctx.strokeStyle = color
    ctx.lineWidth = 2
    ctx.beginPath()
    
    let firstPoint = true
    for (let i = 0; i < dataLength; i++) {
      const value = data[i]
      const x = padding.left + chartWidth - (dataLength - 1 - i) * step
      const y = padding.top + chartHeight - (value / 100) * chartHeight
      
      if (firstPoint) {
        ctx.moveTo(x, y)
        firstPoint = false
      } else {
        ctx.lineTo(x, y)
      }
    }
    ctx.stroke()
  })
  
  // 坐标轴
  ctx.strokeStyle = '#666'
  ctx.lineWidth = 2
  ctx.beginPath()
  ctx.moveTo(padding.left, padding.top)
  ctx.lineTo(padding.left, padding.top + chartHeight)
  ctx.lineTo(padding.left + chartWidth, padding.top + chartHeight)
  ctx.stroke()
}

// 绘制网络流量图表（动态Y轴）
export function drawNetworkChart(canvas, lines) {
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  const rect = canvas.getBoundingClientRect()
  const width = rect.width
  const height = rect.height
  
  if (width === 0 || height === 0) return
  
  const padding = { top: 10, right: 10, bottom: 10, left: 55 }
  const chartWidth = width - padding.left - padding.right
  const chartHeight = height - padding.top - padding.bottom
  
  ctx.clearRect(0, 0, width, height)
  
  if (!lines[0].data || lines[0].data.length < 1) return
  
  // 找出最大值用于Y轴缩放
  const allValues = [...lines[0].data, ...lines[1].data].filter(v => !isNaN(v) && v !== null)
  const maxValue = Math.max(...allValues, 0)
  const yAxisMax = maxValue > 0 ? maxValue * 1.2 : 1024
  
  // Y轴网格线和标签
  ctx.strokeStyle = '#e0e0e0'
  ctx.lineWidth = 1
  ctx.fillStyle = '#999'
  ctx.font = '10px sans-serif'
  ctx.textAlign = 'right'
  
  for (let i = 0; i <= 4; i++) {
    const y = padding.top + (chartHeight / 4) * i
    const value = yAxisMax - (yAxisMax / 4) * i
    ctx.beginPath()
    ctx.moveTo(padding.left, y)
    ctx.lineTo(padding.left + chartWidth, y)
    ctx.stroke()
    ctx.fillText(formatIOValue(value) + '/s', padding.left - 5, y + 4)
  }
  
  const dataLength = lines[0].data.length
  const step = chartWidth / (maxDataPoints - 1)
  
  // 绘制每条线
  lines.forEach(line => {
    const { data, color } = line
    ctx.strokeStyle = color
    ctx.lineWidth = 2
    ctx.beginPath()
    
    let firstPoint = true
    for (let i = 0; i < dataLength; i++) {
      const value = data[i] || 0
      const x = padding.left + chartWidth - (dataLength - 1 - i) * step
      const normalizedValue = yAxisMax > 0 ? value / yAxisMax : 0
      const y = padding.top + chartHeight - normalizedValue * chartHeight
      
      if (firstPoint) {
        ctx.moveTo(x, y)
        firstPoint = false
      } else {
        ctx.lineTo(x, y)
      }
    }
    ctx.stroke()
  })
  
  // 坐标轴
  ctx.strokeStyle = '#666'
  ctx.lineWidth = 2
  ctx.beginPath()
  ctx.moveTo(padding.left, padding.top)
  ctx.lineTo(padding.left, padding.top + chartHeight)
  ctx.lineTo(padding.left + chartWidth, padding.top + chartHeight)
  ctx.stroke()
}
