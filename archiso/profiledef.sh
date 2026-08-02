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

file_permissions=(
  ["/etc/shadow"]="0:0:400"
  ["/root"]="0:0:750"
  ["/root/.automated_script.sh"]="0:0:755"
  ["/usr/local/bin/choose-mirror"]="0:0:755"
  ["/usr/local/bin/Installation_guide"]="0:0:755"
  ["/usr/local/bin/livecd-sound"]="0:0:755"
  ["/etc/sudoers.d/omni"]="0:0:440"
  ["/usr/local/bin/omni-install"]="0:0:755"
  ["/usr/local/bin/omni-config"]="0:0:755"
)
