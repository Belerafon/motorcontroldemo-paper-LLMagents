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
$codeBenchBin = Join-Path $VectorIde 'CodeBench\bin'
$gnuBin = Join-Path $VectorIde 'GNUGCC\ARM Embedded GCC\8.2.1-1.7-20190524-0603\bin'
$make = Join-Path $codeBenchBin 'cs-make.exe'
$elf = Join-Path $projectDir "$Configuration\MotorControlDemo.elf"
$bin = Join-Path $projectDir "$Configuration\MotorControlDemo.bin"

foreach ($required in @(
    $make,
    (Join-Path $codeBenchBin 'arm-none-eabi-gcc.exe'),
    (Join-Path $codeBenchBin 'arm-none-eabi-objcopy.exe'),
    (Join-Path $codeBenchBin 'arm-none-eabi-size.exe'),
    (Join-Path $repoRoot 'Makefile')
)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required VectorIDE file not found: $required"
    }
}
$savedPath = $env:PATH
$env:PATH = "$codeBenchBin;$gnuBin;$savedPath"

try {
    if (-not $Incremental) {
        & $make -C $repoRoot clean
        if ($LASTEXITCODE -ne 0) {
            throw "Make clean failed with exit code $LASTEXITCODE."
        }
    }

    & $make -C $repoRoot -j8 all
    if ($LASTEXITCODE -ne 0) {
        throw "Make build failed with exit code $LASTEXITCODE."
    }
    if (-not (Test-Path -LiteralPath $elf -PathType Leaf) -or
        -not (Test-Path -LiteralPath $bin -PathType Leaf)) {
        throw 'Build finished without the expected ELF and BIN artifacts.'
    }

    Write-Host "ELF: $elf"
    Write-Host "BIN: $bin"
}
finally {
    $env:PATH = $savedPath
}
