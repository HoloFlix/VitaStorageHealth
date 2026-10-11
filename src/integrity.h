#ifndef INTEGRITY_H
#define INTEGRITY_H

#include <stdint.h>
#include <stdbool.h>

struct StoragePartition;

typedef enum {
    SECTOR_UNTESTED = 0,
    SECTOR_GOOD,
    SECTOR_SLOW,
    SECTOR_CORRUPT
} SectorStatus;

#define GRID_SIZE 100

typedef struct {
    SectorStatus grid[GRID_SIZE];
    uint32_t total_sectors;
    uint32_t good_count;
    uint32_t slow_count;
    uint32_t bad_count;
    bool fake_card_detected;
    uint64_t reported_capacity;
    uint64_t actual_usable_capacity;
} SurfaceScanResult;

typedef struct {
    bool is_scanning;
    bool is_complete;
    float scan_progress;
    char status_text[128];
    SurfaceScanResult scan;
} IntegrityContext;

void integrity_init(IntegrityContext *ctx);
void integrity_run_scan(IntegrityContext *ctx, const struct StoragePartition *partition, bool full_surface_scan);
void integrity_cancel(IntegrityContext *ctx);

#endif // INTEGRITY_H
