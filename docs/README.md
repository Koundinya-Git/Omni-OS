# OMNI-OS
**The Sovereign AI-Powered OS**

---

## What is Omni-OS?
Omni-OS is an enterprise-grade, Apex Arch Linux distribution designed for power users, developers, and AI enthusiasts. It brings together cutting-edge intelligence, rock-solid stability, and adaptive performance into a single, cohesive ecosystem. Engineered with a Tokyo Night glassmorphic design language, Omni-OS redefines what a modern desktop operating system can be.

## Key Features

- **Tri-Kernel Redundancy**: Ships with `linux-zen` for optimal desktop performance, `linux-lts` for fallback stability, and the mainline `linux` kernel.
- **20-Tier Adaptive Hardware Matrix**: Intelligently scales resources, tuning itself whether running on a 4GB potato laptop or a 128GB God-Tier workstation.
- **AI Conversational Setup**: An Ollama-powered first-boot interview that configures your system based on a natural conversation about your needs.
- **Omni-Recall**: A local, encrypted screen OCR timeline that lets you instantly search everything you've seen and done.
- **Omni-Automator**: Zero-code agentic OS commands via `Super+Space`. Tell your OS what to do in natural language, and it executes.
- **Deep Work Mode**: A comprehensive distraction blocker that utilizes `/etc/hosts` level filtering and Observer telemetry to keep you focused.
- **Unbreakable Btrfs + Snapper Rollbacks**: Automated filesystem snapshots before every update ensure you can always roll back to a working state.
- **Dynamic VRAM Purging**: Automatically frees up graphical memory before launching heavy workloads or games.
- **Enterprise Security**: Full Disk Encryption (LUKS2) paired with AppArmor profiles out-of-the-box.
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

## Building the ISO

### Prerequisites
You need an Arch Linux host environment with the `archiso` package installed.
```bash
sudo pacman -S archiso
```

### Build Instructions
Run the build script with root privileges:
```bash
sudo ./build.sh
```

### Testing the ISO
You can quickly test the built image using QEMU:
```bash
./run-qemu.sh
```

## Project Structure
```text
Omni-OS/
├── archiso/
│   ├── airootfs/
│   │   ├── etc/
│   │   └── usr/
│   ├── profiledef.sh
│   └── packages.x86_64
├── docs/
│   ├── README.md
│   └── KEYBINDINGS.md
├── scripts/
│   ├── build.sh
│   └── run-qemu.sh
└── LICENSE
```

## Contributing
Contributions are welcome! Please ensure your pull requests align with the Omni-OS design philosophy. Refer to the contributing guidelines (coming soon) before submitting major changes.

## License
Omni-OS is released under the GPLv3 License. See the `LICENSE` file for details.
