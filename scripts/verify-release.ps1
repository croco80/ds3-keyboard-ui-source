[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ReleaseDirectory
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$root = [IO.Path]::GetFullPath($ReleaseDirectory)
if (-not (Test-Path -LiteralPath $root -PathType Container)) {
    throw "Release directory not found: $root"
}

$expected = [ordered]@{
    'xinput1_3.dll' = '0FAB51021393699D573DB40DAEF2DDCBC92CBAA546497B7820593245C0B3E8E1'
    'DS3 Seamless + Keyboard UI.exe' = 'DD85D43C9ED4EA927AD62A64C43396854D9787135F0F400809764C19C2D10695'
    'DS3 Keyboard Icons\licenses\XInput-Proxy-SOURCE.cpp' = 'EEF3F75603BE9C5380B694BCC650053A4DC7B04DA9EFBB20E90B05E49D0F6E4A'
    'DS3 Keyboard Icons\licenses\Seamless-Mode-Launcher-SOURCE.cpp' = 'DAF5C0B1810D20FC7FEFC6D6BA235DFD7C28FC3531BC2F02F51165A8A2CB35FD'
    'DS3 Keyboard Icons\licenses\Win32-XInput-MINIMAL.h' = '85DB9116630DD119D7B59EA04F0984F63E018A1E87EED323B7FB8B3749F9A657'
    'DS3 Keyboard Icons\licenses\XInput-Proxy-EXPORTS.def' = 'F471937C8D114478F021D4B2BC36374696349170244A5F25AD11CEA8AC965333'
}

$failed = $false
foreach ($relative in $expected.Keys) {
    $path = Join-Path $root $relative
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        Write-Error "MISSING: $relative"
        $failed = $true
        continue
    }
    $actual = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    if ($actual -ne $expected[$relative]) {
        Write-Error "HASH MISMATCH: $relative`nExpected: $($expected[$relative])`nActual:   $actual"
        $failed = $true
    }
    else {
        Write-Host "OK  $relative"
    }
}

$thirdParty = @(
    'DS3 Keyboard Icons\modengine2\bin\modengine2.dll',
    'DS3 Keyboard Icons\modengine2\bin\lua.dll'
)
foreach ($relative in $thirdParty) {
    $path = Join-Path $root $relative
    if (Test-Path -LiteralPath $path -PathType Leaf) {
        $hash = Get-FileHash -LiteralPath $path -Algorithm SHA256
        Write-Host "INFO  $relative  $($hash.Hash)"
    }
}

if ($failed) {
    throw 'Release verification failed.'
}

Write-Host 'Release verification passed.'

