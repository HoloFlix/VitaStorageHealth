#ifndef PLUGINS_H
#define PLUGINS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PLUGINS 32
#define PLUGIN_NAME_LEN 64
#define PLUGIN_PATH_LEN 128
#define PLUGIN_INFO_LEN 128

typedef enum {
    PLUGIN_STATUS_UNKNOWN = 0,
    PLUGIN_STATUS_ACTIVE,      // Present on disk AND in config
    PLUGIN_STATUS_ORPHANED,    // In config but NOT found on disk
    PLUGIN_STATUS_UNREGISTERED // Found on disk but NOT in config
} PluginStatus;

typedef struct {
    char filename[PLUGIN_NAME_LEN];       // e.g. "yamt.skprx"
    char full_path[PLUGIN_PATH_LEN];      // e.g. "ur0:tai/yamt.skprx"
    char config_path[PLUGIN_PATH_LEN];    // e.g. "ur0:tai/config.txt"
    char section[32];                     // e.g. "*ALL", "*KERNEL"
    PluginStatus status;
    bool in_config;
    bool on_disk;
    bool restart_required;                // Always true when toggling
    // Optional metadata parsed from embedded comments
    char author[64];
    char description[PLUGIN_INFO_LEN];
    char version[16];
} PluginEntry;

typedef struct {
    PluginEntry plugins[MAX_PLUGINS];
    int count;
    int selected_index;
    bool is_scanning;
    bool scan_complete;
    char tai_base[32];       // e.g. "ur0:tai" or "ux0:tai"
    char config_file[PLUGIN_PATH_LEN];
    char status_msg[128];
    bool restart_banner;     // Show "Restart to apply" message
} PluginManager;

void plugins_init(PluginManager *mgr, const char *mount_point);
void plugins_scan(PluginManager *mgr);
void plugins_refresh_config(PluginManager *mgr);
bool plugins_toggle_in_config(PluginManager *mgr, int plugin_index);
void plugins_navigate(PluginManager *mgr, int dir);
PluginEntry* plugins_get_selected(PluginManager *mgr);
const char* plugins_status_string(PluginStatus status);

#endif // PLUGINS_H
