@echo off
rem BespokeSynth Korean patch (uninstall) - runs korean_patch_windows.ps1 as administrator
net session >nul 2>&1
if %errorlevel% neq 0 (
   powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
   exit /b
)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0korean_patch_windows.ps1" -Uninstall
pause
