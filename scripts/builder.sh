#!/usr/bin/env bash

SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPTS_DIR/.." && pwd)"
BUILD_ACTION="$1"

set -e

log_error() {
    echo "❌ Error: $1"
    exit 1
}

run_step() {
    local description="$1"
    local cmd="$2"
    echo
    echo "▶️  $cmd"
    if eval "$cmd"; then
        echo "✅ $description completed successfully"
    else
        echo "❌ $description failed"
        exit 1
    fi
}

# Detect OS
if [ "$OS" == "Windows_NT" ]; then
    OS_NAME="Windows_NT"
    VENV_SRC="Scripts"
    PYTHON_NAME="python"
else
    OS_NAME=$(uname -s)
    VENV_SRC="bin"
    PYTHON_NAME="python3"
fi

echo "-----------------------------------------------------------------"
echo "Task started (OS: $OS_NAME)"

cd "$ROOT_DIR"

# Check Python
if ! command -v "$PYTHON_NAME" &> /dev/null; then
    log_error "Python not found. Please install $PYTHON_NAME"
fi

# Set up virtual environment
VENV_DIR="$ROOT_DIR/.venv"
if [ ! -d "$VENV_DIR" ]; then
    echo ".venv not found. Creating virtual environment..."
    "$PYTHON_NAME" -m venv "$VENV_DIR" || log_error "Failed to create virtual environment"
    "$VENV_DIR/$VENV_SRC/pip" install --upgrade pip || log_error "Failed to upgrade pip"
    if [ -f "$ROOT_DIR/scripts/requirements.txt" ]; then
        "$VENV_DIR/$VENV_SRC/pip" install -r "$ROOT_DIR/scripts/requirements.txt" || log_error "Failed to install dependencies"
    fi
else
    echo "Using existing .venv"
fi

VENV_PYTHON="$VENV_DIR/$VENV_SRC/$PYTHON_NAME"
PYTHON_VER=$("$VENV_PYTHON" --version 2>&1)
echo "Use $PYTHON_VER"

# Run Python build script, passing all arguments
FW_BUILDER="$SCRIPTS_DIR/fw_builder.py"
build_cmd="$VENV_PYTHON $FW_BUILDER $*"
run_step "Build" "$build_cmd"
