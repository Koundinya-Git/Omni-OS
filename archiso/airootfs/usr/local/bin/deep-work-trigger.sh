#!/usr/bin/env bash
export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin
lck="/tmp/omni-deep-work.lock"
cfg="$HOME/.config/omni/blocked-sites.conf"
hosts="/etc/hosts"
on() {
    [ -f "$lck" ] && { echo "Already enabled."; exit 0; }
    touch "$lck"
    if [ -f "$cfg" ]; then
        while read -r d; do
            [ -n "$d" ] && {
                echo "127.0.0.1 $d" | sudo tee -a "$hosts" >/dev/null
                echo "::1 $d" | sudo tee -a "$hosts" >/dev/null
            }
        done < "$cfg"
    fi
    notify-send "Deep Work Mode ACTIVATED"
    swaync-client --dnd-on 2>/dev/null || true
}
off() {
    [ ! -f "$lck" ] && { echo "Already disabled."; exit 0; }
    rm -f "$lck"
    if [ -f "$cfg" ]; then
        while read -r d; do
            [ -n "$d" ] && sudo sed -i "/$d/d" "$hosts"
        done < "$cfg"
    fi
    notify-send "Deep Work Mode DEACTIVATED"
    swaync-client --dnd-off 2>/dev/null || true
}
if [ "$1" = "on" ]; then on
elif [ "$1" = "off" ]; then off
else
    [ -f "$lck" ] && off || on
fi
