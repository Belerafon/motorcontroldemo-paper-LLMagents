[CmdletBinding()]
param(
    [string]$VectorIde = 'C:\VectorIDEv1.3.3',
    [ValidateSet('Debug')]
    [string]$Configuration = 'Debug',
    [switch]$Incremental
)

$ErrorActionPreference = 'Stop'
$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..\..\..'))
$projectDir = Join-Path $repoRoot 'EclipseIDE'
$eclipse = Join-Path $VectorIde 'Eclipse\eclipsec.exe'
$codeBenchBin = Join-Path $VectorIde 'CodeBench\bin'
$gnuBin = Join-Path $VectorIde 'GNUGCC\ARM Embedded GCC\8.2.1-1.7-20190524-0603\bin'
$settingsFile = Join-Path $projectDir '.settings\language.settings.xml'
$elf = Join-Path $projectDir "$Configuration\MotorControlDemo.elf"
$bin = Join-Path $projectDir "$Configuration\MotorControlDemo.bin"

foreach ($required in @(
    $eclipse,
    (Join-Path $codeBenchBin 'arm-none-eabi-gcc.exe'),
    (Join-Path $codeBenchBin 'cs-make.exe')
)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required VectorIDE file not found: $required"
    }
}
$savedPath = $env:PATH
$env:PATH = "$codeBenchBin;$gnuBin;$savedPath"
$workspace = Join-Path ([IO.Path]::GetTempPath()) ("vectoride-mcd-{0}-{1}" -f $PID, [guid]::NewGuid().ToString('N'))
$settingsBackup = $null

try {
    New-Item -ItemType Directory -Path $workspace | Out-Null

    if (Test-Path -LiteralPath $settingsFile -PathType Leaf) {
        $settingsBackup = Join-Path ([IO.Path]::GetTempPath()) ("language-settings-{0}.xml" -f [guid]::NewGuid().ToString('N'))
        Copy-Item -LiteralPath $settingsFile -Destination $settingsBackup
    }

    $buildOption = if ($Incremental) { '-build' } else { '-cleanBuild' }
    & $eclipse `
        -nosplash `
        -data $workspace `
        -application org.eclipse.cdt.managedbuilder.core.headlessbuild `
        -import $projectDir `
        $buildOption "MotorControlDemo/$Configuration"

    if ($LASTEXITCODE -ne 0) {
        throw "VectorIDE headless build failed with exit code $LASTEXITCODE."
    }
    if (-not (Test-Path -LiteralPath $elf -PathType Leaf) -or
        -not (Test-Path -LiteralPath $bin -PathType Leaf)) {
        throw 'Build finished without the expected ELF and BIN artifacts.'
    }

    & (Join-Path $codeBenchBin 'arm-none-eabi-size.exe') --format=berkeley $elf
    if ($LASTEXITCODE -ne 0) {
        throw 'arm-none-eabi-size failed.'
    }

    Write-Host "ELF: $elf"
    Write-Host "BIN: $bin"
}
finally {
    $env:PATH = $savedPath

    if ($null -ne $settingsBackup -and (Test-Path -LiteralPath $settingsBackup)) {
        Copy-Item -LiteralPath $settingsBackup -Destination $settingsFile -Force
        Remove-Item -LiteralPath $settingsBackup -Force
    }

    $resolvedTemp = [IO.Path]::GetFullPath([IO.Path]::GetTempPath())
    $resolvedWorkspace = [IO.Path]::GetFullPath($workspace)
    if ($resolvedWorkspace.StartsWith($resolvedTemp, [StringComparison]::OrdinalIgnoreCase) -and
        (Test-Path -LiteralPath $resolvedWorkspace)) {
        Remove-Item -LiteralPath $resolvedWorkspace -Recurse -Force
    }
}
