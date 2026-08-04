#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
WORK_DIR="${SCRIPT_DIR}/work"
OUT_DIR="${SCRIPT_DIR}/out"
PROFILE_DIR="${SCRIPT_DIR}/archiso"

if [ "$EUID" -ne 0 ]; then
  exit 1
fi

mkdir -p "$WORK_DIR" "$OUT_DIR"
mkdir -p "$PROFILE_DIR/airootfs/etc/systemd/system/multi-user.target.wants"
ln -sf /etc/systemd/system/omni-live-setup.service "$PROFILE_DIR/airootfs/etc/systemd/system/multi-user.target.wants/omni-live-setup.service"

mkdir -p "$PROFILE_DIR/airootfs/opt/omni-venv"
python3 -m venv "$PROFILE_DIR/airootfs/opt/omni-venv"
"$PROFILE_DIR/airootfs/opt/omni-venv/bin/pip" install --upgrade pip
"$PROFILE_DIR/airootfs/opt/omni-venv/bin/pip" install chromadb sentence-transformers

mkarchiso -v -w "$WORK_DIR" -o "$OUT_DIR" "$PROFILE_DIR"
