[CmdletBinding()]
param(
    [string]$OutputDirectory
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $repoRoot 'build\native'
}
$output = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $output -Force | Out-Null

$vswhere = Join-Path ${env:ProgramFiles(x86)} `
    'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
    throw 'vswhere.exe was not found. Install Visual Studio Build Tools with Desktop development with C++.'
}

$visualStudio = & $vswhere -all -products * -property installationPath |
    Where-Object {
        Test-Path -LiteralPath (Join-Path $_ 'VC\Auxiliary\Build\vcvars64.bat')
    } |
    Select-Object -First 1
if (-not $visualStudio) {
    throw 'A Visual Studio installation with the x64 C++ build tools was not found.'
}

$developerEnvironment = Join-Path $visualStudio 'VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path -LiteralPath $developerEnvironment -PathType Leaf)) {
    throw "Visual Studio developer environment was not found: $developerEnvironment"
}

$proxySource = Join-Path $repoRoot 'src\xinput-proxy\XInput-Proxy.cpp'
$proxyHeaderDirectory = Join-Path $repoRoot 'src\xinput-proxy'
$proxyExports = Join-Path $repoRoot 'src\xinput-proxy\XInput-Proxy-EXPORTS.def'
$launcherSource = Join-Path $repoRoot 'src\seamless-launcher\Seamless-Mode-Launcher.cpp'
$launcherHeaderDirectory = Join-Path $repoRoot 'src\seamless-launcher'
$kernelExports = Join-Path $repoRoot 'src\win32-imports\kernel32-minimal.def'
$userExports = Join-Path $repoRoot 'src\win32-imports\user32-minimal.def'

$commands = @(
    ('call "{0}" >nul' -f $developerEnvironment),
    ('cd /d "{0}"' -f $output),
    ('lib.exe /nologo /machine:x64 /def:"{0}" /out:kernel32-minimal.lib' -f $kernelExports),
    ('lib.exe /nologo /machine:x64 /def:"{0}" /out:user32-minimal.lib' -f $userExports),
    ('cl.exe /nologo /c /O2 /GS- /GR- /EHsc- /Zl /std:c++17 /I"{0}" /Fo:XInput-Proxy.obj "{1}"' -f $proxyHeaderDirectory, $proxySource),
    ('link.exe /nologo /DLL /OUT:xinput1_3.dll /DEF:"{0}" /ENTRY:DllMain /SUBSYSTEM:WINDOWS /NODEFAULTLIB /OPT:REF /OPT:ICF XInput-Proxy.obj kernel32-minimal.lib' -f $proxyExports),
    ('cl.exe /nologo /c /O2 /GS- /GR- /EHsc- /Zl /std:c++17 /I"{0}" /Fo:Seamless-Mode-Launcher.obj "{1}"' -f $launcherHeaderDirectory, $launcherSource),
    'link.exe /nologo /OUT:"DS3 Seamless + Keyboard UI.exe" /ENTRY:LauncherEntry /SUBSYSTEM:WINDOWS /NODEFAULTLIB /OPT:REF /OPT:ICF Seamless-Mode-Launcher.obj kernel32-minimal.lib user32-minimal.lib'
)

$commandLine = $commands -join ' && '
Write-Verbose $commandLine
& $env:ComSpec /d /s /c $commandLine
if ($LASTEXITCODE -ne 0) {
    throw "Native build failed with exit code $LASTEXITCODE."
}

$proxy = Join-Path $output 'xinput1_3.dll'
$launcher = Join-Path $output 'DS3 Seamless + Keyboard UI.exe'
if ((Get-Item -LiteralPath $proxy).Length -ne 17920) {
    throw 'Unexpected xinput1_3.dll size; verify the compiler version and flags.'
}
if ((Get-Item -LiteralPath $launcher).Length -ne 6144) {
    throw 'Unexpected launcher size; verify the compiler version and flags.'
}

$toolsetRoot = Join-Path $visualStudio 'VC\Tools\MSVC'
$toolset = Get-ChildItem -LiteralPath $toolsetRoot -Directory |
    Sort-Object { [version]$_.Name } -Descending |
    Select-Object -First 1
$dumpbin = if ($toolset) {
    Join-Path $toolset.FullName 'bin\Hostx64\x64\dumpbin.exe'
}

if ($dumpbin -and (Test-Path -LiteralPath $dumpbin -PathType Leaf)) {
    $proxyImports = (& $dumpbin /imports $proxy) -join "`n"
    if ($proxyImports -notmatch 'KERNEL32\.dll' -or
        $proxyImports -match 'USER32\.dll|VCRUNTIME|MSVCP|ucrtbase') {
        throw 'The proxy import-module check failed.'
    }
    $launcherImports = (& $dumpbin /imports $launcher) -join "`n"
    if ($launcherImports -notmatch 'KERNEL32\.dll' -or
        $launcherImports -notmatch 'USER32\.dll' -or
        $launcherImports -match 'VCRUNTIME|MSVCP|ucrtbase') {
        throw 'The launcher import-module check failed.'
    }
    $proxyExportsText = (& $dumpbin /exports $proxy) -join "`n"
    foreach ($expected in @(
        'XInputEnable',
        'XInputGetBatteryInformation',
        'XInputGetCapabilities',
        'XInputGetDSoundAudioDeviceGuids',
        'XInputGetKeystroke',
        'XInputGetState',
        'XInputSetState')) {
        if ($proxyExportsText -notmatch [regex]::Escape($expected)) {
            throw "Missing expected proxy export: $expected"
        }
    }
}

Write-Host ''
Write-Host 'Native build completed:'
foreach ($file in @($proxy, $launcher)) {
    $item = Get-Item -LiteralPath $file
    $hash = Get-FileHash -LiteralPath $file -Algorithm SHA256
    Write-Host ("{0} bytes  {1}  {2}" -f $item.Length, $hash.Hash, $item.Name)
}
