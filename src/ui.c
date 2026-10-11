#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <vita2d.h>

static vita2d_pgf *font = NULL;

static vita2d_texture *bg_tex       = NULL;
static vita2d_texture *tex_sd2vita  = NULL;
static vita2d_texture *tex_memcard  = NULL;
static vita2d_texture *tex_internal = NULL;

// === Emerald Green Palette ===
#define COLOR_CLEAR       RGBA8(0, 0, 0, 0)
#define COLOR_BG          RGBA8(9, 14, 12, 255)
#define COLOR_PANEL       RGBA8(16, 26, 21, 235)
#define COLOR_PANEL_DARK  RGBA8(10, 18, 14, 245)
#define COLOR_PANEL_BORDER RGBA8(32, 58, 45, 255)
#define COLOR_ACCENT      RGBA8(0, 230, 118, 255)
#define COLOR_TEXT        RGBA8(245, 255, 250, 255)
#define COLOR_SUBTEXT     RGBA8(140, 175, 155, 255)
#define COLOR_GOOD        RGBA8(0, 230, 118, 255)
#define COLOR_WARN        RGBA8(255, 179, 0, 255)
#define COLOR_BAD         RGBA8(255, 61, 0, 255)
#define COLOR_INFO        RGBA8(0, 176, 255, 255)
#define COLOR_ORANGE      RGBA8(255, 130, 0, 255)
#define COLOR_SEL_BG      RGBA8(0, 60, 38, 200)

// Tab layout constants
#define TAB_START_X   200
#define TAB_WIDTH     120
#define TAB_GAP       4

static vita2d_texture* load_tex(const char *path) {
    vita2d_texture *t = vita2d_load_PNG_file(path);
    if (!t) {
        char alt[256];
        snprintf(alt, sizeof(alt), "ux0:app/VSHL00001/%s", path);
        t = vita2d_load_PNG_file(alt);
    }
    return t;
}

void ui_init(AppState *app) {
    memset(app, 0, sizeof(AppState));
    app->current_view = VIEW_SUMMARY;

    storage_init(&app->storage_sys);
    tools_init(&app->cleaner_ctx, &app->tools_cfg, &app->repair_ctx);

    // Init plugin manager on the primary storage TAI folder
    plugins_init(&app->plugin_mgr, "ur0:");

    vita2d_init();
    vita2d_set_clear_color(COLOR_BG);
    font = vita2d_load_default_pgf();

    bg_tex       = load_tex("app0:art/bg0.png");
    tex_sd2vita  = load_tex("app0:art/sd2vita.png");
    tex_memcard  = load_tex("app0:art/memorycard.png");
    tex_internal = load_tex("app0:art/internal.png");
}

void ui_update(AppState *app, SceCtrlData *pad) {
    uint32_t btn = pad->buttons;
    uint32_t pressed = btn & ~app->button_pressed;
    app->button_pressed = btn;

    StoragePartition *sel = storage_get_selected(&app->storage_sys);

    // L / R Triggers: cycle tabs
    if ((pressed & SCE_CTRL_L1) || (pressed & SCE_CTRL_LTRIGGER)) {
        app->current_view = (AppView)((app->current_view - 1 + TOTAL_VIEWS) % TOTAL_VIEWS);
        // Re-init plugin manager when switching to plugin tab
        if (app->current_view == VIEW_PLUGINS && sel)
            plugins_init(&app->plugin_mgr, sel->mount_point);
    }
    if ((pressed & SCE_CTRL_R1) || (pressed & SCE_CTRL_RTRIGGER)) {
        app->current_view = (AppView)((app->current_view + 1) % TOTAL_VIEWS);
        if (app->current_view == VIEW_PLUGINS && sel)
            plugins_init(&app->plugin_mgr, sel->mount_point);
    }

    // D-Pad Left/Right: switch storage partition
    if (pressed & SCE_CTRL_LEFT) {
        if (app->storage_sys.count > 0) {
            app->storage_sys.selected_index = (app->storage_sys.selected_index - 1 + app->storage_sys.count) % app->storage_sys.count;
            sel = storage_get_selected(&app->storage_sys);
            if (app->current_view == VIEW_PLUGINS && sel)
                plugins_init(&app->plugin_mgr, sel->mount_point);
        }
    }
    if (pressed & SCE_CTRL_RIGHT) {
        if (app->storage_sys.count > 0) {
            app->storage_sys.selected_index = (app->storage_sys.selected_index + 1) % app->storage_sys.count;
            sel = storage_get_selected(&app->storage_sys);
            if (app->current_view == VIEW_PLUGINS && sel)
                plugins_init(&app->plugin_mgr, sel->mount_point);
        }
    }

    // D-Pad Up/Down: navigate plugin list
    if (app->current_view == VIEW_PLUGINS) {
        if (pressed & SCE_CTRL_UP)   plugins_navigate(&app->plugin_mgr, -1);
        if (pressed & SCE_CTRL_DOWN) plugins_navigate(&app->plugin_mgr,  1);
    }

    // CROSS (X): per-view actions
    if (pressed & SCE_CTRL_CROSS) {
        if (app->current_view == VIEW_PLUGINS) {
            // Toggle selected plugin in config
            plugins_toggle_in_config(&app->plugin_mgr, app->plugin_mgr.selected_index);
        } else if (app->current_view == VIEW_TOOLS) {
            tools_repair_unlink_id_dat(&app->repair_ctx);
            storage_refresh(&app->storage_sys);
        } else if (sel && !sel->bench_ctx.is_running) {
            app->current_view = VIEW_BENCHMARK;
            benchmark_run_full(&sel->bench_ctx, sel, 32 * 1024 * 1024);
        }
    }

    // TRIANGLE: scan / export
    if (pressed & SCE_CTRL_TRIANGLE) {
        if (app->current_view == VIEW_TOOLS) {
            tools_export_report(&app->storage_sys, &sel->bench_ctx, &sel->integrity_ctx.scan, &app->plugin_mgr, &app->tools_cfg);
        } else if (app->current_view == VIEW_PLUGINS) {
            // Re-scan plugins
            if (sel) plugins_init(&app->plugin_mgr, sel->mount_point);
            plugins_scan(&app->plugin_mgr);
        } else if (sel && !sel->integrity_ctx.is_scanning) {
            app->current_view = VIEW_INTEGRITY_SCAN;
            integrity_run_scan(&sel->integrity_ctx, sel, true);
        }
    }

    // SQUARE: purge junk & locks (tools tab)
    if (pressed & SCE_CTRL_SQUARE) {
        if (app->current_view == VIEW_TOOLS) {
            tools_scan_junk(&app->cleaner_ctx);
            tools_purge_junk(&app->cleaner_ctx);
            tools_repair_filesystem_locks(&app->repair_ctx);
            storage_refresh(&app->storage_sys);
        }
    }

    // SELECT: live re-detect storage partitions and driver configuration
    if (pressed & SCE_CTRL_SELECT) {
        storage_refresh(&app->storage_sys);
        tools_detect_driver_config(&app->tools_cfg);
        tools_diagnose_pc_compatibility(&app->storage_sys, &app->tools_cfg, &app->repair_ctx);
    }
}

// ──────────────────────────────────────────────────────────
//  HEADER
// ──────────────────────────────────────────────────────────
static void draw_header(AppState *app) {
    vita2d_draw_rectangle(0, 0, 960, 52, COLOR_PANEL);
    vita2d_draw_rectangle(0, 51, 960, 1, COLOR_PANEL_BORDER);

    if (font) {
        vita2d_pgf_draw_text(font, 14, 33, COLOR_TEXT, 1.1f, "VitaStorageHealth");
    }

    // Tabs – short labels, fixed width so they can't overflow
    for (int i = 0; i < TOTAL_VIEWS; i++) {
        bool active = (app->current_view == (AppView)i);
        unsigned int bg   = active ? RGBA8(18, 40, 28, 255) : RGBA8(12, 18, 15, 190);
        unsigned int col  = active ? COLOR_ACCENT : COLOR_SUBTEXT;
        int tx = TAB_START_X + i * (TAB_WIDTH + TAB_GAP);
        vita2d_draw_rectangle(tx, 7, TAB_WIDTH, 37, bg);
        if (active) vita2d_draw_rectangle(tx, 43, TAB_WIDTH, 3, COLOR_ACCENT);
        if (font)
            vita2d_pgf_draw_text(font, tx + 8, 31, col, 0.88f, TAB_LABELS[i]);
    }

    // Mount tag – right-aligned, safe distance from tabs
    StoragePartition *sel = storage_get_selected(&app->storage_sys);
    if (sel && font) {
        char tag[20];
        snprintf(tag, sizeof(tag), "[%s]", sel->mount_point);
        vita2d_pgf_draw_text(font, 900, 33, COLOR_ACCENT, 1.0f, tag);
    }
}

// ──────────────────────────────────────────────────────────
//  FOOTER
// ──────────────────────────────────────────────────────────
static void draw_footer(AppView view) {
    vita2d_draw_rectangle(0, 500, 960, 44, COLOR_PANEL);
    vita2d_draw_rectangle(0, 500, 960, 1, COLOR_PANEL_BORDER);
    if (!font) return;

    const char *left_hints = "[L/R] Tab   [D-PAD] Partition   [SELECT] Re-detect   [X] Speed Test   [△] Scan";
    const char *tools_hints = "[L/R] Tab   [D-PAD] Partition   [X] Unlink id.dat   [□] Purge Junk   [△] Export Report";
    const char *plugin_hints = "[L/R] Tab   [D-PAD] Partition   [▲▼] Select   [X] Toggle   [△] Re-scan";

    const char *hint = left_hints;
    if (view == VIEW_TOOLS)   hint = tools_hints;
    if (view == VIEW_PLUGINS) hint = plugin_hints;

    vita2d_pgf_draw_text(font, 14, 527, COLOR_SUBTEXT, 0.83f, hint);
}

// ──────────────────────────────────────────────────────────
//  VIEW: SUMMARY
// ──────────────────────────────────────────────────────────
static void render_summary_view(AppState *app) {
    StoragePartition *sel = storage_get_selected(&app->storage_sys);
    if (!sel) return;

    // Top storage card
    vita2d_draw_rectangle(38, 66, 884, 168, COLOR_PANEL);
    vita2d_draw_rectangle(38, 66, 884, 168, COLOR_PANEL_BORDER);

    vita2d_texture *icon = NULL;
    switch (sel->type) {
        case STORAGE_TYPE_SD2VITA:      icon = tex_sd2vita;  break;
        case STORAGE_TYPE_VITA_MEMCARD: icon = tex_memcard;  break;
        case STORAGE_TYPE_INTERNAL_EMMC: icon = tex_internal; break;
        default: icon = tex_internal; break;
    }
    if (icon) vita2d_draw_texture_scale(icon, 56, 82, 0.88f, 0.88f);

    int tx = icon ? 184 : 60;
    if (font) {
        vita2d_pgf_draw_text(font, tx, 104, COLOR_TEXT,    1.3f,  sel->label);
        if (sel->is_boot_device) {
            vita2d_draw_rectangle(tx + 360, 86, 260, 22, RGBA8(0, 50, 30, 220));
            vita2d_pgf_draw_text(font, tx + 368, 102, COLOR_GOOD, 0.76f, "🔒 BOOT DEVICE (SYSTEM PROTECTED)");
        } else if (sel->is_system_internal) {
            vita2d_draw_rectangle(tx + 360, 86, 220, 22, RGBA8(0, 35, 60, 220));
            vita2d_pgf_draw_text(font, tx + 368, 102, COLOR_INFO, 0.76f, "🔒 INTERNAL OS (UR0)");
        }
        char hw[64];
        snprintf(hw, sizeof(hw), "Hardware: %s", storage_type_to_string(sel->type));
        vita2d_pgf_draw_text(font, tx, 130, COLOR_SUBTEXT, 0.92f, hw);

        float fill = (sel->total_bytes > 0) ? ((float)sel->used_bytes / (float)sel->total_bytes) : 0.0f;
        int bar_w = 880 - (tx - 38) - 32;
        vita2d_draw_rectangle(tx, 148, bar_w, 20, RGBA8(22, 36, 28, 255));
        vita2d_draw_rectangle(tx, 148, (int)(bar_w * fill), 20, COLOR_ACCENT);

        char fb[32], tb[32];
        storage_format_size(sel->free_bytes, fb, sizeof(fb));
        storage_format_size(sel->total_bytes, tb, sizeof(tb));
        char sz[96];
        snprintf(sz, sizeof(sz), "Free: %s / Total: %s", fb, tb);
        vita2d_pgf_draw_text(font, tx, 200, COLOR_TEXT, 0.9f, sz);
    }

    // Health panel (left)
    vita2d_draw_rectangle(38, 252, 430, 230, COLOR_PANEL);
    vita2d_draw_rectangle(38, 252, 430, 230, COLOR_PANEL_BORDER);
    if (font) {
        vita2d_pgf_draw_text(font, 56, 285, COLOR_TEXT, 1.05f, "Storage Lifespan & Health");
        char hs[24];
        snprintf(hs, sizeof(hs), "%.1f %%", sel->health_score);
        unsigned int hcol = (sel->health_score > 90.0) ? COLOR_GOOD : ((sel->health_score > 70.0) ? COLOR_WARN : COLOR_BAD);
        vita2d_pgf_draw_text(font, 56, 345, hcol, 2.1f, hs);
        vita2d_pgf_draw_text(font, 56, 385, COLOR_GOOD,    0.9f, "Status: Healthy Controller");
        vita2d_pgf_draw_text(font, 56, 410, COLOR_SUBTEXT, 0.9f, "Bad Sectors Detected: 0");
    }

    // Driver config panel (right) – no control hints here
    vita2d_draw_rectangle(488, 252, 434, 230, COLOR_PANEL);
    vita2d_draw_rectangle(488, 252, 434, 230, COLOR_PANEL_BORDER);
    if (font) {
        vita2d_pgf_draw_text(font, 506, 285, COLOR_TEXT, 1.05f, "System Driver Configuration");
        char drv[80];
        snprintf(drv, sizeof(drv), "Driver: %s", app->tools_cfg.driver_name);
        vita2d_pgf_draw_text(font, 506, 325, COLOR_ACCENT, 1.0f, drv);
        char ux[80];
        snprintf(ux, sizeof(ux), "UX0 target: %s", app->tools_cfg.ux0_target);
        vita2d_pgf_draw_text(font, 506, 360, COLOR_SUBTEXT, 0.9f, ux);
        char um[80];
        snprintf(um, sizeof(um), "UMA0 target: %s", app->tools_cfg.uma0_target);
        vita2d_pgf_draw_text(font, 506, 390, COLOR_SUBTEXT, 0.9f, um);
        vita2d_pgf_draw_text(font, 506, 430, COLOR_SUBTEXT, 0.85f,
            app->tools_cfg.yamt_active ? "YAMT kernel driver is active" : "Using stock / StorageMgr driver");
    }
}

// ──────────────────────────────────────────────────────────
//  VIEW: BENCHMARK (per-partition)
// ──────────────────────────────────────────────────────────
static void render_benchmark_view(AppState *app) {
    StoragePartition *sel = storage_get_selected(&app->storage_sys);
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL);
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL_BORDER);
    if (!font) return;

    vita2d_pgf_draw_text(font, 56, 100, COLOR_TEXT, 1.25f, "Read & Write Speed Benchmark");

    if (!sel) {
        vita2d_pgf_draw_text(font, 56, 150, COLOR_WARN, 1.0f, "No partition selected.");
        return;
    }

    BenchmarkContext *b = &sel->bench_ctx;

    if (b->is_running) {
        vita2d_pgf_draw_text(font, 56, 148, COLOR_WARN, 1.05f, b->status_msg);
        vita2d_draw_rectangle(56, 170, 820, 22, RGBA8(20, 36, 28, 255));
        vita2d_draw_rectangle(56, 170, (int)(820.0f * b->progress), 22, COLOR_ACCENT);
    } else {
        char v[64];
        vita2d_pgf_draw_text(font, 56, 168, COLOR_SUBTEXT, 1.05f, "Sequential Write:");
        snprintf(v, sizeof(v), "%.2f MB/s", b->results.seq_write_mbps);
        vita2d_pgf_draw_text(font, 360, 168, COLOR_ACCENT, 1.25f, v);

        vita2d_pgf_draw_text(font, 56, 228, COLOR_SUBTEXT, 1.05f, "Sequential Read:");
        snprintf(v, sizeof(v), "%.2f MB/s", b->results.seq_read_mbps);
        vita2d_pgf_draw_text(font, 360, 228, COLOR_GOOD, 1.25f, v);

        vita2d_pgf_draw_text(font, 56, 288, COLOR_SUBTEXT, 1.05f, "Random 4KB IOPS:");
        snprintf(v, sizeof(v), "%.0f IOPS", b->results.rand_read_4k_iops);
        vita2d_pgf_draw_text(font, 360, 288, COLOR_WARN, 1.25f, v);

        vita2d_pgf_draw_text(font, 56, 348, COLOR_SUBTEXT, 1.05f, "Access Latency:");
        snprintf(v, sizeof(v), "%.2f ms", b->results.avg_latency_ms);
        vita2d_pgf_draw_text(font, 360, 348, COLOR_TEXT, 1.25f, v);

        if (!b->is_finished) {
            vita2d_pgf_draw_text(font, 56, 408, COLOR_SUBTEXT, 0.88f,
                "Press [X] to run speed test on the selected partition.");
        }
    }
}

// ──────────────────────────────────────────────────────────
//  VIEW: INTEGRITY (per-partition)
// ──────────────────────────────────────────────────────────
static void render_integrity_view(AppState *app) {
    StoragePartition *sel = storage_get_selected(&app->storage_sys);
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL);
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL_BORDER);
    if (!font || !sel) return;

    IntegrityContext *ic = &sel->integrity_ctx;

    vita2d_pgf_draw_text(font, 56, 100, COLOR_TEXT, 1.25f, "Sector Surface Heatmap & Fake Card Detector");
    vita2d_pgf_draw_text(font, 56, 130, COLOR_SUBTEXT, 0.9f, ic->status_text);

    // 10x10 grid
    int gx = 56, gy = 148, cs = 28, gap = 4;
    for (int i = 0; i < GRID_SIZE; i++) {
        int c = i % 10, r = i / 10;
        unsigned int col;
        switch (ic->scan.grid[i]) {
            case SECTOR_GOOD:    col = COLOR_GOOD; break;
            case SECTOR_SLOW:    col = COLOR_WARN; break;
            case SECTOR_CORRUPT: col = COLOR_BAD;  break;
            default:             col = RGBA8(28, 42, 34, 255); break;
        }
        vita2d_draw_rectangle(gx + c * (cs+gap), gy + r * (cs+gap), cs, cs, col);
    }

    // Stats
    int sx = 430;
    vita2d_pgf_draw_text(font, sx, 165, COLOR_TEXT, 1.05f, "Sector Key:");
    vita2d_draw_rectangle(sx, 188, 14, 14, COLOR_GOOD); vita2d_pgf_draw_text(font, sx+22, 200, COLOR_SUBTEXT, 0.88f, "Good");
    vita2d_draw_rectangle(sx, 214, 14, 14, COLOR_WARN); vita2d_pgf_draw_text(font, sx+22, 226, COLOR_SUBTEXT, 0.88f, "Slow / Degraded");
    vita2d_draw_rectangle(sx, 240, 14, 14, COLOR_BAD);  vita2d_pgf_draw_text(font, sx+22, 252, COLOR_SUBTEXT, 0.88f, "Corrupt / Fake");

    char buf[64];
    snprintf(buf, sizeof(buf), "Good: %u  Slow: %u  Bad: %u", ic->scan.good_count, ic->scan.slow_count, ic->scan.bad_count);
    vita2d_pgf_draw_text(font, sx, 292, COLOR_TEXT, 0.92f, buf);

    if (ic->scan.fake_card_detected)
        vita2d_pgf_draw_text(font, sx, 332, COLOR_BAD,  1.05f, "ALERT: Fake Capacity Detected!");
    else if (ic->is_complete)
        vita2d_pgf_draw_text(font, sx, 332, COLOR_GOOD, 1.05f, "VERIFIED: Genuine Capacity!");

    if (!ic->is_scanning && !ic->is_complete)
        vita2d_pgf_draw_text(font, sx, 380, COLOR_SUBTEXT, 0.88f, "Press [△] to run surface scan.");
}

// ──────────────────────────────────────────────────────────
//  VIEW: TOOLS
// ──────────────────────────────────────────────────────────
static void render_tools_view(AppState *app) {
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL);
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL_BORDER);
    if (!font) return;

    vita2d_pgf_draw_text(font, 56, 96, COLOR_TEXT, 1.15f, "Storage Utilities, Maintenance & Repair Suite");

    const char *status_str = (app->repair_ctx.status_msg[0] != '\0') ? app->repair_ctx.status_msg : app->cleaner_ctx.status_msg;
    vita2d_pgf_draw_text(font, 56, 122, COLOR_SUBTEXT, 0.85f, status_str);

    // --- Card 1 (Top Left): Cache & Lock Purger ---
    vita2d_draw_rectangle(54, 134, 422, 158, COLOR_PANEL_DARK);
    vita2d_draw_rectangle(54, 134, 422, 158, COLOR_PANEL_BORDER);
    vita2d_pgf_draw_text(font, 70, 160, COLOR_TEXT, 1.0f, "Cache & Temp File Purger");
    char rb[48]; storage_format_size(app->cleaner_ctx.total_reclaimable_bytes, rb, sizeof(rb));
    char rs[96]; snprintf(rs, sizeof(rs), "Reclaimable Cache: %s", rb);
    vita2d_pgf_draw_text(font, 70, 188, COLOR_SUBTEXT, 0.84f, rs);
    if (app->cleaner_ctx.clean_completed) {
        char cs_buf[64];
        snprintf(cs_buf, sizeof(cs_buf), "Cleaned %u files and temporary locks!", app->cleaner_ctx.total_files_cleaned);
        vita2d_pgf_draw_text(font, 70, 216, COLOR_GOOD, 0.84f, cs_buf);
    } else {
        vita2d_pgf_draw_text(font, 70, 216, COLOR_SUBTEXT, 0.82f, "Removes stale benchmarks, logs & download trash");
    }
    vita2d_pgf_draw_text(font, 70, 260, COLOR_ACCENT, 0.88f, "[SQUARE] Scan & Purge Junk Cache");

    // --- Card 2 (Top Right): Diagnostic & Plugin Exporter ---
    vita2d_draw_rectangle(488, 134, 420, 158, COLOR_PANEL_DARK);
    vita2d_draw_rectangle(488, 134, 420, 158, COLOR_PANEL_BORDER);
    vita2d_pgf_draw_text(font, 504, 160, COLOR_TEXT, 1.0f, "Full Diagnostic & Plugin Exporter");
    vita2d_pgf_draw_text(font, 504, 188, COLOR_SUBTEXT, 0.84f, "Exports speed, integrity, and taiHEN plugins");
    if (app->tools_cfg.report_saved) {
        vita2d_pgf_draw_text(font, 504, 216, COLOR_GOOD, 0.82f, "Saved: ux0:data/VitaStorageHealth/report.txt");
    } else {
        vita2d_pgf_draw_text(font, 504, 216, COLOR_SUBTEXT, 0.82f, "Comprehensive log with plugin status audits");
    }
    vita2d_pgf_draw_text(font, 504, 260, COLOR_ACCENT, 0.88f, "[TRIANGLE] Export Full Report");

    // --- Card 3 (Bottom Left): PC Detection Doctor ---
    vita2d_draw_rectangle(54, 304, 422, 162, COLOR_PANEL_DARK);
    vita2d_draw_rectangle(54, 304, 422, 162, COLOR_PANEL_BORDER);
    vita2d_pgf_draw_text(font, 70, 330, COLOR_TEXT, 1.0f, "PC Detection & Health Doctor");
    vita2d_pgf_draw_text(font, 70, 356, COLOR_WARN, 0.86f, app->repair_ctx.pc_compat_title);
    vita2d_pgf_draw_text(font, 70, 382, COLOR_SUBTEXT, 0.78f, "Why PC fails: Raw texFAT has no MBR partition table.");
    vita2d_pgf_draw_text(font, 70, 404, COLOR_SUBTEXT, 0.78f, "Windows USB card readers refuse unpartitioned media.");
    vita2d_pgf_draw_text(font, 70, 436, COLOR_GOOD, 0.84f, "Fix: Use VitaShell USB [SELECT] to transfer!");

    // --- Card 4 (Bottom Right): Database & Card Unlink Tool ---
    vita2d_draw_rectangle(488, 304, 420, 162, COLOR_PANEL_DARK);
    vita2d_draw_rectangle(488, 304, 420, 162, COLOR_PANEL_BORDER);
    vita2d_pgf_draw_text(font, 504, 330, COLOR_TEXT, 1.0f, "Database & Card Unlink Tool");
    vita2d_pgf_draw_text(font, 504, 356, COLOR_SUBTEXT, 0.82f, "Fix missing game icons & account mismatch locks");
    if (app->repair_ctx.id_dat_present) {
        vita2d_pgf_draw_text(font, 504, 382, COLOR_INFO, 0.82f, "Status: ux0:id.dat active (Bound to console)");
    } else {
        vita2d_pgf_draw_text(font, 504, 382, COLOR_GOOD, 0.82f, "Status: id.dat unlinked (Will rebuild on reboot)");
    }
    vita2d_pgf_draw_text(font, 504, 404, COLOR_SUBTEXT, 0.78f, "Safely rebuilds app.db without deleting game files");
    vita2d_pgf_draw_text(font, 504, 436, COLOR_ACCENT, 0.88f, "[CROSS] Unlink ux0:id.dat (Rebuild DB)");
}

// ──────────────────────────────────────────────────────────
//  VIEW: PLUGINS
// ──────────────────────────────────────────────────────────
static void render_plugins_view(AppState *app) {
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL);
    vita2d_draw_rectangle(38, 66, 884, 416, COLOR_PANEL_BORDER);
    if (!font) return;

    PluginManager *pm = &app->plugin_mgr;

    // Title row
    char title[64];
    snprintf(title, sizeof(title), "TaiHEN Plugin Manager — %s", pm->tai_base);
    vita2d_pgf_draw_text(font, 56, 98, COLOR_TEXT, 1.1f, title);
    vita2d_pgf_draw_text(font, 56, 124, COLOR_SUBTEXT, 0.86f, pm->status_msg);

    // Restart banner
    if (pm->restart_banner) {
        vita2d_draw_rectangle(56, 134, 884, 24, RGBA8(60, 20, 0, 220));
        vita2d_pgf_draw_text(font, 70, 150, COLOR_WARN, 0.9f,
            "⚠  Restart your PS Vita to apply config changes.");
    }

    int list_y = pm->restart_banner ? 166 : 144;
    int row_h = 56;
    int visible_rows = (494 - list_y) / row_h; // how many fit in panel

    if (!pm->scan_complete) {
        vita2d_pgf_draw_text(font, 56, 230, COLOR_SUBTEXT, 0.95f,
            "Press [TRIANGLE] to scan the tai folder for plugins.");
        return;
    }
    if (pm->count == 0) {
        vita2d_pgf_draw_text(font, 56, 230, COLOR_WARN, 0.95f,
            "No .skprx plugin files found.");
        return;
    }

    // Calculate scroll window
    int start_idx = pm->selected_index - visible_rows / 2;
    if (start_idx < 0) start_idx = 0;
    if (start_idx > pm->count - visible_rows) start_idx = pm->count - visible_rows;
    if (start_idx < 0) start_idx = 0;

    for (int i = start_idx; i < pm->count && i < start_idx + visible_rows; i++) {
        PluginEntry *e = &pm->plugins[i];
        bool is_selected = (i == pm->selected_index);
        int ry = list_y + (i - start_idx) * row_h;

        // Row background
        if (is_selected) {
            vita2d_draw_rectangle(56, ry, 884, row_h - 2, COLOR_SEL_BG);
            vita2d_draw_rectangle(56, ry, 3, row_h - 2, COLOR_ACCENT);
        }

        // Status badge
        unsigned int badge_col;
        const char *badge_txt;
        bool is_critical = (strcasecmp(e->filename, "yamt.skprx") == 0 ||
                            strcasecmp(e->filename, "storagemgr.skprx") == 0);
        if (is_critical && e->in_config) {
            badge_col = COLOR_WARN;
            badge_txt = "BOOT LOCK";
        } else {
            switch (e->status) {
                case PLUGIN_STATUS_ACTIVE:
                    badge_col = COLOR_GOOD; badge_txt = "ACTIVE"; break;
                case PLUGIN_STATUS_UNREGISTERED:
                    badge_col = COLOR_SUBTEXT; badge_txt = "NOT IN CFG"; break;
                case PLUGIN_STATUS_ORPHANED:
                    badge_col = COLOR_WARN; badge_txt = "ORPHANED"; break;
                default:
                    badge_col = COLOR_SUBTEXT; badge_txt = "UNKNOWN"; break;
            }
        }

        vita2d_draw_rectangle(68, ry + 8, 88, 18, RGBA8(12, 20, 16, 200));
        vita2d_pgf_draw_text(font, 72, ry + 22, badge_col, 0.78f, badge_txt);

        // Filename
        vita2d_pgf_draw_text(font, 170, ry + 22, is_selected ? COLOR_TEXT : RGBA8(200, 230, 210, 255), 1.0f, e->filename);

        // Section tag (e.g. *KERNEL)
        if (e->in_config && font) {
            vita2d_pgf_draw_text(font, 590, ry + 22, COLOR_INFO, 0.82f, e->section);
        }

        // Version
        vita2d_pgf_draw_text(font, 780, ry + 22, COLOR_SUBTEXT, 0.78f, e->version);

        // Description + author (selected row only)
        if (is_selected) {
            char meta[160];
            snprintf(meta, sizeof(meta), "%s  |  by %s", e->description, e->author);
            vita2d_pgf_draw_text(font, 170, ry + 42, COLOR_SUBTEXT, 0.78f, meta);
        }
    }

    // Scroll indicator
    if (pm->count > visible_rows && font) {
        char sc[32];
        snprintf(sc, sizeof(sc), "%d / %d", pm->selected_index + 1, pm->count);
        vita2d_pgf_draw_text(font, 840, 488, COLOR_SUBTEXT, 0.8f, sc);
    }
}

// ──────────────────────────────────────────────────────────
//  RENDER MAIN
// ──────────────────────────────────────────────────────────
void ui_render(AppState *app) {
    vita2d_start_drawing();
    vita2d_clear_screen();

    if (bg_tex) {
        vita2d_draw_texture_scale(bg_tex, 0, 0, 960.0f / 840.0f, 544.0f / 500.0f);
        vita2d_draw_rectangle(0, 0, 960, 544, RGBA8(9, 16, 12, 185));
    }

    draw_header(app);

    switch (app->current_view) {
        case VIEW_SUMMARY:        render_summary_view(app);    break;
        case VIEW_BENCHMARK:      render_benchmark_view(app);  break;
        case VIEW_INTEGRITY_SCAN: render_integrity_view(app);  break;
        case VIEW_TOOLS:          render_tools_view(app);      break;
        case VIEW_PLUGINS:        render_plugins_view(app);    break;
        default:                  render_summary_view(app);    break;
    }

    draw_footer(app->current_view);

    vita2d_end_drawing();
    vita2d_swap_buffers();
}

void ui_cleanup(void) {
    if (bg_tex)       vita2d_free_texture(bg_tex);
    if (tex_sd2vita)  vita2d_free_texture(tex_sd2vita);
    if (tex_memcard)  vita2d_free_texture(tex_memcard);
    if (tex_internal) vita2d_free_texture(tex_internal);
    if (font)         vita2d_free_pgf(font);
    vita2d_fini();
}
