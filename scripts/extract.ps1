param(
    [Parameter(Mandatory=$true)]
    [string]$zipPath,
    [Parameter(Mandatory=$true)]
    [string]$destination,
    [int]$delaySeconds = 3  # 可选参数，默认延时5秒
)

Add-Type -AssemblyName System.Windows.Forms

Write-Host "============================="s
Write-Host "PowerShell 脚本执行开始"
Write-Host "ZIP 文件路径: $zipPath"
Write-Host "解压目标路径: $destination"
Write-Host "============================="

# 创建窗口
$form = New-Object System.Windows.Forms.Form
$form.Text = "PaperTracker更新程序"
$form.Size = New-Object System.Drawing.Size(400, 200)
$form.StartPosition = "CenterScreen"

# 创建标签
$label = New-Object System.Windows.Forms.Label
$label.Location = New-Object System.Drawing.Point(50, 20)
$label.Size = New-Object System.Drawing.Size(300, 20)
$label.Text = "准备安装..."
$form.Controls.Add($label)

# 创建进度条
$progressBar = New-Object System.Windows.Forms.ProgressBar
$progressBar.Location = New-Object System.Drawing.Point(50, 50)
$progressBar.Size = New-Object System.Drawing.Size(300, 30)
$progressBar.Minimum = 0
$progressBar.Maximum = 100
$form.Controls.Add($progressBar)

# 显示窗口
$form.Show()

# 显示文件路径，方便调试
Write-Host "ZIP 文件路径: $zipPath"
Write-Host "安装目标路径: $destination"

# 延时
$label.Text = "等待 $delaySeconds 秒后开始安装..."
$form.Refresh()
Start-Sleep -Seconds $delaySeconds

# 确保 ZIP 文件存在
if (!(Test-Path $zipPath)) {
$label.Text = "错误: ZIP 文件 $zipPath 不存在！"
Write-Host "错误: ZIP 文件 $zipPath 不存在！"
Start-Sleep -Seconds 3
$form.Close()
exit 1
}

# 如果目标文件夹已存在，先删除后重建
if (Test-Path $destination) {
$label.Text = "目标文件夹已存在，正在删除..."
$form.Refresh()
Remove-Item -Path $destination -Recurse -Force
}

# 重新创建目标文件夹
$label.Text = "创建目标文件夹..."
$form.Refresh()
New-Item -ItemType Directory -Force -Path $destination | Out-Null

# 开始解压缩
$label.Text = "正在安装更新文件..."
$form.Refresh()

# 计算大约的进度更新次数（这里只是模拟，Expand-Archive 无法提供实际进度）
$progressBar.Value = 10
$form.Refresh()

Start-Sleep -Seconds 1  # 模拟进度

# **确保解压缩过程发生错误时能看到日志**
try {
Expand-Archive -Path $zipPath -DestinationPath $destination -Force -ErrorAction Stop
} catch {
Write-Host "更新安装失败: $_"
$label.Text = "更新失败，请检查日志或联系售后下载最新安装包"
Start-Sleep -Seconds 3
$form.Close()
exit 1
}

# 完成
$progressBar.Value = 100
$label.Text = "更新安装完成！"
$form.Refresh()
Write-Host "更新安装完成！"

Start-Sleep -Seconds 2  # 保持窗口短暂显示
$form.Close()
