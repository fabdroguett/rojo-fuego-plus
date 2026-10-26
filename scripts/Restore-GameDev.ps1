[CmdletBinding(SupportsShouldProcess=$true)]
param()
$ErrorActionPreference='Stop'
$repo=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$manifestPath=Join-Path $repo 'build_artifacts/gamedev/install-manifest.json'
if (-not (Test-Path -LiteralPath $manifestPath)) { throw "No existe manifiesto de instalación: $manifestPath" }
$entries=@(Get-Content -Raw -LiteralPath $manifestPath | ConvertFrom-Json)
[array]::Reverse($entries)
foreach ($entry in $entries) {
    $target=Join-Path $repo $entry.Path
    if (-not (Test-Path -LiteralPath $target)) { Write-Warning "Destino ya ausente: $($entry.Path)"; continue }
    $hash=(Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash
    if ($hash -ne $entry.InstalledHash) { throw "Destino editado tras Setup; se preserva sin tocar: $($entry.Path)" }
    if ($entry.Existed) {
        if (-not (Test-Path -LiteralPath $entry.Backup)) { throw "Falta respaldo: $($entry.Backup)" }
        if ((Get-FileHash -Algorithm SHA256 -LiteralPath $entry.Backup).Hash -ne $entry.PreviousHash) { throw "Hash de respaldo inválido: $($entry.Backup)" }
        if ($PSCmdlet.ShouldProcess($target,'Restaurar respaldo')) { Copy-Item -LiteralPath $entry.Backup -Destination $target -Force }
    } else {
        if ($PSCmdlet.ShouldProcess($target,'Eliminar skill creada por Setup')) { Remove-Item -LiteralPath $target }
    }
}
Write-Output 'Restauración revisada; los archivos editados desde la instalación se preservan y bloquean el proceso.'
