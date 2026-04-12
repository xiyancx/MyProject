/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file srio_fft.c
 *
 * @brief Example application main file.
 * This application will demonstrating how to respond to srio doorbell
 * interruption and 8 cores process data using FFT.
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2019-08-23
 *
 **/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/cfg/global.h> //SystemHeap

/* SYS/BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/family/c66/Cache.h>

/* NDK include */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>

/* DRV Header files */
#include <ti/drv/srio/srio_drv.h>
#include <ti/drv/pcie/pcie.h>

/* CSL Header files */
#include <ti/csl/soc.h>
#include <ti/csl/cslr_device.h>
#include <ti/csl/csl_bootcfgAux.h>
#include <ti/csl/csl_pscAux.h>
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_chipAux.h>

#include <c6x.h>
#include <inc/mathlib.h>
/* DSPLIB Header files */
#include <inc/dsplib.h>

/* Platform utilities include */
#include "system/resource_mgr.h"
#include "system/platform.h"

/* Driver utilities include */
#include "driver/c66x_uart.h"

#ifdef C66_PLATFORMS
#include <ti/csl/csl_cpsgmii.h>
#include <ti/csl/csl_cpsgmiiAux.h>
#endif

#include "net_init.h"
#include "protocol.h"
#include "adaptive_filter.h"
#include "srio.h"
#include "shared_mem.h"
#include "nand_op.h"

#pragma DATA_ALIGN(g_system_ready, 128);
#pragma DATA_SECTION(g_system_ready, ".shared_msmc");
volatile uint32_t g_system_ready = 0;

void collect_results_task(UArg arg0, UArg arg1);

// 所有核同时启动，但等待主核完成初始化
/**
 * @brief enable psc module
 *
 * @param void
 *
 * @return void
 *
 */
void psc_init(void)
{
    /* Set psc as Always on state */
    CSL_PSC_enablePowerDomain(CSL_PSC_PD_ALWAYSON);

#ifdef C66_PLATFORMS
    /* Enable EMIF Clock*/
    CSL_PSC_setModuleNextState(CSL_PSC_LPSC_EMIF25_SPI, PSC_MODSTATE_ENABLE);
#endif

    /* Start state change */
    CSL_PSC_startStateTransition(CSL_PSC_PD_ALWAYSON);

    /* Wait until the status change is completed */
    while (!CSL_PSC_isStateTransitionDone(CSL_PSC_PD_ALWAYSON))
        ;

    /*
     * SRIO power domain is turned OFF by default. It needs to be turned on before doing any
     * SRIO device register access. This not required for the simulator
     */

    /* Set SRIO Power domain to ON */
    CSL_PSC_enablePowerDomain(CSL_PSC_PD_SRIO);

    /* Enable the clocks too for SRIO */
    CSL_PSC_setModuleNextState(CSL_PSC_LPSC_SRIO, PSC_MODSTATE_ENABLE);

    /* Start the state transition */
    CSL_PSC_startStateTransition(CSL_PSC_PD_SRIO);

    /* Wait until the state transition process is completed. */
    while (!CSL_PSC_isStateTransitionDone(CSL_PSC_PD_SRIO))
        ;

    /* PASS power domain is turned OFF by default. It needs to be turned on before doing any
     * PASS device register access. This not required for the simulator. */

    /* Set PASS Power domain to ON */
    CSL_PSC_enablePowerDomain(CSL_PSC_PD_PASS);

    /* Enable the clocks for PASS modules */
    CSL_PSC_setModuleNextState(CSL_PSC_LPSC_PKTPROC, PSC_MODSTATE_ENABLE);
    CSL_PSC_setModuleNextState(CSL_PSC_LPSC_CPGMAC, PSC_MODSTATE_ENABLE);
    CSL_PSC_setModuleNextState(CSL_PSC_LPSC_Crypto, PSC_MODSTATE_ENABLE);

    /* Start the state transition */
    CSL_PSC_startStateTransition(CSL_PSC_PD_PASS);

    /* Wait until the state transition process is completed. */
    while (!CSL_PSC_isStateTransitionDone(CSL_PSC_PD_PASS))
        ;
}
void idle_core_loop(void)
{
    while (1)
    {
        Task_yield();
    }
}

// 原子写入（带内存屏障）
static inline void atomic_write(volatile uint32_t *addr, uint32_t value)
{
    __asm(" NOP 5 ");
    *addr = value;
    Cache_wb((void *)addr, sizeof(uint32_t), Cache_Type_ALL, CACHE_WAIT);
    __asm(" NOP 5 ");
}

// 原子读取（带缓存无效）
static inline uint32_t atomic_read(volatile uint32_t *addr)
{
    Cache_inv((void *)addr, sizeof(uint32_t), Cache_Type_ALL, CACHE_WAIT);
    __asm(" NOP 5 ");
    return *addr;
}
/**
 * @brief main function
 *
 * @details Program unique entry
 *
 * @param void
 *
 * @return successful execution of the program
 *     @retval 0  successful
 *     @retval -1 failed
 */

//uint8_t nandRd_buf[2048];
//uint8_t nandWr_buf[2048];

int main()
{
    Task_Params taskParams;
    Error_Block eb;
    uint32_t main_pll_freq;
    Task_Handle task;
    Error_init(&eb);
    Task_Params_init(&taskParams);

    uint8_t core_id = CSL_chipReadReg(CSL_CHIP_DNUM);

    switch (core_id)
    {
    case 0:

        psc_init();
        /* UART initialization */
        uart_init(CSL_UART_REGS);
        /* Get the cpu freq */
        main_pll_freq = platform_get_main_pll_freq();
        /* Set the  baud rate to 115200 */
        uart_set_baudrate(CSL_UART_REGS, main_pll_freq / 6, 115200);

        init_srio();
        #ifdef RESTORE_NAND
        nand_intialize();
        //erase_result_block();
        #endif

        // 清零共享同步块
        /*SharedSyncBlock* sync = SHARED_SYNC_BLOCK;
        memset((void*)sync, 0, sizeof(SharedSyncBlock));
        shared_wb(sync, sizeof(SharedSyncBlock));*/
        /*
        int i = 0;
        int ret = 0;
        for (i = 0; i < NAND_PAGE_SIZE; i++)
            nandWr_buf[i] = i;
        for(i = 0; i < 85; i++){
            // ret = nandflash_write(NAND_RESULT_START_ADDR + i * sizeof(SharedResults_t), sizeof(SharedResults_t), (uint8_t *)&g_shared_results);
            ret = nandflash_write(NAND_RESULT_START_ADDR + i * 24, 24, nandWr_buf + i * 24);
            // ret = nandflash_read(NAND_RESULT_START_ADDR + i * sizeof(SharedResults_t), sizeof(SharedResults_t), (uint8_t *)&temp);
            ret = nandflash_read(NAND_RESULT_START_ADDR + i * 24, 24, nandRd_buf + i * 24);
        }

        for (i = 0; i < 2048; i++)
        {
            if (nandRd_buf[i] != nandWr_buf[i])
            {
                printf("NAND read error at offset %d, expect %d, got %d\n", i, nandWr_buf[i], nandRd_buf[i]);
                break;
            }
        }
        if (i == 2048)
        {
            printf("NAND read/write verification passed for page %d\n", i);
        }
        else
        {
            printf("NAND read/write verification failed for page %d\n", i);
        }
        */
        

        memset((void *)SHM_BASE, 0, SHM_SIZE);
        shared_wb((void *)SHM_BASE, SHM_SIZE);

        generate_twiddle_factors(g_twiddle_fft_LMS, g_twiddle_ifft_LMS, FFT_NUM_LMS);
        generate_twiddle_factors(g_twiddle_fft_SLIP, g_twiddle_ifft_SLIP, FFT_NUM_SLIP);
        g_twiddle_ready = 1;
        shared_wb(&g_twiddle_ready, sizeof(g_twiddle_ready));

        printf("Core 0: Master init done, g_system_ready = %d\n", g_system_ready);
        // 主核心任务
        taskParams.stackSize = 0x10000;
        taskParams.priority = 7;
        taskParams.arg0 = (UArg)core_id;
        task = Task_create(netTask, &taskParams, &eb);
        if (task == NULL)
        {
            while (1)
                __asm(" NOP "); // 死在这里，便于调试
        }

        g_system_ready = 1;

        // 写回内存
        Cache_wb((void *)&g_system_ready, sizeof(g_system_ready),
                 Cache_Type_ALL, CACHE_WAIT);
        __asm(" NOP 5 ");
        printf("Core 0: Master init done, g_system_ready = %d\n", g_system_ready);
        // atomic_write(&g_system_ready, 1);
        //  设置系统就绪标志
        BIOS_start();
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        // 为每个计算核心创建独立任务
        /*taskParams.stackSize = 0x2000; // 16KB (adjust as needed)
        taskParams.priority = 3;
        taskParams.arg0 = (UArg)core_id; // 传递 core ID
        task = Task_create(compute_core_task, &taskParams, &eb);
        if (task == NULL) {
            while(1) __asm(" NOP "); // 死在这里，便于调试
        }
        BIOS_start();*/
        compute_core_task(0, 0);
        break;
    case 6:
        /*taskParams.stackSize = 0x2000; // 8KB for collector
        taskParams.priority = 4;
        task = Task_create(collect_results_task, &taskParams, &eb);
        if (task == NULL) {
            while(1) __asm(" NOP "); // 死在这里，便于调试
        }
        BIOS_start();*/
        collect_results_task(0, 0);
        break;
    default:
        break;
    }
    return 0;
}
