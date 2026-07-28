[CmdletBinding()]
param(
    [string]$VectorIde = 'C:\VectorIDEv1.3.3',
    [string]$ElfPath,
    [int]$IntervalMs = 500,
    [switch]$KeepServer
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
$gdb = Join-Path $VectorIde 'CodeBench\bin\arm-none-eabi-gdb.exe'

foreach ($required in @($openocd, $config, $gdb, $ElfPath)) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required file not found: $required"
    }
}

function Test-TelnetPort {
    try {
        $client = [Net.Sockets.TcpClient]::new()
        $task = $client.ConnectAsync('127.0.0.1', 4444)
        if (-not $task.Wait(200)) {
            $client.Dispose()
            return $false
        }
        $client.Dispose()
        return $true
    }
    catch {
        return $false
    }
}

function Invoke-OcdTelnet([string]$Command) {
    $client = [Net.Sockets.TcpClient]::new('127.0.0.1', 4444)
    try {
        $stream = $client.GetStream()
        Start-Sleep -Milliseconds 80
        while ($stream.DataAvailable) {
            $junk = New-Object byte[] 1024
            [void]$stream.Read($junk, 0, $junk.Length)
        }

        $bytes = [Text.Encoding]::ASCII.GetBytes($Command + "`n")
        $stream.Write($bytes, 0, $bytes.Length)
        $stream.Flush()
        Start-Sleep -Milliseconds 180

        $buffer = New-Object byte[] 16384
        $result = ''
        while ($stream.DataAvailable) {
            $count = $stream.Read($buffer, 0, $buffer.Length)
            $result += [Text.Encoding]::ASCII.GetString($buffer, 0, $count)
        }
        return $result
    }
    finally {
        $client.Dispose()
    }
}

function Get-ExpressionAddress([string]$Expression) {
    $output = & $gdb -q -batch $ElfPath -ex "p/x &$Expression" 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "GDB could not resolve '$Expression': $($output -join ' ')"
    }
    $match = [regex]::Match(($output -join "`n"), '0x[0-9a-fA-F]+')
    if (-not $match.Success) {
        throw "No address found for '$Expression'."
    }
    return [Convert]::ToUInt32($match.Value.Substring(2), 16)
}

$items = @(
    [pscustomobject]@{ Name = 'msCounter'; Expression = 'msCounter'; Width = 16 },
    [pscustomobject]@{ Name = 'FastCounter'; Expression = 'FastCounter'; Width = 16 },
    [pscustomobject]@{ Name = 'TIsr1'; Expression = 'TIsr1'; Width = 32 },
    [pscustomobject]@{ Name = 'TIsr10'; Expression = 'TIsr10'; Width = 32 },
    [pscustomobject]@{ Name = 'sys_state'; Expression = 'sm_sys.state'; Width = 16 },
    [pscustomobject]@{ Name = 'prot_state'; Expression = 'sm_prot.state'; Width = 16 },
    [pscustomobject]@{ Name = 'fault1'; Expression = 'sm_prot.bit_fault1'; Width = 16 },
    [pscustomobject]@{ Name = 'fault2'; Expression = 'sm_prot.bit_fault2'; Width = 16 },
    [pscustomobject]@{ Name = 'ctrl_state'; Expression = 'sm_ctrl.state'; Width = 16 },
    [pscustomobject]@{ Name = 'run_mode'; Expression = 'sm_ctrl.run_mode'; Width = 16 },
    [pscustomobject]@{ Name = 'speed_ref'; Expression = 'refs.speed_ref'; Width = 32 },
    [pscustomobject]@{ Name = 'Iq_ref'; Expression = 'refs.Iq_ref'; Width = 32 },
    [pscustomobject]@{ Name = 'i_flux_ref'; Expression = 'refs.i_flux_ref'; Width = 32 }
)

foreach ($item in $items) {
    $item | Add-Member -NotePropertyName Address -NotePropertyValue (Get-ExpressionAddress $item.Expression)
}

function Read-Snapshot {
    $commands = foreach ($item in $items) {
        $verb = if ($item.Width -eq 16) { 'mdh' } else { 'mdw' }
        '{0} 0x{1:x8} 1' -f $verb, $item.Address
    }
    $response = Invoke-OcdTelnet ($commands -join '; ')
    $matches = [regex]::Matches($response, '0x[0-9a-fA-F]+:\s+([0-9a-fA-F]+)')
    if ($matches.Count -ne $items.Count) {
        throw "Unexpected OpenOCD memory response: $response"
    }

    $snapshot = @{}
    for ($i = 0; $i -lt $items.Count; $i++) {
        $snapshot[$items[$i].Name] = [Convert]::ToUInt32($matches[$i].Groups[1].Value, 16)
    }
    return $snapshot
}

$startedServer = $false
$server = $null
$outLog = Join-Path ([IO.Path]::GetTempPath()) 'motorcontrol-demo-openocd.out.log'
$errLog = Join-Path ([IO.Path]::GetTempPath()) 'motorcontrol-demo-openocd.err.log'

try {
    if (-not (Test-TelnetPort)) {
        $arguments = @(
            '-f', ('"{0}"' -f $config),
            '-s', ('"{0}"' -f $scripts),
            '-c', 'reset',
            '-c', 'halt',
            '-c', 'resume'
        )
        $server = Start-Process `
            -FilePath $openocd `
            -ArgumentList $arguments `
            -WorkingDirectory $repoRoot `
            -WindowStyle Hidden `
            -RedirectStandardOutput $outLog `
            -RedirectStandardError $errLog `
            -PassThru
        $startedServer = $true

        $ready = $false
        for ($i = 0; $i -lt 30; $i++) {
            Start-Sleep -Milliseconds 100
            if ($server.HasExited) {
                $details = Get-Content -LiteralPath $errLog -ErrorAction SilentlyContinue
                throw "OpenOCD exited before accepting connections: $($details -join ' ')"
            }
            if (Test-TelnetPort) {
                $ready = $true
                break
            }
        }
        if (-not $ready) {
            throw 'OpenOCD did not open the telnet port.'
        }
    }

    $first = Read-Snapshot
    $timer = [Diagnostics.Stopwatch]::StartNew()
    Start-Sleep -Milliseconds $IntervalMs
    $second = Read-Snapshot
    $timer.Stop()

    $msDelta = ($second.msCounter - $first.msCounter + 65536) % 65536
    $fastDelta = ($second.FastCounter - $first.FastCounter + 65536) % 65536

    [pscustomobject]@{
        SampleSeconds = [math]::Round($timer.Elapsed.TotalSeconds, 3)
        MsCounterA = $first.msCounter
        MsCounterB = $second.msCounter
        MsDelta = $msDelta
        MsRateHz = [math]::Round($msDelta / $timer.Elapsed.TotalSeconds, 1)
        FastCounterA = $first.FastCounter
        FastCounterB = $second.FastCounter
        FastDelta = $fastDelta
        FastRateHz = [math]::Round($fastDelta / $timer.Elapsed.TotalSeconds, 1)
        TIsr1 = $second.TIsr1
        TIsr10 = $second.TIsr10
        SysState = $second.sys_state
        ProtectionState = $second.prot_state
        Fault1 = ('0x{0:X4}' -f $second.fault1)
        Fault2 = ('0x{0:X4}' -f $second.fault2)
        ControlState = $second.ctrl_state
        RunMode = $second.run_mode
        SpeedRef = ('0x{0:X8}' -f $second.speed_ref)
        IqRef = ('0x{0:X8}' -f $second.Iq_ref)
        FluxRef = ('0x{0:X8}' -f $second.i_flux_ref)
    } | Format-List
}
finally {
    if ($startedServer -and -not $KeepServer) {
        if (Test-TelnetPort) {
            [void](Invoke-OcdTelnet 'shutdown')
            if ($null -ne $server) {
                [void]$server.WaitForExit(3000)
            }
        }
        elseif ($null -ne $server -and -not $server.HasExited) {
            Stop-Process -Id $server.Id -Force
        }
    }
    elseif ($startedServer -and $KeepServer -and $null -ne $server) {
        Write-Host "OpenOCD remains running with PID $($server.Id)."
    }
}
