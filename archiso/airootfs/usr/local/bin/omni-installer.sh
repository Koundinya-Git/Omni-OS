#!/bin/bash
# First, run the cinematic neural setup engine if not already completed
if [ ! -f "$HOME/.config/omni/.setup-complete" ]; then
    mkdir -p "$HOME/.config/omni"
    /usr/local/bin/omni-setup-engine
    touch "$HOME/.config/omni/.setup-complete"
fi

# Then, prepare and launch the actual system installer
sudo cp -r /etc/omni-installer-config/* /etc/calamares/
sudo calamares
