#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "benchmark.h"
#include "integrity.h"

typedef enum {
    STORAGE_TYPE_UNKNOWN = 0,
    STORAGE_TYPE_SD2VITA,
    STORAGE_TYPE_VITA_MEMCARD,
    STORAGE_TYPE_INTERNAL_EMMC,
    STORAGE_TYPE_GAME_CARD,
    STORAGE_TYPE_USB_PD
} StorageType;

typedef struct StoragePartition {
    char mount_point[16];     // e.g. "ux0:", "ur0:"
    char label[32];           // e.g. "SD2Vita (Main Storage)"
    StorageType type;
    bool is_mounted;
    uint64_t total_bytes;
    uint64_t free_bytes;
    uint64_t used_bytes;
    double health_score;       // Estimated health percentage (0.0 - 100.0)
    uint32_t bad_blocks_found;
    bool is_boot_device;       // Currently booted primary storage (ux0:) - protected
    bool is_system_internal;   // Internal system storage (ur0:) - protected

    // Per-partition isolated results
    BenchmarkContext bench_ctx;
    IntegrityContext integrity_ctx;
} StoragePartition;

#define MAX_PARTITIONS 8

typedef struct {
    StoragePartition partitions[MAX_PARTITIONS];
    int count;
    int selected_index;
} StorageSystem;

void storage_init(StorageSystem *sys);
void storage_refresh(StorageSystem *sys);
StoragePartition* storage_get_selected(StorageSystem *sys);
const char* storage_type_to_string(StorageType type);
void storage_format_size(uint64_t bytes, char *out_buf, size_t buf_size);

#endif // STORAGE_H
