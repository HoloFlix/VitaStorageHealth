#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>
#include <stdbool.h>

struct StoragePartition; // forward declaration

typedef struct {
    double seq_read_mbps;
    double seq_write_mbps;
    double rand_read_4k_iops;
    double rand_write_4k_iops;
    double avg_latency_ms;
    uint64_t total_tested_bytes;
    double elapsed_seconds;
} BenchmarkResults;

typedef struct {
    bool is_running;
    bool is_finished;
    float progress;               // 0.0 to 1.0
    char status_msg[128];
    BenchmarkResults results;
} BenchmarkContext;

void benchmark_init(BenchmarkContext *ctx);
void benchmark_run_full(BenchmarkContext *ctx, const struct StoragePartition *partition, uint64_t test_size_bytes);
void benchmark_cancel(BenchmarkContext *ctx);

#endif // BENCHMARK_H
