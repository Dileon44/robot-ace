#!/usr/bin/env bash

SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPTS_DIR/.." && pwd)"
BUILD_ACTION="$1"

set -e

BOLD="\033[1m"
GREEN="\033[32m"
RED="\033[31m"
RESET="\033[0m"

log_error() {
    echo -e "${BOLD}${RED}❌ Error: $1${RESET}"
    exit 1
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
BUILD_START=$SECONDS
"$VENV_PYTHON" "$FW_BUILDER" $*
BUILD_RET=$?
BUILD_ELAPSED=$(( SECONDS - BUILD_START ))
if [ $BUILD_RET -eq 0 ]; then
    echo -e "${BOLD}${GREEN}\n✅ Build completed successfully (${BUILD_ELAPSED}s)${RESET}"
else
    echo -e "${BOLD}${RED}\n❌ Build failed (${BUILD_ELAPSED}s)${RESET}"
    exit 1
fi
