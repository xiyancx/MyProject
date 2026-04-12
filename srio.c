#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/Types.h>
#include <xdc/cfg/global.h>

/* SYS/BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/family/c66/Cache.h>
#include <ti/sysbios/family/c66/tci66xx/CpIntc.h>
#include <ti/sysbios/heaps/HeapBuf.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Semaphore.h>

/* CSL Header file */
#include <ti/csl/soc.h>
#include <ti/csl/cslr_device.h>
#include <ti/csl/csl_srio.h>
#include <ti/csl/cslr_srio.h>
#include <ti/csl/csl_srioAux.h>
#include <ti/csl/csl_srioAuxPhyLayer.h>
#include <ti/csl/csl_bootcfgAux.h>
#include <ti/csl/csl_pscAux.h>
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_chipAux.h>
#include <ti/csl/csl_cacheAux.h>
#include <ti/csl/csl_cache.h>
#include <ti/drv/srio/srio_drv.h>

/* Platform utilities include */
#include "system/platform.h"
#include "srio.h"
#include "shared_mem.h"

/* ======================= 用户可配置的固定参数宏 ======================= */
#define SRIO_LSU_NUM            0   // 核心号固定为0
#define SRIO_OUT_PORT           0                                // 输出端口号
#define SRIO_REMOTE_TARGET_SIZE 64 //Byte0-7:5A5A 5A5A 5A5A 5A5A, byte8:start01 stop00, byte9-10: freq/10, byte11:Amp, byte12-63:reserve
#define SRIO_REMOTE_TARGET_ADDR 0x20000000                       // 发送时的远程目标地址（与FPGA约定）
#define SRIO_REMOTE_SOURCE_ADDR 0x20000000                       // 接收时的远程源地址（通常与发送目标地址相同）
#define SRIO_SEND_FTYPE         Srio_Ftype_WRITE                 // 发送包类型：Srio_Ftype_WRITE (NWRITE) 或 Srio_Ftype_SWRITE
/* ==================================================================== */


extern const uint32_t CSR_LOCAL_DEVICEID_16BIT    = 0x0001;
extern const uint32_t CSR_LOCAL_DEVICEID_8BIT     = 0x01;
extern const uint32_t REMOTE_DEVICEID_16BIT      = 0x0099;
extern const uint32_t REMOTE_DEVICEID_8BIT       = 0x99;

/* SRIO DoorBell INFO */
#define SRIO_DOORB_INFO0           0
#define SRIO_DOORB_INFO1           1

/* global handle */
CSL_SrioHandle hSrio;
//extern Mailbox_Handle mbox_filter;

//Semaphore_Handle srio_data_sem;

#pragma DATA_ALIGN(srio_data_available, 128);
#pragma DATA_SECTION(srio_data_available, ".shared_msmc");
volatile uint32_t srio_data_available[6] = {0};

// Default parameters
#pragma DATA_ALIGN(fpgaParams, 128);
#pragma DATA_SECTION(fpgaParams, ".shared_msmc");

volatile FPGAParams fpgaParams;
DeviceParams devParams = {1, 50, 80, 1, 1, 2, 3, 0};

//#pragma DATA_SECTION(compute_available, ".shared_msmc");
//volatile uint32_t compute_available[4] = {0};

//#pragma DATA_ALIGN(srio_data_addr, 128)
//#pragma DATA_SECTION(srio_data_addr, ".fpga_reserved")
//uint32_t srio_data_addr = SRIO_DATA_ADDR;  // 0x0C3F0000

static volatile uint8_t srio_isr_buffer_index = 0;
static volatile uint32_t srio_isr_frame_counter = 0;

// 双锟斤拷锟斤拷峁�
typedef struct {
    uint32_t data_addr[2];        // 双锟斤拷锟斤拷锟街�
    volatile uint8_t write_idx;   // 锟斤拷前写锟斤拷锟斤拷锟斤拷
} SrioBuffer_t;

static SrioBuffer_t srio_buffer = {
    {0x0C3F0000, 0x0C3F4000},  // 锟斤拷应data_addr锟斤拷员
    0,                         // 锟斤拷应write_idx锟斤拷员
};


uint32_t isrCounter = 0;
void srio_db_isr(UArg arg)
{
    uint16_t db_status;

    // 锟斤拷取锟叫讹拷状态
    CSL_SRIO_GetDoorbellPendingInterrupt(hSrio, 0, &db_status);

    if(db_status && (1 << SRIO_DOORB_INFO0)) {
        // 锟叫伙拷锟斤拷锟斤拷锟斤拷
        //srio_buffer.write_idx ^= 1;//锟斤拷锟�
    	srio_buffer.write_idx = 0;
        uint32_t current_addr = srio_buffer.data_addr[srio_buffer.write_idx];

        // 锟斤拷锟斤拷锟斤拷锟捷匡拷锟矫憋拷志
        srio_data_available[0] = 1;
        srio_data_available[1] = 1;
        srio_data_available[2] = 1;
        srio_data_available[3] = 1;
        srio_data_available[4] = 1;
        srio_data_available[5] = 1;

        shared_wb(srio_data_available, sizeof(uint32_t) * 5);

        //srio_data_addr = current_addr;
        volatile void* fpga_data = (volatile void*)SRIO_DATA_ADDR;
        shared_wb(fpga_data, SRIO_DATA_SIZE);

        isrCounter ++;

        // 通知锟斤拷锟叫硷拷锟斤拷锟斤拷模锟斤拷锟揭伙拷锟斤拷锟姐播锟斤拷
        /*if (srio_mailbox_compute) {
            Mailbox_post(srio_mailbox_compute, &current_addr, BIOS_NO_WAIT);
        }*/
    }

    // 锟斤拷锟斤拷卸锟�
    if(db_status) {
        CSL_SRIO_ClearDoorbellPendingInterrupt(hSrio, 0, db_status);
    }
}

/**
 * @brief SRIO发送函数（将本地数据发送到固定远程地址）
 * @param local_addr  本地缓冲区地址（物理地址，可直接是DDR地址或经转换后的全局地址）
 * @param size        传输字节数
 * @return 0-成功，-1-失败
 */
int32_t srio_send(void)
{
    SRIO_LSU_TRANSFER tparams;
    uint8_t  contextBit, transactionID;
    uint32_t timeout = 10000000;          /* 超时等待计数 */
    uint8_t  uiCompletionCode, context;
    int32_t  status = 0;

    /* 确保本地地址为全局物理地址（若地址位于核心L2，需转换；若位于DDR，直接使用即可） */
    uint32_t global_local_addr = Convert_CoreLocal2GlobalAddr((uint32_t)SRIO_REMOTE_TARGET_ADDR);
    /* 设置LSU传输参数 */
    memset(&tparams, 0, sizeof(tparams));
    tparams.rapidIOLSB = global_local_addr;   /* 远程目标地址（固定） */
    tparams.dspAddress = (uint32_t)&fpgaParams;    /* 本地源地址 */
    tparams.bytecount = SRIO_REMOTE_TARGET_SIZE;
    tparams.dstID = REMOTE_DEVICEID_16BIT;

    tparams.outPortID = SRIO_OUT_PORT;
    tparams.idSize = 1;                             /* 8位ID */
    if(SRIO_SEND_FTYPE == Srio_Ftype_WRITE)
        tparams.ttype = Srio_Ttype_Write_NWRITE;
    tparams.ftype = SRIO_SEND_FTYPE;

    /* 等待LSU可用 */
    while (CSL_SRIO_IsLSUFull(hSrio, SRIO_LSU_NUM) == TRUE);

    /* 获取LSU上下文和事务ID */
    CSL_SRIO_GetLSUContextTransaction(hSrio, SRIO_LSU_NUM, &contextBit, &transactionID);

    /* 启动传输 */
    CSL_SRIO_SetLSUTransfer(hSrio, SRIO_LSU_NUM, &tparams);

    /* 等待传输完成 */
    while (timeout) {
        CSL_SRIO_GetLSUCompletionCode(hSrio, SRIO_LSU_NUM, transactionID,
                                       &uiCompletionCode, &context);
        if (context == contextBit) {
            transactionID = 0xFF;
            contextBit = 0xFF;
            if (uiCompletionCode != 0) {
                printf("SRIO发送错误，完成码: %d\n", -(uiCompletionCode));
                status = -1;
            }
            break;
        } else {
            timeout--;
            asm(" nop");
        }
    }

    if (timeout == 0) {
        printf("SRIO发送超时\n");
        status = -1;
    }

    return status;
}

/**
 * @brief SRIO接收函数（从固定远程地址读取数据到本地缓冲区）
 * @param local_addr  本地缓冲区地址（物理地址）
 * @param size        传输字节数
 * @return 0-成功，-1-失败
 */
int32_t srio_receive(uint32_t *local_addr, uint32_t size)
{
    SRIO_LSU_TRANSFER tparams;
    uint8_t  contextBit, transactionID;
    uint32_t timeout = 10000000;
    uint8_t  uiCompletionCode, context;
    int32_t  status = 0;

    /* 确保本地地址为全局物理地址 */
    uint32_t global_local_addr = Convert_CoreLocal2GlobalAddr((uint32_t)local_addr);

    /* 设置LSU传输参数（NREAD） */
    memset(&tparams, 0, sizeof(tparams));
    tparams.rapidIOLSB = SRIO_REMOTE_SOURCE_ADDR;   /* 远程源地址（固定） */
    tparams.dspAddress = global_local_addr;         /* 本地目标地址 */
    tparams.bytecount = size;
    tparams.ftype = Srio_Ftype_REQUEST;
    tparams.ttype = Srio_Ttype_Request_NREAD;
    tparams.dstID = REMOTE_DEVICEID_16BIT;
    tparams.outPortID = SRIO_OUT_PORT;
    tparams.idSize = 0;

    /* 等待LSU可用 */
    while (CSL_SRIO_IsLSUFull(hSrio, SRIO_LSU_NUM) == TRUE);

    /* 获取LSU上下文和事务ID */
    CSL_SRIO_GetLSUContextTransaction(hSrio, SRIO_LSU_NUM, &contextBit, &transactionID);

    /* 启动传输 */
    CSL_SRIO_SetLSUTransfer(hSrio, SRIO_LSU_NUM, &tparams);

    /* 等待传输完成 */
    while (timeout) {
        CSL_SRIO_GetLSUCompletionCode(hSrio, SRIO_LSU_NUM, transactionID,
                                       &uiCompletionCode, &context);
        if (context == contextBit) {
            transactionID = 0xFF;
            contextBit = 0xFF;
            if (uiCompletionCode != 0) {
                printf("SRIO接收错误，完成码: %d\n", -(uiCompletionCode));
                status = -1;
            }
            break;
        } else {
            timeout--;
            asm(" nop");
        }
    }

    if (timeout == 0) {
        printf("SRIO接收超时\n");
        status = -1;
    }

    return status;
}

static Int32 enable_srio (void)
{
    if(CSL_chipReadReg(CSL_CHIP_DNUM) == CORE_SYS_INIT) {
        /* SRIO power domain is turned OFF by default. It needs to be turned on before doing any
         * SRIO device register access. This not required for the simulator. */

        /* Set SRIO Power domain to ON */
        CSL_PSC_enablePowerDomain (CSL_PSC_PD_SRIO);

        /* Enable the clocks too for SRIO */
        CSL_PSC_setModuleNextState (CSL_PSC_LPSC_SRIO, PSC_MODSTATE_ENABLE);

        /* Start the state transition */
        CSL_PSC_startStateTransition (CSL_PSC_PD_SRIO);

        /* Wait until the state transition process is completed. */
        while (!CSL_PSC_isStateTransitionDone (CSL_PSC_PD_SRIO));

        /* Return SRIO PSC status */
        if ((CSL_PSC_getPowerDomainState(CSL_PSC_PD_SRIO) != PSC_PDSTATE_ON) ||
            (CSL_PSC_getModuleState (CSL_PSC_LPSC_SRIO) != PSC_MODSTATE_ENABLE)) {
            /* SRIO Power on failed. Return error */
            return -1;
        }
    } else {
        /* Waiting for the SRIO PSC subsystem to be initialized completelly */
        while((CSL_PSC_getPowerDomainState(CSL_PSC_PD_SRIO) != PSC_PDSTATE_ON) || \
                (CSL_PSC_getModuleState (CSL_PSC_LPSC_SRIO) != PSC_MODSTATE_ENABLE));
    }

    /* SRIO ON. Ready for use */
    return 0;
}

int srio_interrupt_init(void)
{
	Error_Block eb;
    Hwi_Params hwiParams;
    uint8_t event_id, cic_id, sys_int_id;

    Hwi_Params_init(&hwiParams);
    Error_init(&eb);
    /* Register interrupt: CIC0/1 INTDST0 -> HOST_interrupt 8 --> HWI interrupt 6 */
    if(CSL_chipReadReg(CSL_CHIP_DNUM) < 4) {
        cic_id = 0;
        sys_int_id = CSL_INTC0_INTDST0;
    } else {
        cic_id = 1;
        sys_int_id = CSL_INTC1_INTDST0;
    }

    CpIntc_dispatchPlug(sys_int_id, (CpIntc_FuncPtr)srio_db_isr, NULL, TRUE);

    /* Map CIC0/1 INTDST0 to Host INT8 */
    CpIntc_mapSysIntToHostInt(cic_id, sys_int_id, 8);

    /* Enable Host INT8 */
    CpIntc_enableHostInt(cic_id, 8);

    /* Enable CIC0/1 INTDST0 interrupt */
    CpIntc_enableSysInt(cic_id, sys_int_id);

    /* Get Host INT8 event ID */
    event_id = CpIntc_getEventId(8);
    Hwi_Params_init(&hwiParams);

    /* params arg must be host interrupt number */
    hwiParams.arg = 8;
    hwiParams.eventId = event_id;
    hwiParams.enableInt = TRUE;

    /* Register HWI interrupt */
    Hwi_create(6, &CpIntc_dispatch, &hwiParams, &eb);
    return 0;
}

Int32 srio_device_init (void)
{
    UInt32             i;
    UInt32             status;

    /* Get the CSL SRIO Handle. */
    hSrio = CSL_SRIO_Open (0);
    if (hSrio == NULL)
        return -1;

    if(CSL_chipReadReg(CSL_CHIP_DNUM) == CORE_SYS_INIT) {
        /* Code to disable SRIO reset isolation */
        if(CSL_PSC_isModuleResetIsolationEnabled(CSL_PSC_LPSC_SRIO))
            CSL_PSC_disableModuleResetIsolation(CSL_PSC_LPSC_SRIO);

        /* Disable the SRIO Global block */
        CSL_SRIO_GlobalDisable(hSrio);

        /* Disable each of the individual SRIO blocks */
        for(i = 0; i <= 9; i++)
            CSL_SRIO_DisableBlock(hSrio, i);

        /* BOOT_COMPLETE = 0:  write enabled */
        CSL_SRIO_SetBootComplete(hSrio, 0);

        /* Now enable the SRIO block and all the individual blocks also */
        CSL_SRIO_GlobalEnable(hSrio);
        for(i = 0; i <= 9; i++)
            CSL_SRIO_EnableBlock(hSrio,i);

        /* Configure SRIO ports mode */
        for(i = 0; i <= 3; i++)
            CSL_SRIO_SetNormalMode(hSrio, i);

        /* Enable Automatic Priority Promotion of response packets */
        CSL_SRIO_EnableAutomaticPriorityPromotion(hSrio);

        /*
         * Set the SRIO Prescalar select to operate in the range
         * PERSCALER_SELECT = 0: 44.7 ~ 89.5 MHz
         */
        CSL_SRIO_SetPrescalarSelect(hSrio, 0);

        /* Unlock the Boot Configuration Kicker */
        CSL_BootCfgUnlockKicker();

        /*
         * MPY = 0x50: 10x
         * ENPLL = 1: PLL Enable
         * srio_serdes_clock = RefClk(250MHz) * MPY = 2.5GHz
         */
        CSL_BootCfgSetSRIOSERDESConfigPLL(0x51);

        /*
         * Configure the SRIO SERDES Receive Configuration
         * ENOC     = 1: Enable offset compensation
         * EQ       = 1: Fully adaptive equalization
         * CDR      = 5: First order with fast lock
         * ALIGN    = 1: Comma alignment enabled
         * TERM     = 1: Input termination, the only valid value for this field is 0x1
         * RATE     = 1: Data Rate = 2 * srio_serdes_clock = 5Gbps
         * BUSWIDTH = 1: Bus width, indicate a 20-bit wide parallel bus to the clock
         * ENRX     = 1: Enable this receiver
         */
        for(i = 0; i <= 3; i++)
            CSL_BootCfgSetSRIOSERDESRxConfig(i, 0x00468495);

        /*
         * Configure the SRIO SERDES Transmit Configuration
         * MSYNC    = 1:  Enables the channel as the master lane
         * FIRUPT   = 1:  Transmitter pre and post cursor FIR filter update
         * TWPST1   = 18: Adjacent post cursor Tap weight
         * TWPRE    = 1:  Precursor Tap weight
         * SWING    = 16: Output swing
         * RATE     = 1:  Data Rate = 2 * srio_serdes_clock = 5Gbps
         * BUSWIDTH = 1:  Bus width, indicate a 20-bit wide parallel bus to the clock
         * ENRX     = 1:  Enable this receiver
         */
        for(i = 0; i <= 3; i++)
            CSL_BootCfgSetSRIOSERDESTxConfig(i, 0x001C8F95);

        /* Loop around till the SERDES PLL is not locked. */
        while(1) {
            /* Get the SRIO SERDES Status */
            CSL_BootCfgGetSRIOSERDESStatus(&status);
            if(status & 0x1)
                break;
        }

        /* Lock the Boot Configuration Kicker */
        CSL_BootCfgLockKicker();

        /* Clear the LSU pending interrupts. */
        CSL_SRIO_ClearLSUPendingInterrupt(hSrio, 0xFFFFFFFF, 0xFFFFFFFF);

        /* Set the 16 bit and 8 bit identifier for the SRIO Device */
        CSL_SRIO_SetDeviceIDCSR(hSrio, CSR_LOCAL_DEVICEID_8BIT, CSR_LOCAL_DEVICEID_16BIT);

        /* Configure the Base Routing Register */
        CSL_SRIO_SetTLMPortBaseRoutingInfo(hSrio, 0, 1, 1, 0, 0); //new add 20260327
        CSL_SRIO_SetTLMPortBaseRoutingPatternMatch(hSrio, 0, 1, REMOTE_DEVICEID_8BIT, 0xFF);//new add 20260327

        /* Configure the PLM for all the ports */
        for(i = 0; i <= 3; i++) {
            /*
             * TODO: We need to ensure that the Port 0 is configured to support both
             * the 2x and 4x modes. The Port Width field is read only.
             * So here we simply ensure that the Input and Output ports are enabled
             */
            CSL_SRIO_EnableInputPort(hSrio, i);
            CSL_SRIO_EnableOutputPort(hSrio, i);

            /*
             * Discovery timer is specified to be 28 msec +/- 4 msec
             * Discovery timer = RefClk(250MHz) period * PRESCALAR_SRV_CLK * 52429 * DISCOVERY_TIMER
             *                 = (1 / 250Mhz) * (250 / 10) * 52429 * 5 = 26.2ms
             */
            CSL_SRIO_SetPLMPortDiscoveryTimer(hSrio, i, 0x5);
        }

        /* Set the Port link timeout CSR */
        CSL_SRIO_SetPortLinkTimeoutCSR(hSrio, 0x000FFF);
        CSL_SRIO_SetPortResponseTimeoutCSR(hSrio, 0xFF0FFF);

        /* Set the Port General CSR: Only executing as Master Enable */
        CSL_SRIO_SetPortGeneralCSR(hSrio, 0, 1, 0);

        /* Clear the sticky register bits */
        CSL_SRIO_SetLLMResetControl(hSrio, 1);

        /* Set the Data Streaming MTU */
        CSL_SRIO_SetDataStreamingMTU(hSrio, 64);

        /* Configure the path mode 4 for the ports */
        CSL_SRIO_SetPLMPortPathControlMode(hSrio, 0, 4);

        /*
         * Set the LLM Port IP Prescalar
         * PRESCALAR_SRV_CLK = RefClk(250MHz) / 10 = 0x19
         */
        CSL_SRIO_SetLLMPortIPPrescalar(hSrio, 0x19);

        CSL_SRIO_DisableInterruptPacing (hSrio, 0);

        /*
         *  Set the Doorbell route to determine which routing table is to be used
         *  This configuration implies that the Interrupt Routing Table is configured as
         *  follows:-
         *  Interrupt Destination 0 - INTDST 0
         *  Interrupt Destination 1 - INTDST 1
         *  Interrupt Destination 2 - INTDST 2
         *  Interrupt Destination 3 - INTDST 3
         */
        CSL_SRIO_SetDoorbellRoute(hSrio, 1);

        /*
         * Route the Doorbell interrupts
         * Doorbell Register 0 - All 16 Doorbits are routed to Interrupt Destination 0.
         */
        for(i = 0; i < 16; i++)
            CSL_SRIO_RouteDoorbellInterrupts(hSrio, 0, i, 0);

        /* Set Data swap mode as Mode D, convert big-endian to little-endian */
        CSL_SRIO_SetMAUSwappingMode(hSrio, 3);

        /* Enable the peripheral. */
        CSL_SRIO_EnablePeripheral(hSrio);

        /* Configuration has been completed. */
        CSL_SRIO_SetBootComplete(hSrio, 1);

        /* This code checks if the ports are operational or not */
        while(CSL_SRIO_IsPortOk(hSrio, 0) == FALSE);
    } else {
        /* Waiting for the SRIO subsystem to be initialized completelly */
        while(CSL_SRIO_IsPortOk(hSrio, 0) == FALSE);
    }

    /* Initialization has been completed. */
    return 0;
}
int init_srio(void)
{
	int ret;
	Error_Block eb;
	Error_init(&eb);
#if 0
	srio_data_available = 1;
#endif

#if 1
    // enable SRIO PSC
    ret = enable_srio();
    if(ret != 0) {
        printf("srio psc initialization failed ! \r\n");
        return ret;
    }

    // configure and enable SRIO subsystem
    ret = srio_device_init();
    if(ret != 0) {
        printf("srio system initialization failed ! \r\n");
        return ret;
    }
    srio_interrupt_init();

    if(ret != 0) {
        printf("srio system initialization failed ! \r\n");
        return ret;
    }

#endif
    return 0;
}



