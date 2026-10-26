[CmdletBinding()]
param(
    [switch]$Diagnose,
    [switch]$Install,
    [switch]$Validate,
    [switch]$DryRun,
    [switch]$IncludeExternal,
    [switch]$Force
)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$selectedModes = @()
if ($Diagnose) { $selectedModes += 'Diagnose' }
if ($Install) { $selectedModes += 'Install' }
if ($Validate) { $selectedModes += 'Validate' }
if ($selectedModes.Count -gt 1) { throw 'Usa solo uno entre -Diagnose, -Install y -Validate.' }
if ($Force -and -not $Install) { throw '-Force solo es válido junto con -Install.' }
if (-not ($Diagnose -or $Install -or $Validate)) { $Diagnose = $true }
$logDir = Join-Path $repo 'build_artifacts/gamedev/logs'
New-Item -ItemType Directory -Force $logDir | Out-Null
$log = Join-Path $logDir ("setup-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
Start-Transcript -LiteralPath $log | Out-Null
try {
    $canonical = Join-Path $repo 'skills'
    $names = Get-ChildItem -LiteralPath $canonical -Directory | Sort-Object Name
    if (-not $names -or $names.Count -ne 17) { throw "Se esperaban 17 skills en $canonical; encontradas: $($names.Count)." }
    $files = foreach ($name in $names) {
        $source = Join-Path $name.FullName 'SKILL.md'
        if (-not (Test-Path -LiteralPath $source)) { throw "Falta $source" }
        [pscustomobject]@{ Name=$name.Name; Source=$source; Hash=(Get-FileHash -Algorithm SHA256 -LiteralPath $source).Hash }
    }
    if ($Diagnose) {
        [pscustomobject]@{ Repository=$repo; CanonicalSkills=$files.Count; CodexSkills=(Test-Path (Join-Path $repo '.agents/skills')); ClaudeSkills=(Test-Path (Join-Path $repo '.claude/skills')); ExternalAuditOnly=[bool]$IncludeExternal; PowerShell=$PSVersionTable.PSVersion.ToString() } | Format-List
        if ($IncludeExternal) { Get-Content (Join-Path $repo 'docs/development/01-skills-inventory.md') }
    }
    if ($Validate) {
        foreach ($targetRoot in @('.agents/skills','.claude/skills')) {
            foreach ($entry in $files) {
                $target = Join-Path $repo (Join-Path $targetRoot (Join-Path $entry.Name 'SKILL.md'))
                if (-not (Test-Path -LiteralPath $target)) { throw "Falta instalación local: $target" }
                if ((Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash -ne $entry.Hash) { throw "Skill divergente: $target" }
                $content = Get-Content -Raw -LiteralPath $target
                if ($content -notmatch "(?m)^name: $([regex]::Escape($entry.Name))\r?$") { throw "Frontmatter inválido: $target" }
            }
        }
        Write-Output 'PASS: 17 skills canónicas coinciden con las copias Codex y Claude.'
    }
    if ($Install) {
        $manifestPath = Join-Path $repo 'build_artifacts/gamedev/install-manifest.json'
        $manifestDir = Split-Path -Parent $manifestPath
        New-Item -ItemType Directory -Force $manifestDir | Out-Null
        $manifest = [System.Collections.Generic.List[object]]::new()
        foreach ($targetRoot in @('.agents/skills','.claude/skills')) {
            foreach ($entry in $files) {
                $target = Join-Path $repo (Join-Path $targetRoot (Join-Path $entry.Name 'SKILL.md'))
                $targetDir = Split-Path -Parent $target
                New-Item -ItemType Directory -Force $targetDir | Out-Null
                $exists = Test-Path -LiteralPath $target
                $oldHash = if ($exists) { (Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash } else { $null }
                if ($exists -and $oldHash -eq $entry.Hash) { continue }
                if (-not $DryRun -and (Test-Path -LiteralPath $manifestPath) -and (Get-Content -Raw -LiteralPath $manifestPath | ConvertFrom-Json)) {
                    throw 'Hay una instalación anterior sin restaurar. Ejecuta Restore-GameDev.ps1 antes de crear otro manifiesto.'
                }
                if ($exists -and -not $Force) { throw "Destino existente distinto (sin cambios): $target. Revisa diff o usa -Install -Force para reemplazarlo con respaldo." }
                $backup = $null
                if ($exists) {
                    $backupDir = Join-Path $manifestDir (Join-Path 'backups' (Join-Path $targetRoot $entry.Name))
                    New-Item -ItemType Directory -Force $backupDir | Out-Null
                    $backup = Join-Path $backupDir 'SKILL.md'
                    Copy-Item -LiteralPath $target -Destination $backup
                }
                $record = [pscustomobject]@{ Path=$target.Substring($repo.Length).TrimStart('\','/').Replace('\','/'); Existed=$exists; PreviousHash=$oldHash; Backup=$backup; InstalledHash=$entry.Hash }
                $manifest.Add($record)
                if (-not $DryRun) { Copy-Item -LiteralPath $entry.Source -Destination $target -Force }
                Write-Output ("{0} {1}" -f $(if ($DryRun) {'DRYRUN'} else {'INSTALLED'}), $record.Path)
            }
        }
        if (-not $DryRun -and $manifest.Count -gt 0) { $manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $manifestPath -Encoding utf8 }
        elseif ($manifest.Count -eq 0) { Write-Output 'No hay destinos para instalar.' }
        if ($IncludeExternal) { Write-Output 'External: diagnóstico del inventario solamente; no se descargan ni ejecutan colecciones.' }
    }
} finally { Stop-Transcript | Out-Null }

