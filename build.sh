#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
WORK_DIR="${SCRIPT_DIR}/work"
OUT_DIR="${SCRIPT_DIR}/out"
PROFILE_DIR="${SCRIPT_DIR}/archiso"

echo "Building Omni-OS iso..."

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root (sudo ./build.sh)"
  exit 1
fi

mkdir -p "$WORK_DIR" "$OUT_DIR"

mkarchiso -v -w "$WORK_DIR" -o "$OUT_DIR" "$PROFILE_DIR"

echo "Build complete. ISO is in $OUT_DIR"
