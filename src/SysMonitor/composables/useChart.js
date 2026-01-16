// Chart utilities for system monitor

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
  return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + sizes[i]
}

// 获取图表配置
export function getChartConfig(canvas, isNetwork = false) {
  const rect = canvas.getBoundingClientRect()
  const padding = isNetwork ? { top: 10, right: 10, bottom: 10, left: 55 } : { top: 10, right: 10, bottom: 10, left: 50 }
  return {
    padding,
    width: rect.width,
    height: rect.height,
    chartWidth: rect.width - padding.left - padding.right,
    chartHeight: rect.height - padding.top - padding.bottom,
    step: (rect.width - padding.left - padding.right) / (maxDataPoints - 1)
  }
}

// 根据鼠标位置获取数据点索引
export function getDataIndexAtPosition(canvas, mouseX, dataLength, isNetwork = false) {
  const config = getChartConfig(canvas, isNetwork)
  const relativeX = mouseX - config.padding.left
  if (relativeX < 0 || relativeX > config.chartWidth) return -1
  const offsetFromRight = config.chartWidth - relativeX
  const index = dataLength - 1 - Math.round(offsetFromRight / config.step)
  return Math.max(0, Math.min(dataLength - 1, index))
}


// 绘制单线图表（百分比）
export function drawChart(canvas, data, color) {
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  const config = getChartConfig(canvas)
  const { padding, width, height, chartWidth, chartHeight, step } = config
  
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
    ctx.beginPath()
    ctx.moveTo(padding.left, y)
    ctx.lineTo(padding.left + chartWidth, y)
    ctx.stroke()
    ctx.fillText((100 - i * 25) + '%', padding.left - 8, y + 4)
  }
  
  // 绘制折线
  ctx.strokeStyle = color
  ctx.lineWidth = 2
  ctx.beginPath()
  
  for (let i = 0; i < data.length; i++) {
    const x = padding.left + chartWidth - (data.length - 1 - i) * step
    const y = padding.top + chartHeight - (data[i] / 100) * chartHeight
    i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y)
  }
  ctx.stroke()
  
  // 填充区域
  if (data.length > 0) {
    ctx.lineTo(padding.left + chartWidth, padding.top + chartHeight)
    ctx.lineTo(padding.left + chartWidth - (data.length - 1) * step, padding.top + chartHeight)
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

export const drawLineChart = drawChart

// 绘制带高亮点的图表
export function drawChartWithHighlight(canvas, data, color, highlightIndex = -1) {
  drawChart(canvas, data, color)
  if (highlightIndex < 0 || highlightIndex >= data.length) return
  
  const ctx = canvas.getContext('2d')
  const config = getChartConfig(canvas)
  const { padding, chartWidth, chartHeight, step } = config
  const x = padding.left + chartWidth - (data.length - 1 - highlightIndex) * step
  const y = padding.top + chartHeight - (data[highlightIndex] / 100) * chartHeight
  
  // 垂直参考线
  ctx.strokeStyle = '#ccc'
  ctx.lineWidth = 1
  ctx.setLineDash([4, 4])
  ctx.beginPath()
  ctx.moveTo(x, padding.top)
  ctx.lineTo(x, padding.top + chartHeight)
  ctx.stroke()
  ctx.setLineDash([])
  
  // 高亮点
  ctx.fillStyle = color
  ctx.beginPath()
  ctx.arc(x, y, 5, 0, Math.PI * 2)
  ctx.fill()
  ctx.strokeStyle = '#fff'
  ctx.lineWidth = 2
  ctx.stroke()
}


// 绘制网络流量图表（动态Y轴）
export function drawNetworkChart(canvas, lines) {
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  const config = getChartConfig(canvas, true)
  const { padding, width, height, chartWidth, chartHeight, step } = config
  
  if (width === 0 || height === 0) return
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
    ctx.beginPath()
    ctx.moveTo(padding.left, y)
    ctx.lineTo(padding.left + chartWidth, y)
    ctx.stroke()
    ctx.fillText(formatIOValue(yAxisMax - (yAxisMax / 4) * i) + '/s', padding.left - 5, y + 4)
  }
  
  const dataLength = lines[0].data.length
  
  // 绘制每条线
  lines.forEach(line => {
    ctx.strokeStyle = line.color
    ctx.lineWidth = 2
    ctx.beginPath()
    for (let i = 0; i < dataLength; i++) {
      const value = line.data[i] || 0
      const x = padding.left + chartWidth - (dataLength - 1 - i) * step
      const y = padding.top + chartHeight - (yAxisMax > 0 ? value / yAxisMax : 0) * chartHeight
      i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y)
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

// 绘制带高亮的网络图表
export function drawNetworkChartWithHighlight(canvas, lines, highlightIndex = -1) {
  drawNetworkChart(canvas, lines)
  if (highlightIndex < 0 || !lines[0].data || highlightIndex >= lines[0].data.length) return
  
  const ctx = canvas.getContext('2d')
  const config = getChartConfig(canvas, true)
  const { padding, chartWidth, chartHeight, step } = config
  const dataLength = lines[0].data.length
  
  const allValues = [...lines[0].data, ...lines[1].data].filter(v => !isNaN(v) && v !== null)
  const maxValue = Math.max(...allValues, 0)
  const yAxisMax = maxValue > 0 ? maxValue * 1.2 : 1024
  
  const x = padding.left + chartWidth - (dataLength - 1 - highlightIndex) * step
  
  // 垂直参考线
  ctx.strokeStyle = '#ccc'
  ctx.lineWidth = 1
  ctx.setLineDash([4, 4])
  ctx.beginPath()
  ctx.moveTo(x, padding.top)
  ctx.lineTo(x, padding.top + chartHeight)
  ctx.stroke()
  ctx.setLineDash([])
  
  // 高亮点
  lines.forEach(line => {
    const value = line.data[highlightIndex] || 0
    const y = padding.top + chartHeight - (yAxisMax > 0 ? value / yAxisMax : 0) * chartHeight
    ctx.fillStyle = line.color
    ctx.beginPath()
    ctx.arc(x, y, 5, 0, Math.PI * 2)
    ctx.fill()
    ctx.strokeStyle = '#fff'
    ctx.lineWidth = 2
    ctx.stroke()
  })
}

// 绘制多线图表（百分比）
export function drawMultiLineChart(canvas, lines) {
  if (!canvas) return
  const ctx = canvas.getContext('2d')
  const config = getChartConfig(canvas)
  const { padding, width, height, chartWidth, chartHeight, step } = config
  
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
    ctx.beginPath()
    ctx.moveTo(padding.left, y)
    ctx.lineTo(padding.left + chartWidth, y)
    ctx.stroke()
    ctx.fillText((100 - i * 25) + '%', padding.left - 8, y + 4)
  }
  
  const dataLength = lines[0].data.length
  
  lines.forEach(line => {
    ctx.strokeStyle = line.color
    ctx.lineWidth = 2
    ctx.beginPath()
    for (let i = 0; i < dataLength; i++) {
      const x = padding.left + chartWidth - (dataLength - 1 - i) * step
      const y = padding.top + chartHeight - (line.data[i] / 100) * chartHeight
      i === 0 ? ctx.moveTo(x, y) : ctx.lineTo(x, y)
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
