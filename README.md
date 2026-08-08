# OMNI-OS

## What is Omni-OS?
Omni-OS is an Arch Linux distribution designed for students, who are both privacy-focused and AI enthusiasts. It brings together cutting-edge intelligence, rock-solid stability, and adaptive performance into one place. (Note: to make this work, download the full Release and re-unify it via 7-Zip or smtg and verify via the SHA256 Checksum). It fully integrates Local-LLMs btw, no telemetry.

## Key Features

- **Tri-Kernel Redundancy**: `linux-zen` for optimal desktop performance, `linux-lts` for fallback stability, and the mainline `linux` kernel.
- **20-Tier Adaptive Hardware Matrix**: Scales resources and tunes itself whether running on a 6GB potato laptop or a 128GB God-Tier workstation.
- **AI Conversational Setup**: An Ollama-powered first-boot interview that configures your system based on a natural conversation about your needs.
- **Omni-Recall**: A local, encrypted screen OCR timeline that lets you instantly search everything you've seen and done.
- **Omni-Automator**: Zero-code agentic OS commands via `Super+Space`. Tell your OS what to do in natural language, and it executes.
- **Deep Work Mode**: A comprehensive distraction blocker that utilizes `/etc/hosts` level filtering and Observer telemetry to keep you focused.
- **Unbreakable Btrfs + Snapper Rollbacks**: Automated filesystem snapshots before every update ensure you can always roll back to a working state.
- **Dynamic VRAM Purging**: Automatically frees up graphical memory before launching heavy workloads or games.
- **Enterprise Security**: Full Disk Encryption (LUKS2) paired with AppArmor profiles.
- **Zero-Friction Base**: Essential developer and power user tools (`git`, `nano`, `yt-dlp`, `ffmpeg`, `base-devel`) are pre-installed.

## Default Keybindings
| Key Combo | Action | Description |
|---|---|---|
| `Super+Return` | Launch Terminal | Opens the default terminal emulator |
| `Super+D` | App Launcher | Opens the application menu |
| `Super+Q` | Close Window | Closes the currently focused window |
| `Super+Shift+A` | Omni-Automator | Voice/Text prompt for agentic actions |
| `Super+Shift+D` | Deep Work Mode | Toggles distraction blocking |
| `Super+Space` | AI Assistant | Summons the Omni-OS conversational agent |
| `Super+S` | Omni-Recall Search | Search your encrypted OCR timeline |
| `Super+E` | File Manager | Opens the default file manager |

*(For a complete list, see [KEYBINDINGS.md](KEYBINDINGS.md))*

## The Omni Tools Suite
- **omni-cli**: The primary command-line interface for managing Omni-OS specific features.
- **omni-hw-profiler**: Analyzes hardware and applies the appropriate tier from the 20-Tier Matrix.
- **omni-recall**: The background daemon and search interface for the local OCR timeline.
- **omni-observer**: Telemetry and context-awareness engine for Deep Work Mode.


## Setup
Just download the ISO files, re-unify them with 7-Zip or smtg, and then go use Rufus or BalenaEtcher to just flash a USB (atleast 16GB cuz the ISO itself is lowk 8.82GB when fully compiled) in ISO mode.

## Contributing
Contributions are welcome! Please ensure your pull requests align with the Omni-OS' design philosophy. Refer to the contributing guidelines (coming soon) before submitting major changes.

## License
Omni-OS is released under the GPLv3 License. See the `LICENSE` file for details.
