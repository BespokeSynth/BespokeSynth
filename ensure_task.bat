@echo off
rem ensure_task.bat
rem - checks for `task` in PATH and installs it if possible
rem - then runs `task` to build BespokeSynth

setlocal

echo === ensure_task.bat ===

rem Optional argument: pass "force" or "rebuild" to force a full rebuild
rem If any arguments are provided, they will be passed to task as arguments.
set "TASK_ARGS=%*"

echo Checking for 'task' on PATH...
where task >nul 2>&1
if %ERRORLEVEL%==0 goto :TASK_FOUND

echo 'task' not found. Attempting to install automatically.

rem Prefer winget, then choco, then scoop. If none available, download a specific release.
where winget >nul 2>&1
if %ERRORLEVEL%==0 (
  echo Installing go-task via winget...
  winget install --silent --accept-source-agreements --accept-package-agreements --id go.task || echo winget install failed
) else (
  where choco >nul 2>&1
  if %ERRORLEVEL%==0 (
    echo Installing go-task via chocolatey...
    choco install go-task -y || echo choco install failed
  ) else (
    where scoop >nul 2>&1
    if %ERRORLEVEL%==0 (
      echo Installing go-task via scoop...
      scoop install go-task || echo scoop install failed
    ) else (
      echo No package manager found; downloading go-task v3.44.0 and extracting to %USERPROFILE%\bin
      if not exist "%USERPROFILE%\bin" mkdir "%USERPROFILE%\bin"
      powershell -NoProfile -Command "try { $u='https://github.com/go-task/task/releases/download/v3.44.0/task_windows_amd64.zip'; $out=Join-Path $env:USERPROFILE 'task.zip'; Invoke-WebRequest -Uri $u -OutFile $out -UseBasicParsing; Expand-Archive -Path $out -DestinationPath (Join-Path $env:USERPROFILE 'bin') -Force; Remove-Item $out -Force } catch { Write-Error $_; exit 1 }"
      if %ERRORLEVEL% neq 0 (
        echo Failed to download or extract task. Please install go-task manually from https://github.com/go-task/task/releases
        pause
        exit /b 1
      )
      rem Add to PATH for this session
      set "PATH=%USERPROFILE%\bin;%PATH%"
    )
  )
)

goto :TASK_INST_DONE

:TASK_FOUND
echo Found 'task' in PATH.
task --version 2>nul || echo (unable to run task --version)

:TASK_INST_DONE

rem verify installation
where task >nul 2>&1
if %ERRORLEVEL% neq 0 (
  echo 'task' still not found after attempted installs. Please install it manually and re-run this script.
  pause
  exit /b 1
)

echo 'task' is available:
task --version

echo Running build with task...
task %TASK_ARGS%
