#ifndef UI_H
#define UI_H

#include "storage.h"
#include "tools.h"
#include "plugins.h"
#include <psp2/ctrl.h>

typedef enum {
    VIEW_SUMMARY = 0,
    VIEW_BENCHMARK,
    VIEW_INTEGRITY_SCAN,
    VIEW_TOOLS,
    VIEW_PLUGINS
} AppView;

#define TOTAL_VIEWS 5

// Short tab labels that fit within the header without overflow
static const char* const TAB_LABELS[TOTAL_VIEWS] = {
    "OVERVIEW",
    "BENCHMARK",
    "SCAN",
    "TOOLS",
    "PLUGINS"
};

typedef struct {
    AppView current_view;
    StorageSystem storage_sys;
    CleanerContext cleaner_ctx;
    StorageToolsConfig tools_cfg;
    RepairContext repair_ctx;
    PluginManager plugin_mgr;
    uint32_t button_pressed;
    bool quit_requested;
} AppState;

void ui_init(AppState *app);
void ui_update(AppState *app, SceCtrlData *pad);
void ui_render(AppState *app);
void ui_cleanup(void);

#endif // UI_H
