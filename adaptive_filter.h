// adaptive_filter.h - 添加多核相关声明
#ifndef ADAPTIVE_FILTER_H
#define ADAPTIVE_FILTER_H

#include <stdint.h>

/* Configuration parameters */
#define FFT_NUM_LMS              4096//65536//1024    // Buffer size
#define FFT_NUM_SLIP             65536
#define MAX_FILTER_ORDER         32      // Maximum filter order
#define MAX_LEARN_FRAMES         80      // Maximum learning frames
#define FS    					 50000.0f         // Original sampling rate
//#define F0	                     1000.0f           // Signal frequency
#define BANDWIDTH                25.0f       // Bandwidth
//#define LMS_STEP_SIZE            0.01f       // LMS step size

/* PI, used in create FFT twiddle */
#define PI                         3.141592654f
/* singel ADC Channel sampling points */
//#define ADC_SAMPLING_FFT_NUM       1024

#define ADC_RANGE                  (10.0)
#define TOTAL_LSB                  32768

/* defines the core number responsible for system initialization */
// 平滑滤波器相关变量
#define SMOOTH_WINDOW_SIZE 3                         // 平滑窗口大小（3帧）
#define ENVELOPE_POINT_INDEX 5                       // 包络取第5个点
#define PHASE_POINT_INDEX (FFT_NUM_LMS - 1) // 相位取最后一个点

// 缓冲区大小计算
#define DATA_BLOCK_SIZE  FFT_NUM_LMS
#define TWIDDLE_SIZE_SLIP (2 * FFT_NUM_SLIP)        // 8KB
#define TWIDDLE_SIZE_LMS (2 * FFT_NUM_LMS)
#define COMPLEX_SIGNAL_SIZE (2 * FFT_NUM_SLIP) // 8KB

// 斜率计算参数
#define SLOPE_WINDOW_SIZE 15
#define SLOPE_THRESHOLD   1.0f
#define AMP_THRESHOLD     0.1f

typedef struct {
    float kalman_xk[2];      // Kalman filter state
    float kalman_P[2][2];    // Kalman covariance matrix
    float lms_coeff[MAX_FILTER_ORDER];     // LMS filter coefficients (order 30)
    float lms_buffer[MAX_FILTER_ORDER];    // LMS input buffer
    uint32_t buffer_idx;     // Buffer index
    double time_stamp;       // Timestamp (continuous time)
    uint32_t frame_count;    // Frame counter
    float last_amplitude;    // Amplitude of the 5th point in the previous frame
    float last_phase;        // Phase of the last point in the previous frame

    // 新增：斜率计算相关
    float amp_history[SLOPE_WINDOW_SIZE];   // 最近15个 amplitude 值
    uint32_t amp_count;      // 当前有效点数 (0-15)
    uint32_t amp_index;      // 环形缓冲区下一个写入位置
} axis_state_t;

typedef struct {
    float mad_result;
    float algorithm_result[5]; // 存储每个轴的结果
} AlgorithmResults_t;

extern float g_twiddle_fft_LMS[TWIDDLE_SIZE_LMS];
extern float g_twiddle_ifft_LMS[TWIDDLE_SIZE_LMS];
extern float g_twiddle_fft_SLIP[TWIDDLE_SIZE_SLIP];
extern float g_twiddle_ifft_SLIP[TWIDDLE_SIZE_SLIP];

extern volatile uint32_t g_twiddle_ready;

/* exported buffers/twiddles */
//extern float twiddle_fft[FFT_NUM * 2];
//extern float twiddle_ifft[FFT_NUM * 2];

/* public APIs */
void generate_twiddle_factors(float *tw_fft, float *tw_ifft, int n);
void srio_algorithm(void);
void kalman_demodulation(float *signal, float *envelope, float *phase,
                         float fs, float f0, uint32_t idx);
void kalman_demodulation_with_state(float *signal, float *envelope, float *phase,
                                   float fs, float f0, float t,
                                   float *xk, float P[2][2]);

/*int adaptive_filter_process(float *ref_signal, float *axis_signal,
                            float *twiddle_fft, float *twiddle_ifft,
                            uint32_t signal_length, SharedResults_t *results, axis_state_t *axis_state);*/

/* 多核相关函数 */
//void collect_results_task(UArg arg0, UArg arg1);
void compute_core_task(UArg arg0, UArg arg1);
void idle_core_loop(void);
#endif /* ADAPTIVE_FILTER_H */
