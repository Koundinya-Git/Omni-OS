FROM archlinux:latest

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm archiso git mtools dosfstools squashfs-tools libisoburn grub && \
    pacman -Scc --noconfirm

WORKDIR /build
COPY archiso/ /build/profile/
RUN chmod +x /build/profile/airootfs/usr/local/bin/*

RUN echo '#!/bin/bash' > /build/entrypoint.sh && \
    echo 'set -e' >> /build/entrypoint.sh && \
    echo 'mkdir -p /build/work /build/out' >> /build/entrypoint.sh && \
    echo 'mkarchiso -v -w /build/work -o /build/out /build/profile' >> /build/entrypoint.sh && \
    echo 'ls -lh /build/out/*.iso' >> /build/entrypoint.sh && \
    chmod +x /build/entrypoint.sh

ENTRYPOINT ["/build/entrypoint.sh"]
