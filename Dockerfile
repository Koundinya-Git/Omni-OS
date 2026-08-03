FROM archlinux:base-devel

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm archiso git mtools dosfstools squashfs-tools libisoburn grub rust cargo cmake qt6-base perl && \
    pacman -Scc --noconfirm

RUN useradd -m builder && echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
USER builder
RUN git clone https://aur.archlinux.org/paru.git /tmp/paru && cd /tmp/paru && makepkg -sri --noconfirm
RUN paru -S --noconfirm opera-gx catppuccin-gtk-theme-mocha vmtouch

USER root
RUN mkdir -p /customrepo && \
    find /home/builder/.cache/paru/clone -name "*.pkg.tar.zst" -exec cp {} /customrepo/ \; && \
    repo-add /customrepo/customrepo.db.tar.gz /customrepo/*.pkg.tar.zst

WORKDIR /build
COPY archiso/ /build/profile/
COPY src/ /build/src/

RUN cd /build/src/omni-precacher && cargo build --release && \
    cp target/release/omni-precacher /build/profile/airootfs/usr/local/bin/

RUN cd /build/src/omni-greeter && mkdir build && cd build && \
    cmake .. && make && \
    cp omni-greeter /build/profile/airootfs/usr/local/bin/

RUN chmod +x /build/profile/airootfs/usr/local/bin/*

RUN echo '#!/bin/bash' > /build/entrypoint.sh && \
    echo 'set -e' >> /build/entrypoint.sh && \
    echo 'mkdir -p /build/work /build/out' >> /build/entrypoint.sh && \
    echo 'mkarchiso -v -w /build/work -o /build/out /build/profile' >> /build/entrypoint.sh && \
    echo 'ls -lh /build/out/*.iso' >> /build/entrypoint.sh && \
    chmod +x /build/entrypoint.sh

ENTRYPOINT ["/build/entrypoint.sh"]
