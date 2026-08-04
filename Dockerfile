FROM archlinux:base-devel

RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm archiso git mtools dosfstools squashfs-tools libisoburn grub rust cargo cmake qt6-base qt6-multimedia qt6-networkauth perl && \
    pacman -Scc --noconfirm

RUN useradd -m builder && echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers
USER builder
RUN git clone https://aur.archlinux.org/paru.git /tmp/paru && cd /tmp/paru && makepkg -sri --noconfirm
RUN paru -S --noconfirm opera-gx catppuccin-gtk-theme-mocha

USER root
RUN mkdir -p /customrepo && \
    find /home/builder/.cache/paru/clone -name "*.pkg.tar.zst" -exec cp {} /customrepo/ \; && \
    repo-add /customrepo/customrepo.db.tar.gz /customrepo/*.pkg.tar.zst

WORKDIR /build
COPY archiso/ /build/profile/
COPY src/ /build/src/
COPY calamares/ /build/profile/airootfs/etc/omni-installer-config/

RUN git clone https://github.com/hoytech/vmtouch.git /tmp/vmtouch && \
    cd /tmp/vmtouch && \
    gcc -O3 -o vmtouch vmtouch.c && \
    cp vmtouch /build/profile/airootfs/usr/local/bin/

RUN cd /build/src/omni-precacher && cargo build --release && \
    cp target/release/omni-precacher /build/profile/airootfs/usr/local/bin/

RUN cd /build/src/omni-greeter && mkdir build && cd build && \
    cmake .. && make && \
    cp omni-greeter /build/profile/airootfs/usr/local/bin/

RUN cd /build/src/omni-action-bridge && cargo build --release && \
    cp target/release/omni-action-bridge /build/profile/airootfs/usr/local/bin/

RUN mkdir -p /build/src/omni-setup-engine/assets && \
    dd if=/dev/zero of=/build/src/omni-setup-engine/assets/ambient.ogg bs=1024 count=1 && \
    cd /build/src/omni-setup-engine && mkdir build && cd build && \
    cmake .. && make && \
    cp omni-setup-engine /build/profile/airootfs/usr/local/bin/

RUN chmod +x /build/profile/airootfs/usr/local/bin/*

RUN ln -sf /usr/lib/systemd/system/greetd.service /build/profile/airootfs/etc/systemd/system/display-manager.service && \
    rm -rf /build/profile/airootfs/etc/systemd/system/getty@tty1.service.d

RUN pacman -S --noconfirm ollama && \
    export OLLAMA_MODELS=/build/profile/airootfs/var/lib/ollama/models && \
    mkdir -p $OLLAMA_MODELS && \
    ollama serve & \
    sleep 5 && \
    ollama pull llama3.2:1b && \
    pkill ollama || true && \
    chmod -R 777 /build/profile/airootfs/var/lib/ollama

RUN echo '#!/bin/bash' > /build/entrypoint.sh && \
    echo 'set -e' >> /build/entrypoint.sh && \
    echo 'mkdir -p /build/work /build/out' >> /build/entrypoint.sh && \
    echo 'mkarchiso -v -w /build/work -o /build/out /build/profile' >> /build/entrypoint.sh && \
    echo 'ls -lh /build/out/*.iso' >> /build/entrypoint.sh && \
    chmod +x /build/entrypoint.sh

ENTRYPOINT ["/build/entrypoint.sh"]
