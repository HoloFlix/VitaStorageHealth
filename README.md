# Vita Storage Health 🎮💾

[![Platform](https://img.shields.io/badge/Platform-PS%20Vita%20%7C%20PS%20TV-blue.svg)](https://vitasdk.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Category](https://img.shields.io/badge/Category-Utility-emerald.svg)]()

**Vita Storage Health** is an all-in-one storage diagnostic, benchmarking, maintenance, and TaiHEN plugin management suite for the PlayStation Vita (PCH-1000 / PCH-2000 / PS TV).

Developed in native C using Vita2D, it provides deep hardware inspection, real-time read/write throughput benchmarks, sector surface health scanning, fake MicroSD card capacity detection, system maintenance tools, and a protected TaiHEN plugin manager.

---

## 🌟 Key Features

### 1. Multi-Partition Mount Inspector (`OVERVIEW`)
- **Real-Time Storage Hardware Query:**
  - `ux0:` (Primary storage / SD2Vita / Sony Memory Card)
  - `ur0:` (Internal system flash eMMC)
  - `imc0:` (Internal 1GB/2GB storage partition on Slim & PS TV)
  - `uma0:` (Secondary USB storage / official Memory Card slot)
  - `xmc0:`, `gro0:`, `grw0:` (Game Card read/write partitions)
- **Active Boot Device Safeguards:**
  - Clearly badges `🔒 BOOT DEVICE (SYSTEM PROTECTED)` and `🔒 INTERNAL OS (UR0)` to prevent user mistakes.
  - Displays partition hardware type, formatted capacity, used/free space gauges, and calculated health score.

### 2. Isolated Speed Benchmarking (`BENCHMARK`)
- **Per-Partition Isolation:** Speed test results are kept isolated per partition and never bleed across devices.
- **Sequential Read / Write:** Measures throughput in **MB/s** using pattern-buffered payloads.
- **Random 4KB IOPS:** Tests micro read/write speeds to gauge homebrew and game asset loading performance.
- **Access Latency:** Measures controller response time in milliseconds (**ms**).

### 3. Sector Surface Heatmap & Fake Card Detector (`SCAN`)
- **100-Sector Visual Matrix:** Renders an animated 10x10 colored sector grid displaying healthy (green), slow/degraded (amber), or corrupt (red) blocks.
- **H2testw-Style Fake Capacity Verifier:** Verifies written blocks using CRC32 checksum hashes to identify counterfeit microSD cards (e.g. cards hacked to report 512GB that fail past 32GB).

### 4. System Maintenance & Storage Doctor (`TOOLS`)
- **Cache & Temp Purger (`[SQUARE]`):** Safely cleans temporary log files, download cache residue, and removes stale transaction locks (`.vsh_speedtest.tmp`, `.vsh_integrity.tmp`).
- **Full Diagnostic Log Exporter (`[TRIANGLE]`):** Exports full hardware stats, benchmark scores, surface scans, and complete TaiHEN plugin status to `ux0:data/VitaStorageHealth/report.txt`.
- **PC Detection Doctor:** Explains why Windows PC card readers fail on SD2Vita cards (raw texFAT superfloppy format without MBR partition table) and provides guidance for mounting over VitaShell USB mode.
- **Database Rebuilder & Card Unlink Tool (`[CROSS]`):** Safely unlinks `id.dat` to clear account mismatch locks and trigger an `app.db` refresh on next reboot without deleting user games or saves.

### 5. TaiHEN Plugin Manager (`PLUGINS`)
- **160+ Known Plugins Database:** Sourced from AutoPlugin2, with author credits, version numbers, and descriptions.
- **Status Badges:** Displays `ACTIVE` (loaded in config), `NOT IN CFG` (present on disk but disabled), or `ORPHANED`.
- **Boot Protection Locks:** Kernel drivers required for system boot (`yamt.skprx`, `storagemgr.skprx`) are locked with a `[BOOT LOCK]` badge, preventing accidental lockout.
- **Interactive Toggling (`[CROSS]`):** Toggles plugins directly in/out of `tai/config.txt` with automatic section routing (`*KERNEL` for `.skprx` and `*ALL` for `.suprx`).
- **Automated Backup:** Automatically mirrors `config.txt.bak` before making changes.

---

## 🎮 Controls

| Button | Screen | Action |
| :--- | :--- | :--- |
| **`L` / `R` Triggers** | *Any* | Cycle Tabs (`OVERVIEW` ⇄ `BENCHMARK` ⇄ `SCAN` ⇄ `TOOLS` ⇄ `PLUGINS`) |
| **`D-Pad ◄ / ►`** | *Any* | Switch active storage partition (`ux0:`, `ur0:`, `imc0:`, `uma0:`) |
| **`SELECT`** | *Any* | **Live Re-detect:** Probes all physical storages, free space & driver mappings |
| **`Cross (X)`** | Overview / Tools / Plugins | Run Benchmark / Fix Conflicts & Unlink id.dat / Toggle Plugin |
| **`Triangle (△)`** | Overview / Tools / Plugins | Run Surface Scan / Export Diagnostic Report / Re-scan Plugins |
| **`Square (□)`** | Tools | Purge junk cache & temporary lockfiles |
| **`D-Pad ▲ / ▼`** | Plugins | Scroll plugin list |

---

## 📁 Repository Structure

```
VitaStorageHealth/
├── Makefile                       # VitaSDK Makefile
├── CMakeLists.txt                 # CMake configuration
├── README.md                      # Documentation
├── LICENSE                        # Open-source license
├── .gitignore                     # Git ignore rules for VitaSDK artifacts
├── catalog_submission/            # Prepared NeoVitaDB catalog submission
│   ├── README.md                  # PR instructions for NeoVitaDB
│   ├── apps/vita/                 # 1939-vita-storage-health.json
│   └── icons_vita/                # 1939-vita-storage-health.png (128x128)
├── src/                           # Source code (C99 / VitaSDK)
│   ├── main.c                     # System entry point
│   ├── storage.h / .c             # Hardware partition inspector & safety flags
│   ├── benchmark.h / .c           # Speed benchmark engine
│   ├── integrity.h / .c           # Surface heatmap & fake card detector
│   ├── tools.h / .c               # Maintenance suite & diagnostic exporter
│   ├── plugins.h / .c             # TaiHEN plugin parser & boot driver locks
│   ├── plugins_db.h               # Sourced AutoPlugin2 160+ plugin database
│   └── ui.h / .c                  # Vita2D green theme renderer
├── sce_sys/                       # LiveArea configuration and assets
│   ├── icon0.png                  # LiveArea 128x128 icon
│   ├── pic0.png                   # LiveArea splash image
│   └── livearea/contents/         # LiveArea template.xml, bg0.png, startup.png
└── art/                           # UI sprite assets
    ├── bg0.png                    # Emerald background texture
    ├── sd2vita.png                # SD2Vita hardware sprite
    ├── memorycard.png             # Official memory card sprite
    └── internal.png               # Internal eMMC sprite
```

---

## 🛠️ Building from Source

### Prerequisites
Install the official [VitaSDK](https://vitasdk.org/) toolchain and ensure it is available in your PATH:

```bash
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH
```

### Build VPK
```bash
make clean
make
```

Output: `VitaStorageHealth.vpk`

---

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
