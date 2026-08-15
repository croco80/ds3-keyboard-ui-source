[CmdletBinding()]
param(
    [string]$Python = 'py',
    [string]$OutputDirectory
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $repoRoot 'build\artwork'
}
$output = [IO.Path]::GetFullPath($OutputDirectory)
New-Item -ItemType Directory -Path $output -Force | Out-Null

function Invoke-Renderer {
    param(
        [string]$Style,
        [string[]]$Arguments
    )

    $source = Join-Path $repoRoot "artwork\$Style\render_original_keycaps.py"
    $destination = Join-Path $output $Style
    New-Item -ItemType Directory -Path $destination -Force | Out-Null

    if ($Style -eq 'antique-brass') {
        $copiedRenderer = Join-Path $destination 'render_original_keycaps.py'
        Copy-Item -LiteralPath $source -Destination $copiedRenderer -Force
        & $Python $copiedRenderer
    }
    else {
        & $Python $source @Arguments --output-root $destination
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Artwork renderer failed for $Style with exit code $LASTEXITCODE."
    }
}

Invoke-Renderer -Style 'antique-brass' -Arguments @()
Invoke-Renderer -Style 'ashen-iron' -Arguments @('--worn-ashen')
Invoke-Renderer -Style 'rusted-ember' -Arguments @('--worn-ember')
Invoke-Renderer -Style 'thin-frame' -Arguments @('--subtle-frame')

Write-Host "Artwork rendered under $output"
