#!/usr/bin/env bash
# deep-work-trigger.sh - Toggle Deep Work mode

export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin

LOCK_FILE="/tmp/omni-deep-work.lock"
BLOCK_CONF="$HOME/.config/omni/blocked-sites.conf"
HOSTS_FILE="/etc/hosts"

enable_deep_work() {
    if [ -f "$LOCK_FILE" ]; then echo "Already enabled."; exit 0; fi
    touch "$LOCK_FILE"
    
    if [ -f "$BLOCK_CONF" ]; then
        while read -r domain; do
            if [ -n "$domain" ]; then
                echo "127.0.0.1 $domain" | sudo tee -a "$HOSTS_FILE" >/dev/null
                echo "::1 $domain" | sudo tee -a "$HOSTS_FILE" >/dev/null
            fi
        done < "$BLOCK_CONF"
    fi
    
    notify-send "Deep Work Mode ACTIVATED"
    swaync-client --dnd-on 2>/dev/null || true
}

disable_deep_work() {
    if [ ! -f "$LOCK_FILE" ]; then echo "Already disabled."; exit 0; fi
    rm -f "$LOCK_FILE"
    
    if [ -f "$BLOCK_CONF" ]; then
        while read -r domain; do
            if [ -n "$domain" ]; then
                sudo sed -i "/$domain/d" "$HOSTS_FILE"
            fi
        done < "$BLOCK_CONF"
    fi
    
    notify-send "Deep Work Mode DEACTIVATED"
    swaync-client --dnd-off 2>/dev/null || true
}

if [ "$1" = "on" ]; then
    enable_deep_work
elif [ "$1" = "off" ]; then
    disable_deep_work
else
    if [ -f "$LOCK_FILE" ]; then disable_deep_work; else enable_deep_work; fi
fi
