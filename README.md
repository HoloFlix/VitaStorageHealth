# VitaStorageHealth 🎮💾

**VitaStorageHealth** is an all-in-one homebrew storage utility for the PlayStation Vita (PCH-1000 / PCH-2000 / PS TV). It provides deep hardware inspection, real-time read/write throughput benchmarks, sector surface health scanning, and fake microSD card capacity detection.

---

## 🌟 Key Features

1. **Multi-Storage Mount Inspector:**
   - Detects and queries partition statistics across all PS Vita mount points:
     - `ux0:` (Primary storage / SD2Vita / Memory Card)
     - `ur0:` (Internal system partition eMMC)
     - `imc0:` (Internal 1GB memory on PS Vita 2000 Slim & PS TV)
     - `uma0:` (USB storage / secondary Memory Card / SD2Vita slot)
     - `xmc0:`, `gro0:`, `grw0:` (Game Card read/write partitions)
   - Displays real-time Free vs. Total space, file system allocation, and estimated flash memory wear/health score.

2. **Speed Benchmarking Engine (Sequential & Random 4K IOPS):**
   - **Sequential Read / Write:** Measures throughput in **MB/s** using safe pattern-buffered temporary payloads.
   - **Random 4KB IOPS:** Tests micro-read/write performance to gauge homebrew app launch speeds and game asset loading performance.
   - **Access Latency:** Measures hardware response times in milliseconds (**ms**).

3. **Surface Heatmap & Fake Card Detector (H2testw-style):**
   - **Sector Visualizer:** Renders a 10x10 colored sector grid displaying healthy, degrading (slow), or bad blocks.
   - **Fake Capacity Check:** Verifies written blocks using CRC32 checksum verification to catch fake counterfeit SD cards (e.g. spoofed 512GB cards that fail past 32GB).
   - **Safe Testing:** Only operates on temporary payload files (`.vsh_speedtest.tmp`, `.vsh_integrity.tmp`) and cleans up instantly upon completion.

4. **Modern Vita2D GUI:**
   - Native 960x544 dark OLED theme designed for Vita controls (D-Pad, Cross, Triangle, Square, Start).

---

## 📁 Project Architecture

```
d:/AntiGrav_Projects/VitaStorageHealth/
├── Makefile                # VitaSDK GNU Makefile
├── CMakeLists.txt          # CMake Build Script
├── README.md               # Documentation
├── idea.txt                # Original conceptual design outline
├── src/
│   ├── main.c              # Main loop & Vita system initialization
│   ├── storage.h / .c      # Mount point detection & filesystem space query
│   ├── benchmark.h / .c    # Sequential & Random 4K IOPS speed test engine
│   ├── integrity.h / .c    # Bad sector visualizer & CRC32 fake card verifier
│   └── ui.h / .c           # Vita2D GUI renderer & Vita controls handling
└── preview/
    └── index.html          # Interactive HTML5 PS Vita OLED UI Simulator
```

---

## 🛠️ Building with VitaSDK

### Prerequisites
Make sure you have the official [vitasdk toolchain](https://vitasdk.org/) installed and set in your environment path.

```bash
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH
```

### Option 1: Using Makefile
```bash
make
```
This produces `VitaStorageHealth.vpk` in the root folder.

### Option 2: Using CMake
```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake ..
make
```

---

## 🚀 Installation & Usage on PS Vita

1. Copy `VitaStorageHealth.vpk` to your PS Vita via VitaShell (FTP or USB).
2. Install the `.vpk` on your Vita.
3. Launch **VitaStorageHealth** from the LiveArea.
4. **Controls:**
   - `[D-PAD ◄ / ►]`: Switch between active partitions (`ux0:`, `ur0:`, `imc0:`, `uma0:`).
   - `[CROSS (X)]`: Run Speed Benchmark.
   - `[TRIANGLE (△)]`: Run Surface Sector Scan & Fake Card Check.
   - `[START]`: Cycle views (Summary / Benchmark / Surface Grid).

---

## 🌐 Interactive Browser UI Simulator

You can open `preview/index.html` directly in any web browser to preview and test the complete PS Vita OLED interface, speed test animations, fake card simulation, and D-Pad controls!
