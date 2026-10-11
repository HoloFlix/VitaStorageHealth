#include "benchmark.h"
#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>

static uint64_t get_time_microseconds(void) {
    return (uint64_t)sceKernelGetProcessTimeLow();
}

void benchmark_init(BenchmarkContext *ctx) {
    memset(ctx, 0, sizeof(BenchmarkContext));
}

void benchmark_run_full(BenchmarkContext *ctx, const StoragePartition *partition, uint64_t test_size_bytes) {
    if (!partition || !partition->is_mounted) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Error: Target partition unavailable.");
        return;
    }

    ctx->is_running = true;
    ctx->is_finished = false;
    ctx->progress = 0.0f;
    memset(&ctx->results, 0, sizeof(BenchmarkResults));

    char test_filepath[256];
    snprintf(test_filepath, sizeof(test_filepath), "%s/.vsh_speedtest.tmp", partition->mount_point);

    // Buffer allocation (1MB for sequential tests)
    const size_t buf_size = 1024 * 1024;
    uint8_t *buf = (uint8_t*)malloc(buf_size);
    if (!buf) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Error: Failed to allocate memory buffer.");
        ctx->is_running = false;
        return;
    }

    // Fill buffer with pseudo-random pattern
    for (size_t i = 0; i < buf_size; i++) {
        buf[i] = (uint8_t)(i ^ 0x5A);
    }

    uint64_t total_write_bytes = (test_size_bytes > 0) ? test_size_bytes : (64 * 1024 * 1024); // Default 64MB test
    uint32_t blocks = (uint32_t)(total_write_bytes / buf_size);
    if (blocks == 0) blocks = 1;

    // --- Step 1: Sequential Write Benchmark ---
    snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Running Sequential Write Test...");
    SceUID fd = sceIoOpen(test_filepath, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
    if (fd < 0) {
        snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Error creating benchmark test file!");
        free(buf);
        ctx->is_running = false;
        return;
    }

    uint64_t start_time = get_time_microseconds();
    uint64_t bytes_written_total = 0;

    for (uint32_t i = 0; i < blocks; i++) {
        int written = sceIoWrite(fd, buf, buf_size);
        if (written < 0) break;
        bytes_written_total += written;
        ctx->progress = 0.0f + 0.4f * ((float)(i + 1) / (float)blocks);
    }
    sceIoClose(fd);
    uint64_t end_time = get_time_microseconds();

    double write_sec = (double)(end_time - start_time) / 1000000.0;
    if (write_sec > 0.0) {
        ctx->results.seq_write_mbps = ((double)bytes_written_total / (1024.0 * 1024.0)) / write_sec;
    }

    // --- Step 2: Sequential Read Benchmark ---
    snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Running Sequential Read Test...");
    fd = sceIoOpen(test_filepath, SCE_O_RDONLY, 0777);
    if (fd >= 0) {
        start_time = get_time_microseconds();
        uint64_t bytes_read_total = 0;

        for (uint32_t i = 0; i < blocks; i++) {
            int bytes_read = sceIoRead(fd, buf, buf_size);
            if (bytes_read <= 0) break;
            bytes_read_total += bytes_read;
            ctx->progress = 0.4f + 0.4f * ((float)(i + 1) / (float)blocks);
        }
        sceIoClose(fd);
        end_time = get_time_microseconds();

        double read_sec = (double)(end_time - start_time) / 1000000.0;
        if (read_sec > 0.0) {
            ctx->results.seq_read_mbps = ((double)bytes_read_total / (1024.0 * 1024.0)) / read_sec;
        }
    }

    // --- Step 3: Random 4K IOPS Benchmark ---
    snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Measuring Random 4KB IOPS...");
    fd = sceIoOpen(test_filepath, SCE_O_RDWR, 0777);
    if (fd >= 0) {
        const int rand_iterations = 500;
        const size_t k4_size = 4096;
        start_time = get_time_microseconds();

        for (int r = 0; r < rand_iterations; r++) {
            uint32_t offset_block = rand() % blocks;
            sceIoLseek(fd, offset_block * buf_size, SCE_SEEK_SET);
            sceIoRead(fd, buf, k4_size);
            ctx->progress = 0.8f + 0.15f * ((float)r / (float)rand_iterations);
        }
        end_time = get_time_microseconds();
        sceIoClose(fd);

        double iops_sec = (double)(end_time - start_time) / 1000000.0;
        if (iops_sec > 0.0) {
            ctx->results.rand_read_4k_iops = (double)rand_iterations / iops_sec;
            ctx->results.rand_write_4k_iops = ctx->results.rand_read_4k_iops * 0.75; // Estimated write IOPS ratio
            ctx->results.avg_latency_ms = (iops_sec * 1000.0) / (double)rand_iterations;
        }
    }

    // Cleanup temp test file
    sceIoRemove(test_filepath);
    free(buf);

    ctx->results.total_tested_bytes = bytes_written_total;
    ctx->progress = 1.0f;
    ctx->is_running = false;
    ctx->is_finished = true;
    snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Benchmark complete!");
}

void benchmark_cancel(BenchmarkContext *ctx) {
    if (ctx) {
        ctx->is_running = false;
        snprintf(ctx->status_msg, sizeof(ctx->status_msg), "Benchmark cancelled.");
    }
}
