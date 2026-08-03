#!/usr/bin/env bash
# shellcheck disable=SC2034

iso_name="omni-os"
iso_label="OMNI_OS"
iso_publisher="Omni-OS <https://omni-os.example.com>"
iso_application="Omni-OS Live/Rescue CD"
iso_version="1.0.0"
install_dir="arch"
buildmodes=('iso')
bootmodes=('bios.syslinux' 'uefi.grub')
initramfs_generator="mkinitcpio"
pacman_conf="pacman.conf"

file_permissions=(
  ["/etc/sudoers.d/omni"]="0:0:440"
  ["/usr/local/bin/deep-work-trigger.sh"]="0:0:755"
  ["/usr/local/bin/omni-action-bridge"]="0:0:755"
  ["/usr/local/bin/omni-cli"]="0:0:755"
  ["/usr/local/bin/omni-hw-profiler"]="0:0:755"
  ["/usr/local/bin/omni-observer-daemon"]="0:0:755"
  ["/usr/local/bin/omni-precacher"]="0:0:755"
  ["/usr/local/bin/omni-recall-backend"]="0:0:755"
  ["/usr/local/bin/omni-recall-embedder"]="0:0:755"
  ["/usr/local/bin/omni-recall-search"]="0:0:755"
  ["/usr/local/bin/omni-setup-engine"]="0:0:755"
  ["/usr/local/bin/omni-shell"]="0:0:755"
  ["/usr/local/bin/omni-vram-manager"]="0:0:755"
)
