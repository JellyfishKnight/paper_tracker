param(
    [Parameter(Mandatory=$true)]
    [string]$zipPath,
    [Parameter(Mandatory=$true)]
    [string]$destination,
    [int]$delaySeconds = 5  # 可选参数，默认延时5秒
)

# 延时指定的秒数
Start-Sleep -Seconds $delaySeconds

# 如果目标文件夹已存在，先删除后重建（或根据需要直接覆盖）
if (Test-Path $destination) {
    Remove-Item -Path $destination -Recurse -Force
}

# 创建目标文件夹
New-Item -ItemType Directory -Force -Path $destination | Out-Null

# 使用 Expand-Archive 解压文件，-Force 表示覆盖同名文件
Expand-Archive -Path $zipPath -DestinationPath $destination -Force
