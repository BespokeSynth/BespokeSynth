#!/usr/bin/env bash
# ensure_task.sh
# - checks for `task` in PATH and installs it if possible
# - then runs `task` to build BespokeSynth

set -e

echo "=== ensure_task.sh ==="

# Optional argument: pass "force" or "rebuild" to force a full rebuild
# If any arguments are provided, they will be passed to task as arguments.
TASK_ARGS=("$@")

echo "Checking for 'task' on PATH..."
if command -v task >/dev/null 2>&1; then
  echo "Found 'task' in PATH."
  task --version || echo "(unable to run task --version)"
else
  echo "'task' not found. Attempting to install automatically."
  if command -v brew >/dev/null 2>&1; then
    echo "Installing go-task via Homebrew..."
    brew install go-task/tap/go-task || echo "brew install failed"
  elif command -v apt-get >/dev/null 2>&1; then
    echo "Installing go-task via apt..."
    sudo apt-get update && sudo apt-get install -y go-task || echo "apt install failed"
  else
    echo "No supported package manager found; downloading go-task v3.44.0 to ~/bin"
    mkdir -p "$HOME/bin"
    curl -L -o "$HOME/bin/task.tar.gz" "https://github.com/go-task/task/releases/download/v3.44.0/task_linux_amd64.tar.gz"
    tar -xzf "$HOME/bin/task.tar.gz" -C "$HOME/bin" task
    chmod +x "$HOME/bin/task"
    export PATH="$HOME/bin:$PATH"
  fi
fi

echo "Verifying installation..."
if ! command -v task >/dev/null 2>&1; then
  echo "'task' still not found after attempted installs. Please install it manually from https://github.com/go-task/task/releases and re-run this script."
  exit 1
fi

echo "'task' is available:"
task --version

echo "Running build with task..."
task "${TASK_ARGS[@]}"
