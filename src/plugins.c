#include "plugins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>

#include "plugins_db.h"

static void lookup_known_plugin(PluginEntry *entry) {
    for (int i = 0; i < KNOWN_PLUGINS_COUNT; i++) {
        if (strcasecmp(entry->filename, KNOWN_PLUGINS[i].name) == 0) {
            strncpy(entry->author,      KNOWN_PLUGINS[i].author,  sizeof(entry->author) - 1);
            strncpy(entry->description, KNOWN_PLUGINS[i].desc,    sizeof(entry->description) - 1);
            strncpy(entry->version,     KNOWN_PLUGINS[i].version, sizeof(entry->version) - 1);
            return;
        }
    }
    strncpy(entry->author,      "Unknown Author",              sizeof(entry->author) - 1);
    strncpy(entry->description, "No description available",    sizeof(entry->description) - 1);
    strncpy(entry->version,     "?.x",                        sizeof(entry->version) - 1);
}

void plugins_init(PluginManager *mgr, const char *mount_point) {
    memset(mgr, 0, sizeof(PluginManager));
    snprintf(mgr->tai_base, sizeof(mgr->tai_base), "%stai", mount_point);
    snprintf(mgr->config_file, sizeof(mgr->config_file), "%stai/config.txt", mount_point);
    snprintf(mgr->status_msg, sizeof(mgr->status_msg), "Press [TRIANGLE] to scan for plugins.");
}

void plugins_scan(PluginManager *mgr) {
    mgr->is_scanning = true;
    mgr->scan_complete = false;
    mgr->count = 0;
    mgr->restart_banner = false;
    snprintf(mgr->status_msg, sizeof(mgr->status_msg), "Scanning %s for .skprx plugins...", mgr->tai_base);

    // --- Step 1: Enumerate .skprx files from disk ---
    SceUID dir = sceIoDopen(mgr->tai_base);
    if (dir >= 0) {
        SceIoDirent dirent;
        while (sceIoDread(dir, &dirent) > 0 && mgr->count < MAX_PLUGINS) {
            const char *name = dirent.d_name;
            size_t len = strlen(name);
            bool is_skprx = (len > 6 && strcasecmp(name + len - 6, ".skprx") == 0);
            bool is_suprx = (len > 6 && strcasecmp(name + len - 6, ".suprx") == 0);
            if (is_skprx || is_suprx) {
                PluginEntry *e = &mgr->plugins[mgr->count];
                memset(e, 0, sizeof(PluginEntry));
                strncpy(e->filename, name, sizeof(e->filename) - 1);
                snprintf(e->full_path, sizeof(e->full_path), "%s/%s", mgr->tai_base, name);
                e->on_disk = true;
                e->in_config = false;
                e->status = PLUGIN_STATUS_UNREGISTERED;
                e->restart_required = true;
                lookup_known_plugin(e);
                mgr->count++;
            }
        }
        sceIoDclose(dir);
    }

    // --- Step 2: Parse config.txt and cross-reference ---
    plugins_refresh_config(mgr);

    mgr->is_scanning = false;
    mgr->scan_complete = true;

    if (mgr->count == 0) {
        snprintf(mgr->status_msg, sizeof(mgr->status_msg), "No .skprx plugins found in %s", mgr->tai_base);
    } else {
        snprintf(mgr->status_msg, sizeof(mgr->status_msg), "Found %d plugin(s). [X] Toggle config. [△] Re-scan.", mgr->count);
    }
}

void plugins_refresh_config(PluginManager *mgr) {
    // Mark all as not-in-config initially
    for (int i = 0; i < mgr->count; i++) {
        mgr->plugins[i].in_config = false;
    }

    SceUID fd = sceIoOpen(mgr->config_file, SCE_O_RDONLY, 0);
    if (fd < 0) return;

    char line[256];
    char current_section[32] = "*ALL";
    int pos = 0;
    char ch;

    while (sceIoRead(fd, &ch, 1) == 1) {
        if (ch == '\r') continue;
        if (ch == '\n' || pos >= (int)sizeof(line) - 1) {
            line[pos] = '\0';
            pos = 0;

            // Skip blank lines and comments
            if (line[0] == '\0' || line[0] == '#') continue;

            // Section header
            if (line[0] == '*') {
                snprintf(current_section, sizeof(current_section), "%.31s", line);
                continue;
            }

            // Check if line contains path to a .skprx file
            const char *slash = strrchr(line, '/');
            const char *fname = slash ? slash + 1 : line;

            // Match against discovered plugins
            for (int i = 0; i < mgr->count; i++) {
                if (strcasecmp(fname, mgr->plugins[i].filename) == 0) {
                    mgr->plugins[i].in_config = true;
                    snprintf(mgr->plugins[i].section, sizeof(mgr->plugins[i].section), "%s", current_section);
                    snprintf(mgr->plugins[i].config_path, sizeof(mgr->plugins[i].config_path), "%s", mgr->config_file);
                    mgr->plugins[i].status = PLUGIN_STATUS_ACTIVE;
                    break;
                }
            }
        } else {
            line[pos++] = ch;
        }
    }
    sceIoClose(fd);

    // Mark plugins on disk but NOT in config as unregistered
    for (int i = 0; i < mgr->count; i++) {
        if (mgr->plugins[i].on_disk && !mgr->plugins[i].in_config) {
            mgr->plugins[i].status = PLUGIN_STATUS_UNREGISTERED;
        }
    }
}

bool plugins_toggle_in_config(PluginManager *mgr, int idx) {
    if (idx < 0 || idx >= mgr->count) return false;
    PluginEntry *e = &mgr->plugins[idx];
    if (!e->on_disk) return false; // Can't add orphaned plugin that doesn't exist

    // Read existing config.txt
    SceUID fd = sceIoOpen(mgr->config_file, SCE_O_RDONLY, 0);
    char config_buf[8192] = {0};
    int read_bytes = 0;
    if (fd >= 0) {
        read_bytes = sceIoRead(fd, config_buf, sizeof(config_buf) - 1);
        if (read_bytes < 0) read_bytes = 0;
        config_buf[read_bytes] = '\0';
        sceIoClose(fd);
    }

    bool is_boot_critical = (strcasecmp(e->filename, "yamt.skprx") == 0 ||
                             strcasecmp(e->filename, "storagemgr.skprx") == 0);
    if (e->in_config && is_boot_critical) {
        snprintf(mgr->status_msg, sizeof(mgr->status_msg),
            "PROTECTED: Cannot disable %s! Required to boot current storage.", e->filename);
        return false;
    }

    if (e->in_config) {
        // Create backup of config before modifying
        char bak_path[160];
        snprintf(bak_path, sizeof(bak_path), "%s.bak", mgr->config_file);
        SceUID bak_fd = sceIoOpen(bak_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
        if (bak_fd >= 0) {
            sceIoWrite(bak_fd, config_buf, read_bytes);
            sceIoClose(bak_fd);
        }

        // --- Remove the plugin line from config ---
        char new_buf[8192] = {0};
        char *line = config_buf;
        char *next;
        int out_pos = 0;
        while (*line) {
            next = strchr(line, '\n');
            int line_len = next ? (int)(next - line + 1) : (int)strlen(line);
            // Check if this line references our plugin filename
            const char *slash = strrchr(line, '/');
            const char *fname_in_line = slash ? slash + 1 : line;
            if (strncasecmp(fname_in_line, e->filename, strlen(e->filename)) != 0) {
                strncpy(new_buf + out_pos, line, line_len);
                out_pos += line_len;
            }
            line = next ? next + 1 : line + line_len;
        }
        // Write back
        fd = sceIoOpen(mgr->config_file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
        if (fd >= 0) { sceIoWrite(fd, new_buf, out_pos); sceIoClose(fd); }
        e->in_config = false;
        e->status = PLUGIN_STATUS_UNREGISTERED;
        // --- Append plugin to *KERNEL (for .skprx) or *ALL (for .suprx) ---
        bool is_user_plugin = (strstr(e->filename, ".suprx") != NULL || strstr(e->filename, ".SUPRX") != NULL);
        const char *target_sec = is_user_plugin ? "*ALL" : "*KERNEL";
        fd = sceIoOpen(mgr->config_file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
        if (fd >= 0) {
            char append_line[256];
            if (strstr(config_buf, target_sec) == NULL) {
                char sec_header[32];
                snprintf(sec_header, sizeof(sec_header), "\n%s\n", target_sec);
                sceIoWrite(fd, sec_header, strlen(sec_header));
            }
            snprintf(append_line, sizeof(append_line), "%s\n", e->full_path);
            sceIoWrite(fd, append_line, strlen(append_line));
            sceIoClose(fd);
        }
        e->in_config = true;
        snprintf(e->section, sizeof(e->section), "%s", target_sec);
        snprintf(e->config_path, sizeof(e->config_path), "%s", mgr->config_file);
        e->status = PLUGIN_STATUS_ACTIVE;
    }

    mgr->restart_banner = true;
    return true;
}

void plugins_navigate(PluginManager *mgr, int dir) {
    if (mgr->count == 0) return;
    mgr->selected_index = (mgr->selected_index + dir + mgr->count) % mgr->count;
}

PluginEntry* plugins_get_selected(PluginManager *mgr) {
    if (mgr->count == 0) return NULL;
    return &mgr->plugins[mgr->selected_index];
}

const char* plugins_status_string(PluginStatus status) {
    switch (status) {
        case PLUGIN_STATUS_ACTIVE:       return "ACTIVE";
        case PLUGIN_STATUS_ORPHANED:     return "ORPHANED";
        case PLUGIN_STATUS_UNREGISTERED: return "NOT IN CONFIG";
        default:                         return "UNKNOWN";
    }
}
