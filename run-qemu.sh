#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
OUT_DIR="${SCRIPT_DIR}/out"

ISO_FILE=$(ls -1 "$OUT_DIR"/omni-os-*.iso | head -n 1)

if [ -z "$ISO_FILE" ]; then
    exit 1
fi

qemu-system-x86_64 \
    -m 4096 \
    -enable-kvm \
    -M q35 \
    -cpu host \
    -smp 4 \
    -bios /usr/share/ovmf/x64/OVMF.fd \
    -cdrom "$ISO_FILE" \
    -boot d \
    -vga virtio \
    -display gtk
