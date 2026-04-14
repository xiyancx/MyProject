// master_task.c - 涓绘帶鏍稿績浠诲姟

#include <ti/sysbios/knl/Task.h>
#include <srio.h>
#include <inc/mathlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "nand_op.h"
#include "shared_mem.h"

#pragma DATA_SECTION(g_mad_threshold, ".shared_msmc");
volatile float g_mad_threshold;

// 全局结果指针
// volatile SharedResults_t *g_shared_results;

// volatile uint8_t results_ready = 0;

static SharedResults_t result_buffer[RESULTS_PER_PAGE];

#pragma DATA_SECTION(process_frame_counter, ".shared_msmc");
uint32_t process_frame_counter = 0;

static uint32_t next_free_page = 0; // NAND结果区的下一个空闲页索引
void flush_result_buffer(void)
{
    uint32_t written_frames = next_free_page * RESULTS_PER_PAGE;
    if (process_frame_counter < written_frames)
    {
        printf("Warning: process_frame_counter rollback detected\n");
        return;
    }
    uint32_t frames_in_buffer = process_frame_counter - written_frames;
    if (frames_in_buffer == 0)
        return;
    uint32_t page_addr = NAND_RESULT_START_ADDR + (next_free_page * NAND_PAGE_SIZE);
    uint32_t bytes_to_write = frames_in_buffer * sizeof(SharedResults_t);
    if (nandflash_write(page_addr, bytes_to_write, (uint8_t *)result_buffer) != 0)
    {
        printf("Error: failed to write page %u\n", next_free_page);
    }
    else
    {
        printf("Written page %u, frames=%u, bytes=%u\n", next_free_page, frames_in_buffer, bytes_to_write);
    }
    next_free_page++;

    uint8_t verify_buf[64];
    uint32_t verify_len = (bytes_to_write < sizeof(verify_buf)) ? bytes_to_write : sizeof(verify_buf);
    if (verify_len > 0 && nandflash_read(page_addr, verify_len, verify_buf) == 0 &&
        memcmp(verify_buf, result_buffer, verify_len) != 0)
    {
        printf("Verify failed at page %u\n", next_free_page);
    }
}
void flush_all_results(void)
{
    uint32_t written_frames = next_free_page * RESULTS_PER_PAGE;
    if (process_frame_counter < written_frames)
    {
        printf("Warning: process_frame_counter rollback detected\n");
        return;
    }
    uint32_t frames_in_buffer = process_frame_counter - written_frames;
    if (frames_in_buffer == 0)
        return;

    uint32_t page_addr = NAND_RESULT_START_ADDR + (next_free_page * NAND_PAGE_SIZE);
    uint32_t bytes_to_write = frames_in_buffer * sizeof(SharedResults_t);

    if (nandflash_write(page_addr, bytes_to_write, (uint8_t *)result_buffer) != 0)
    {
        printf("Error: failed to write final page %u\n", next_free_page);
    }
    else
    {
        printf("Written final page %u, frames=%u, bytes=%u\n", next_free_page, frames_in_buffer, bytes_to_write);
    }
    // 注意：不要增加 next_free_page，因为停止后不再写入；或者增加也无妨，但不再使用
}

void add_result(SharedResults_t *res)
{
    if (res == NULL)
    {
        return;
    }
    memcpy(&result_buffer[process_frame_counter % RESULTS_PER_PAGE], res, sizeof(SharedResults_t));
    process_frame_counter++;
    if (process_frame_counter % RESULTS_PER_PAGE == 0)
    {
        flush_result_buffer();
    }
    shared_wb(&process_frame_counter, sizeof(uint32_t));
}

void collect_results_task(UArg arg0, UArg arg1)
{
    uint32_t expected_mask = 0x3E; // 期望核心1,2,3,4,5完成 (二进制: 0011 1110)
    uint32_t ready_mask = 0;
    uint32_t timeout_count = 0;

    uint32_t restore_size = sizeof(SharedResults_t); // 超时阈值（根据实际情况调整）
    int i;

    while (1)
    {
        shared_inv(&g_system_ready, sizeof(uint32_t));
        if (g_system_ready == 1)
            break;
        else
            // Task_yield();
            __asm(" NOP 5 ");
    }

    // 添加栈检查
    uint32_t stack_check[32]; // 栈检查数组
    for (i = 0; i < 32; i++)
        stack_check[i] = 0xDEADBEEF;

    while (1)
    {

        // 检查栈是否被破坏
        for (i = 0; i < 32; i++)
        {
            if (stack_check[i] != 0xDEADBEEF)
            {
                printf("Stack corruption detected at %d!\n", i);
                while (1)
                    ; // 挂起以便调试
            }
        }

        ready_mask = 0;
        for (i = 1; i <= 5; i++)
        {
            shared_inv(&results_ready[i], sizeof(uint32_t));
            if (results_ready[i])
            {
                ready_mask |= (1 << i);
            }
        }
        // shared_inv(&srio_data_available, sizeof(uint32_t));
        if (ready_mask == expected_mask) // if (srio_data_available)
        {
            shared_inv(&mad_axis_signal, sizeof(float) * 3);
            shared_inv(&g_shared_results.algorithm_amplitude, sizeof(float) * CAL_CORE_NUM);
            shared_inv(&devParams.MADThreshold, sizeof(float));
            g_shared_results.mad_result = sqrt(mad_axis_signal[0] * mad_axis_signal[0] + mad_axis_signal[1] * mad_axis_signal[1] + mad_axis_signal[2] * mad_axis_signal[2]);

            shared_inv(&g_mad_threshold, sizeof(float));

            if (g_shared_results.mad_result < g_mad_threshold)
            {
                shared_inv(&mad_result_counter, sizeof(uint8_t));
                mad_result_counter++;
                shared_wb(&mad_result_counter, sizeof(uint8_t));
            }

            // nandflash_write(NAND_RESULT_START_ADDR + process_frame_counter * restore_size, restore_size, (uint8_t *)&g_shared_results);
            #ifdef RESTORE_RAM
            shared_inv(&resultPtr, sizeof(resultPtr));
            if ((process_frame_counter < MAX_RESULE_NUMBER) && (resultPtr != NULL))
            {
                memcpy((uint8_t *)resultPtr + process_frame_counter * sizeof(SharedResults_t), (uint8_t *)&g_shared_results, sizeof(SharedResults_t));
                shared_wb((uint32_t *)((uint8_t *)resultPtr + process_frame_counter * sizeof(SharedResults_t)), sizeof(SharedResults_t));
            }
            process_frame_counter++;
            shared_wb(&process_frame_counter, sizeof(uint32_t));     
            #endif

            #ifdef RESTORE_NAND
            add_result(&g_shared_results);
            #endif
            results_ready[0] = 1; // 通知主核结果
            // 重置标志（为下一次准备）
            for (i = 1; i <= 5; i++)
            {
                results_ready[i] = 0;
            }
            shared_wb(&results_ready, sizeof(uint32_t) * 6);
        }
    }
}
