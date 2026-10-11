#ifndef TOOLS_H
#define TOOLS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "storage.h"
#include "benchmark.h"
#include "integrity.h"
#include "plugins.h"

typedef struct {
    uint64_t temp_bytes_found;
    uint64_t bgdl_bytes_found;
    uint64_t logs_bytes_found;
    uint64_t total_reclaimable_bytes;
    uint32_t total_files_cleaned;
    bool is_cleaning;
    bool clean_completed;
    char status_msg[128];
} CleanerContext;

typedef struct {
    char driver_name[32];      // e.g. "YAMT (Kernel Driver)", "StorageMgr", "Native"
    char ux0_target[32];       // e.g. "SD2Vita MicroSD"
    char uma0_target[32];      // e.g. "Sony Memory Card"
    bool yamt_active;
    bool report_saved;
    char report_path[128];
} StorageToolsConfig;

typedef struct {
    bool id_dat_present;
    bool id_dat_removed;
    uint32_t locks_removed;
    char status_msg[128];
    char pc_compat_title[64];
    char pc_compat_desc[160];
    char pc_solution[160];

    // Boot freeze & Multi-card conflict detection
    bool duplicate_tai_found;
    char duplicate_tai_path[64];
    bool boot_freeze_warning;
    char boot_freeze_desc[160];
    char boot_freeze_solution[160];
} RepairContext;

void tools_init(CleanerContext *cleaner, StorageToolsConfig *cfg, RepairContext *repair);
void tools_scan_junk(CleanerContext *cleaner);
void tools_purge_junk(CleanerContext *cleaner);
void tools_detect_driver_config(StorageToolsConfig *cfg);
void tools_diagnose_pc_compatibility(const StorageSystem *sys, const StorageToolsConfig *cfg, RepairContext *repair);
void tools_check_boot_conflicts(RepairContext *repair);
bool tools_fix_boot_conflicts(RepairContext *repair);
void tools_repair_unlink_id_dat(RepairContext *repair);
void tools_repair_filesystem_locks(RepairContext *repair);
bool tools_export_report(const StorageSystem *sys,
                         const BenchmarkContext *bench,
                         const SurfaceScanResult *scan,
                         const PluginManager *plugins,
                         StorageToolsConfig *cfg);

#endif // TOOLS_H
