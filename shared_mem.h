// shared_mem.h
#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <xdc/std.h>
#include <ti/sysbios/family/c66/Cache.h>

// 锟斤拷锟斤拷锟节达拷锟斤拷锟街凤拷锟斤拷锟�.cfg 一锟铰ｏ拷
#define SHM_BASE        (0x0C000000)
#define SHM_SIZE        (0x400000)  // 2MB

#define SRIO_DATA_ADDR       	0x0C3F0000  // SRIO锟斤拷锟捷斤拷锟斤拷锟斤拷
#define SRIO_DATA_SIZE       	0x10000  //(65536)64K
#define FRAME_COUNTER_ADDR      0x0C3FFFFC  // SRIO锟斤拷锟捷斤拷锟斤拷锟斤拷之前4B

/* NAND 物理参数 */
#define NAND_BLOCK_SIZE         (128 * 1024)      // 128KB
#define NAND_PAGE_SIZE          (2 * 1024)        // 2KB

/* 用户数据起始块（避开第0块） */
#define NAND_USER_BLOCK_START   1                 // 从块1开始

/* 帧计数专用块（单独一个块，每次写入前擦除） */
#define NAND_COUNTER_BLOCK      1                 // 使用块1存储帧计数
#define NAND_COUNTER_ADDR       (NAND_COUNTER_BLOCK * NAND_BLOCK_SIZE)  // 0x20000

/* 日期数据块（可以单独分配一个块，或与结果共用，但注意擦除影响） */
#define NAND_DATE_BLOCK         2                 // 块2存储日期
#define NAND_DATE_ADDR          (NAND_DATE_BLOCK * NAND_BLOCK_SIZE)     // 0x40000

/* 结果数据区（从块3开始，占用64MB） */
#define NAND_RESULT_START_BLOCK  3
#define NAND_RESULT_START_ADDR   (NAND_RESULT_START_BLOCK * NAND_BLOCK_SIZE)  // 0x60000
#define NAND_RESULT_SIZE         (64 * 1024 * 1024)  // 64MB

#define RESULTS_PER_PAGE         85 //(NAND_PAGE_SIZE / sizeof(SharedResults_t))  // 85

#define MAX_RESULE_NUMBER        1080000 //(24 * 60 * 60 * 1000 / 80)  // 24小时 * 3600秒/小时 * 1000毫秒/秒 / 80ms/帧 = 1080000帧
// 锟斤拷锟街э拷锟�4 锟剿ｏ拷0锟斤拷3锟斤拷
#define TOTAL_CORES_USED      6
#define AXES_PER_CORE         1
#define CORE_SYS_INIT         0
#define CAL_CORE_NUM          5
#define REF_CH 6

#define RESTORE_RAM
//#define RESTORE_NAND
#pragma pack(push, 1)
typedef struct {
	float mad_result;
	float algorithm_amplitude[CAL_CORE_NUM];
} SharedResults_t;
#pragma pack(pop)

extern SharedResults_t g_shared_results;
//extern uint32_t srio_data_addr;
extern volatile uint32_t srio_data_available[6];
extern volatile uint32_t results_ready[6]; // [0] unused, [1]-[5] for core1-5;
extern volatile uint32_t g_system_ready;  // 0=not ready, 1=ready

extern volatile float g_mad_threshold;
extern volatile float g_fft_threshold;
extern volatile float g_lms_threshold;
extern volatile float g_lms_step_size;
extern volatile uint8_t g_lms_order;
extern volatile uint16_t g_lms_f0;


extern float mad_axis_signal[CAL_CORE_NUM];
extern uint8_t mad_result_counter;
extern uint8_t lms_result_counter[CAL_CORE_NUM];
extern uint8_t fft_result_counter[CAL_CORE_NUM];
extern float fft_max_amplitude[CAL_CORE_NUM];
extern float lms_amplitude[CAL_CORE_NUM];
extern float lms_phase[CAL_CORE_NUM];

extern float *resultPtr;
extern uint64_t timestamp;

// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷写锟斤拷锟叫�Cache
static inline void shared_wb(volatile void* ptr, size_t size) {
	__asm(" NOP 5 ");
    Cache_wb((void*)ptr, size, Cache_Type_ALL, TRUE);
    __asm(" NOP 5 ");
}

// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷前失效 Cache
static inline void shared_inv(volatile void* ptr, size_t size) {
    Cache_inv((void*)ptr, size, Cache_Type_ALL, TRUE);
    __asm(" NOP 5 ");
}

static inline void memory_barrier(void) {
    __asm(" NOP 5 ");
}

#endif
