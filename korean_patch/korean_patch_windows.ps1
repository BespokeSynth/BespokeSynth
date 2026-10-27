# BespokeSynth 한국어 패치 설치/제거 스크립트 (Windows)
#
#   install_windows.bat / uninstall_windows.bat 에서 실행됩니다.
#   직접 실행: powershell -ExecutionPolicy Bypass -File korean_patch_windows.ps1 [-Uninstall] [-ResourceDir <경로>]

param(
   [switch]$Uninstall,
   [string]$ResourceDir = ""
)

$ErrorActionPreference = "Stop"

function Fail($msg)
{
   Write-Host "오류: $msg" -ForegroundColor Red
   exit 1
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# 패치 파일 위치: 배포용 zip이면 files\, 저장소에서 바로 실행하면 ..\resource
$payloadDir = Join-Path $scriptDir "files"
if (-not (Test-Path (Join-Path $payloadDir "tooltips_kor.txt")))
{
   $payloadDir = Join-Path $scriptDir "..\resource"
}
if (-not $Uninstall -and -not (Test-Path (Join-Path $payloadDir "tooltips_kor.txt")))
{
   Fail "패치 파일(tooltips_kor.txt, NanumGothic-*.ttf)을 찾을 수 없습니다."
}

# BespokeSynth 리소스 폴더 찾기 (frabk.ttf가 있는 폴더)
if ($ResourceDir -eq "")
{
   $candidates = @()
   foreach ($base in @($env:ProgramFiles, ${env:ProgramFiles(x86)}, $env:ProgramW6432, $(if ($env:LOCALAPPDATA) { "$env:LOCALAPPDATA\Programs" })))
   {
      if ($base) { $candidates += (Join-Path $base "BespokeSynth\resource") }
   }
   foreach ($c in $candidates)
   {
      if (Test-Path (Join-Path $c "frabk.ttf"))
      {
         $ResourceDir = $c
         break
      }
   }
}
if ($ResourceDir -eq "")
{
   Write-Host "BespokeSynth 설치 위치를 자동으로 찾지 못했습니다."
   $ResourceDir = Read-Host "BespokeSynth 폴더 안의 resource 폴더 경로를 입력하세요 (예: D:\BespokeSynth\resource)"
   $ResourceDir = $ResourceDir.Trim('"', ' ')
}
if (Test-Path (Join-Path $ResourceDir "resource\frabk.ttf"))
{
   $ResourceDir = Join-Path $ResourceDir "resource"   # BespokeSynth 폴더를 입력한 경우
}
if (-not (Test-Path (Join-Path $ResourceDir "frabk.ttf")))
{
   Fail "'$ResourceDir'에 frabk.ttf가 없습니다. BespokeSynth의 resource 폴더가 맞는지 확인하세요."
}

# 사용자 데이터 폴더 (userprefs.json 위치). bespoke와 같은 규칙을 따릅니다.
if ($env:BESPOKE_DATA_DIR)
{
   $dataDir = $env:BESPOKE_DATA_DIR
}
else
{
   $dataDir = Join-Path ([Environment]::GetFolderPath("MyDocuments")) "BespokeSynth"
}
$prefs = Join-Path $dataDir "userprefs.json"

# userprefs.json의 "tooltips" 값을 설정 (다른 설정은 그대로 둠)
function Set-TooltipsPref($value)
{
   $utf8 = New-Object System.Text.UTF8Encoding($false)
   if (-not (Test-Path $prefs))
   {
      if ($value -eq "tooltips_eng.txt") { return }
      New-Item -ItemType Directory -Force -Path $dataDir | Out-Null
      [IO.File]::WriteAllText($prefs, "{`n   `"tooltips`" : `"$value`"`n}`n", $utf8)
      return
   }
   Copy-Item $prefs "$prefs.bak" -Force
   $text = [IO.File]::ReadAllText($prefs)
   if ($text -match '"tooltips"\s*:')
   {
      $text = [regex]::Replace($text, '("tooltips"\s*:\s*)"[^"]*"', ('$1"' + $value + '"'))
   }
   else
   {
      $text = ([regex]'\{').Replace($text, "{`n   `"tooltips`" : `"$value`",", 1)
   }
   [IO.File]::WriteAllText($prefs, $text, $utf8)
}

Write-Host "BespokeSynth 리소스 폴더: $ResourceDir"

try
{
   if (-not $Uninstall)
   {
      # 원본 폰트 백업 (이미 백업이 있으면 덮어쓰지 않음)
      foreach ($f in @("frabk.ttf", "frabk_m.ttf"))
      {
         $orig = Join-Path $ResourceDir "$f.orig"
         if (-not (Test-Path $orig)) { Copy-Item (Join-Path $ResourceDir $f) $orig }
      }

      # 기존 bespoke는 frabk.ttf만 읽으므로, 한글이 들어 있는 나눔고딕을 같은 이름으로 넣습니다.
      Copy-Item (Join-Path $payloadDir "NanumGothic-Regular.ttf") (Join-Path $ResourceDir "frabk.ttf") -Force
      Copy-Item (Join-Path $payloadDir "NanumGothic-Bold.ttf") (Join-Path $ResourceDir "frabk_m.ttf") -Force
      foreach ($f in @("NanumGothic-Regular.ttf", "NanumGothic-Bold.ttf", "NanumGothic_OFL.txt", "tooltips_kor.txt"))
      {
         Copy-Item (Join-Path $payloadDir $f) (Join-Path $ResourceDir $f) -Force
      }

      Set-TooltipsPref "tooltips_kor.txt"
      Write-Host "설정 파일: $prefs"
      Write-Host ""
      Write-Host "한국어 패치를 설치했습니다. BespokeSynth를 다시 시작하세요. (툴팁 켜기/끄기: F1)" -ForegroundColor Green
   }
   else
   {
      foreach ($f in @("frabk.ttf", "frabk_m.ttf"))
      {
         $orig = Join-Path $ResourceDir "$f.orig"
         if (Test-Path $orig) { Move-Item $orig (Join-Path $ResourceDir $f) -Force }
         else { Write-Host "경고: $f 원본 백업($f.orig)이 없어 폰트를 복원하지 못했습니다." -ForegroundColor Yellow }
      }
      Remove-Item (Join-Path $ResourceDir "tooltips_kor.txt") -ErrorAction SilentlyContinue

      Set-TooltipsPref "tooltips_eng.txt"
      Write-Host ""
      Write-Host "한국어 패치를 제거했습니다. BespokeSynth를 다시 시작하세요." -ForegroundColor Green
   }
}
catch [System.UnauthorizedAccessException]
{
   Fail "파일을 쓸 권한이 없습니다. 관리자 권한으로 실행하고, BespokeSynth가 실행 중이면 종료한 뒤 다시 시도하세요."
}
catch [System.IO.IOException]
{
   Fail "파일을 바꾸지 못했습니다. BespokeSynth가 실행 중이면 종료한 뒤 다시 시도하세요. ($($_.Exception.Message))"
}
catch
{
   Fail $_.Exception.Message
}
