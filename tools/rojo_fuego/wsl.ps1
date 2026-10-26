param(
    [ValidateSet('sync', 'check', 'test', 'build', 'shell', 'stop')]
    [string]$Action = 'check',
    [ValidateRange(1, 16)]
    [int]$Jobs = 2,
    [string]$Distro = 'Ubuntu-24.04'
)
$ErrorActionPreference = 'Stop'
$env:WSL_UTF8 = '1'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
if ($Action -eq 'stop') {
    & wsl.exe --terminate $Distro
    exit $LASTEXITCODE
}
if ($Action -eq 'shell') {
    & wsl.exe -d $Distro --cd /work
    exit $LASTEXITCODE
}
$linuxRepo = (& wsl.exe -d $Distro --exec wslpath -a -u $repo)
if ($LASTEXITCODE -ne 0) { throw 'No se pudo abrir Ubuntu. Consulta docs/ENTORNO_WSL.md.' }
$linuxRepo = $linuxRepo.Trim()
$output = Join-Path $repo 'build_artifacts/wsl'
New-Item -ItemType Directory -Force $output | Out-Null
$previous = Join-Path $output 'previous-sources.json'
& wsl.exe -d $Distro --exec test -f /work/.rf-wsl-sources.json
if ($LASTEXITCODE -eq 0) {
    & wsl.exe -d $Distro --exec cp /work/.rf-wsl-sources.json "$linuxRepo/build_artifacts/wsl/previous-sources.json"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} else {
    Set-Content -LiteralPath $previous -Value '{}' -Encoding utf8
}
python (Join-Path $PSScriptRoot 'package_wsl.py') $previous
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& wsl.exe -d $Distro --exec bash "$linuxRepo/tools/rojo_fuego/wsl_work.sh" $Action $linuxRepo $Jobs "$linuxRepo/build_artifacts/wsl/sources.tar"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
if ($Action -eq 'build') {
    $target = Join-Path $output 'pokefirered.gba'
    if (Test-Path -LiteralPath $target) {
        Copy-Item -LiteralPath $target -Destination "$target.$(Get-Date -Format yyyyMMdd-HHmmss).bak"
    }
    $linuxTarget = (& wsl.exe -d $Distro --exec wslpath -a -u $target).Trim()
    & wsl.exe -d $Distro --exec cp /work/pokefirered.gba $linuxTarget
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    Get-FileHash -Algorithm SHA256 -LiteralPath $target
}
