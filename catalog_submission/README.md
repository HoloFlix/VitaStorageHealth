# NeoVitaDB Catalog Submission Guide

This folder contains the prepared submission files for adding **Vita Storage Health** to the [NeoVitaDB Catalog](https://github.com/robin994/NeoVitaDB-Catalog-Test).

---

## Included Submission Files

- `apps/vita/1939-vita-storage-health.json` — Metadata catalog entry (Validated against `app.schema.json`).
- `icons_vita/1939-vita-storage-health.png` — 128x128 8-bit indexed catalog icon.

---

## How to Submit to NeoVitaDB

### Step 1: Make your GitHub Repository Public (Required)
The NeoVitaDB build bot (`tools/build_catalog.py`) uses the public GitHub API to fetch release assets and compute checksums:
1. Go to your repository settings: `https://github.com/HoloFlix/VitaStorageHealth/settings`
2. Scroll to the **Danger Zone** and change repository visibility to **Public**.

### Step 2: Create a GitHub Release
1. In your repository, click **Releases** → **Draft a new release**.
2. Tag: `v1.0.0`
3. Release Title: `Vita Storage Health v1.0.0`
4. Attach `VitaStorageHealth.vpk` as an asset.
5. Publish release.

### Step 3: Fork and Submit PR
1. Go to [https://github.com/robin994/NeoVitaDB-Catalog-Test](https://github.com/robin994/NeoVitaDB-Catalog-Test) and click **Fork**.
2. Copy these two files into your fork:
   - `catalog_submission/apps/vita/1939-vita-storage-health.json` → `apps/vita/1939-vita-storage-health.json`
   - `catalog_submission/icons_vita/1939-vita-storage-health.png` → `icons_vita/1939-vita-storage-health.png`
3. Commit the changes and open a **Pull Request** to `robin994/NeoVitaDB-Catalog-Test` (branch `main`).
4. Title: `Add Vita Storage Health (1939)`
5. Once merged by the maintainer, Vita Storage Health will appear automatically in the NeoVitaDB Downloader on PS Vita consoles!
