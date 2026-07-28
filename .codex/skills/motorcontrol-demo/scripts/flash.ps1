[CmdletBinding()]
param(
    [string]$VectorIde = 'C:\VectorIDEv1.3.3',
    [string]$ElfPath
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\..'))
if ([string]::IsNullOrWhiteSpace($ElfPath)) {
    $ElfPath = Join-Path $repoRoot 'EclipseIDE\Debug\MotorControlDemo.elf'
}
$ElfPath = [IO.Path]::GetFullPath($ElfPath)

$openocd = Join-Path $VectorIde 'openocd\bin\openocd.exe'
$scripts = Join-Path $VectorIde 'openocd\scripts'
$config = Join-Path $scripts 'k1921\k1921vk01t\connect_stlink.cfg'

foreach ($required in @($openocd, $config, $ElfPath)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required file not found: $required"
    }
}

$existing = Get-Process openocd, openocd-x64 -ErrorAction SilentlyContinue
if ($existing) {
    $ids = ($existing.Id -join ', ')
    throw "OpenOCD is already running (PID: $ids). Reuse or stop that session before flashing."
}

$openocdElf = $ElfPath.Replace('\', '/')
& $openocd `
    -f $config `
    -s $scripts `
    -c 'reset halt' `
    -c "program $openocdElf verify reset exit"

if ($LASTEXITCODE -ne 0) {
    throw "OpenOCD flashing failed with exit code $LASTEXITCODE."
}

Write-Host 'Firmware programmed, verified, and reset successfully.'
