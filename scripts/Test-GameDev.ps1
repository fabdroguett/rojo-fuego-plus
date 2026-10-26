[CmdletBinding()]
param(
    [switch]$ValidateOnly,
    [switch]$RunNativeTests,
    [switch]$RunBuild,
    [ValidateRange(1,16)][int]$Jobs=2
)
$ErrorActionPreference='Continue'
$repo=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$reportDir=Join-Path $repo 'build_artifacts/gamedev'
New-Item -ItemType Directory -Force $reportDir | Out-Null
$records=[System.Collections.Generic.List[object]]::new()
function Record($Name,$Objective,$Command,$Status,$Evidence) { $records.Add([pscustomobject]@{Name=$Name;Objective=$Objective;Command=$Command;Status=$Status;Evidence=$Evidence}) }
try {
    & (Join-Path $PSScriptRoot 'Setup-GameDev.ps1') -Validate
    if ($LASTEXITCODE -and $LASTEXITCODE -ne 0) { throw 'Skill validation falló.' }
    Record 'Skills locales' 'Comprobar skills y copias' 'Setup-GameDev.ps1 -Validate' 'PASS' 'Sincronía verificada por hash.'
} catch { Record 'Skills locales' 'Comprobar skills y copias' 'Setup-GameDev.ps1 -Validate' 'FAIL' $_.Exception.Message }
if (-not $ValidateOnly) {
    if ($RunNativeTests) {
        $cmd="wsl.ps1 -Action test -Jobs $Jobs"
        $outputLines=@(& (Join-Path $repo 'tools/rojo_fuego/wsl.ps1') -Action test -Jobs $Jobs 2>&1)
        $code=$LASTEXITCODE
        $testLog=Join-Path $reportDir ("native-tests-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
        $outputLines | Set-Content -LiteralPath $testLog -Encoding utf8
        $summary=@($outputLines | Where-Object { $_ -match 'Validated \d+ quests|Ran \d+ tests|^OK$|^FAILED' }) -join '; '
        Record 'Python/quest tests WSL' 'Validar misiones y unittest' $cmd $(if($code -eq 0){'PASS'}else{'FAIL'}) ("$summary; log=$testLog")
    }
    if ($RunBuild) {
        $cmd="& '$repo\tools\rojo_fuego\wsl.ps1' -Action build -Jobs $Jobs"
        $buildOutput=@(& (Join-Path $repo 'tools/rojo_fuego/wsl.ps1') -Action build -Jobs $Jobs 2>&1)
        $code=$LASTEXITCODE
        $buildLog=Join-Path $reportDir ("build-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
        $buildOutput | Set-Content -LiteralPath $buildLog -Encoding utf8
        $rom=Join-Path $repo 'build_artifacts/wsl/pokefirered.gba'
        $hash=if(Test-Path -LiteralPath $rom){(Get-FileHash -Algorithm SHA256 -LiteralPath $rom).Hash}else{'ROM no encontrada'}
        $memory=@($buildOutput | Where-Object { $_ -match 'EWRAM:|IWRAM:|ROM:' }) -join '; '
        Record 'Build FireRed WSL' 'Compilar ROM en Ubuntu WSL' $cmd $(if($code -eq 0){'PASS'}else{'FAIL'}) ("$memory; SHA256=$hash; log=$buildLog")
    }
    if (-not $RunNativeTests -and -not $RunBuild) { Record 'Tests nativos y build' 'Sin ejecución solicitada' 'Usar -RunNativeTests y/o -RunBuild' 'SKIPPED' 'No se solicitó ejecución costosa.' }
}
$report=Join-Path $reportDir ("test-report-{0}.json" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
$records | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $report -Encoding utf8
$logLines = foreach ($r in $records) { "| $(Get-Date -Format yyyy-MM-dd) | $($r.Name): $($r.Objective) | ``$($r.Command -replace '\|','/')`` | $($r.Evidence -replace '\|','/') | $($r.Status) |" }
Add-Content -LiteralPath (Join-Path $repo 'docs/development/TESTING.md') -Value "`n$($logLines -join "`n")" -Encoding utf8
$records | Format-Table -Wrap
Write-Output "Reporte: $report"
if (@($records | Where-Object Status -eq 'FAIL').Count) { exit 1 }
