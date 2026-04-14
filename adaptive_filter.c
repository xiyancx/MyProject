/**
 * TI C6678 adaptive filtering algorithm implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <c6x.h>
#include <inc/dsplib.h>
#include <ti/csl/csl_cache.h>
#include <ti/csl/csl_cacheAux.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <inc/mathlib.h>
#include <xdc/runtime/Memory.h> 

/* CSL Header file */
#include <ti/csl/soc.h>
#include <ti/csl/cslr_device.h>

#include <ti/csl/csl_bootcfgAux.h>
#include <ti/csl/csl_pscAux.h>
#include <ti/csl/csl_chip.h>
#include <ti/csl/csl_chipAux.h>
#include <ti/sysbios/family/c66/Cache.h>
#include <ti/sysbios/knl/Task.h>

#include "adaptive_filter.h"
#include "srio.h"
#include "shared_mem.h"

#define PI 3.14159265358979323846f

#define SLIP_HISTORY_LEN  FFT_NUM_SLIP

#define FRAME_SIZE_100Hz 8 // 每帧降采样后点数，必须为偶数
#define IIR_STATE_LEN (FRAME_SIZE_100Hz + 4)
#define DECIM_FACTOR 512//500

// 根据选择的截止频率，填入对应系数（0.1 Hz, 0.5 Hz, 1.0 Hz）
static const float ha[3][5] = {
    {1.06207e-10f, 4.2483e-10f, 6.37244e-10f, 4.2483e-10f, 1.06207e-10f}, // 0.1Hz
    {6.42058e-8f,  2.56823e-7f, 3.85235e-7f,  2.56823e-7f, 6.42058e-8f},  // 0.5Hz
    {9.86013e-7f,  3.94405e-6f, 5.91608e-6f,  3.94405e-6f, 9.86013e-7f}   // 1.0Hz
};
static const float hb[3][5] = {
    { 3.98319f, -5.9497f,  3.94984f, -0.983328f, 0.0f}, // 0.1Hz
    { 3.91594f, -5.75133f, 3.75476f, -0.919368f, 0.0f}, // 0.5Hz
    { 3.83189f, -5.50965f, 3.52296f, -0.845217f, 0.0f}  // 1.0Hz
};

// 状态缓存（全局或静态）
static float y1[CAL_CORE_NUM][IIR_STATE_LEN];

// 全局静态缓冲区（放在 DDR3）
#pragma DATA_SECTION(g_twiddle_fft_LMS, ".filter_const");
#pragma DATA_SECTION(g_twiddle_ifft_LMS, ".filter_const");
float g_twiddle_fft_LMS[TWIDDLE_SIZE_LMS];
float g_twiddle_ifft_LMS[TWIDDLE_SIZE_LMS];

#pragma DATA_SECTION(g_twiddle_fft_SLIP, ".filter_const");
#pragma DATA_SECTION(g_twiddle_ifft_SLIP, ".filter_const");
float g_twiddle_fft_SLIP[TWIDDLE_SIZE_SLIP];
float g_twiddle_ifft_SLIP[TWIDDLE_SIZE_SLIP];

// 每核私有缓冲区
#pragma DATA_SECTION(g_filteredRef, ".filter_per_core");
#pragma DATA_SECTION(g_filteredAxis, ".filter_per_core");
#pragma DATA_SECTION(g_ref_signal, ".filter_per_core");
#pragma DATA_SECTION(g_axis_signal, ".filter_per_core");
#pragma DATA_SECTION(g_fft_slip_new, ".filter_per_core");
#pragma DATA_SECTION(g_axis_states, ".filter_per_core");
#pragma DATA_SECTION(mad_axis_signal, ".filter_per_core");

float g_filteredRef[CAL_CORE_NUM][FFT_NUM_SLIP];  // 存储每核的滤波后参考信号
float g_filteredAxis[CAL_CORE_NUM][FFT_NUM_SLIP]; // 存储每核的滤波后轴信号
float g_ref_signal[CAL_CORE_NUM][FFT_NUM_SLIP];
float g_axis_signal[CAL_CORE_NUM][FFT_NUM_SLIP];
float g_fft_slip_new[CAL_CORE_NUM][FFT_NUM_LMS]; // 存储每核的FFT结果
axis_state_t g_axis_states[CAL_CORE_NUM];
float mad_axis_signal[CAL_CORE_NUM]; // 存储每核MAD处理后的轴信号

// 波形滤波器内部临时缓冲（每核）
#pragma DATA_SECTION(g_fft_input, ".filter_temp");
#pragma DATA_SECTION(g_fft_output, ".filter_temp");
float g_fft_input[CAL_CORE_NUM][COMPLEX_SIGNAL_SIZE];
float g_fft_output[CAL_CORE_NUM][COMPLEX_SIGNAL_SIZE];

float slip_history[CAL_CORE_NUM][SLIP_HISTORY_LEN];

// Kalman demodulation 临时缓冲（每核）
#pragma DATA_SECTION(g_envelope, ".filter_temp");
#pragma DATA_SECTION(g_phase, ".filter_temp");
#pragma DATA_SECTION(g_error, ".filter_temp");
#pragma DATA_SECTION(g_lowpass_output, ".filter_temp");
float g_envelope[CAL_CORE_NUM][FFT_NUM_SLIP];
float g_phase[CAL_CORE_NUM][FFT_NUM_SLIP];
float g_error[CAL_CORE_NUM][FFT_NUM_SLIP];
float g_lowpass_output[CAL_CORE_NUM][FFT_NUM_LMS];

// 标志：旋转因子是否已生成
#pragma DATA_SECTION(g_twiddle_ready, ".filter_const");
volatile uint32_t g_twiddle_ready = 0;

/* Global state variables */
float factor = 0.00030517578125;

uint8_t mad_result_counter = 0; // MAD结果计数器
uint8_t lms_result_counter[CAL_CORE_NUM] = {0};
uint8_t fft_result_counter[CAL_CORE_NUM] = {0}; // Al

volatile uint8_t mad_result_flag;
uint8_t algorithm_detect_result_flag[CAL_CORE_NUM]; // Algorithm result counter for each axis

// extern Semaphore_Handle srio_data_sem;
// extern volatile uint32_t srio_data_available;

#pragma DATA_ALIGN(results_ready, 128)
#pragma DATA_SECTION(results_ready, ".shared_msmc");
volatile uint32_t results_ready[6] = {0}; //[0]:all cores result ready flag, [1-5]: core1-5 result ready flag
  
#pragma DATA_SECTION(g_fft_threshold, ".shared_msmc");
volatile float g_fft_threshold; 
#pragma DATA_SECTION(g_lms_threshold, ".shared_msmc");
volatile float g_lms_threshold;
#pragma DATA_SECTION(g_lms_step_size, ".shared_msmc");
volatile float g_lms_step_size;
#pragma DATA_SECTION(g_lms_order, ".shared_msmc");
volatile uint8_t g_lms_order;
#pragma DATA_SECTION(g_lms_f0, ".shared_msmc");
volatile uint16_t g_lms_f0;


float lms_amplitude[CAL_CORE_NUM];
float lms_phase[CAL_CORE_NUM];

SharedResults_t g_shared_results;

// uint32_t frame_counter = 0;
uint32_t t_start, t_end, t_cost;

unsigned char brev[64] = {
    0x00, 0x20, 0x10, 0x30, 0x08, 0x28, 0x18, 0x38,
    0x04, 0x24, 0x14, 0x34, 0x0c, 0x2c, 0x1c, 0x3c,
    0x02, 0x22, 0x12, 0x32, 0x0a, 0x2a, 0x1a, 0x3a,
    0x06, 0x26, 0x16, 0x36, 0x0e, 0x2e, 0x1e, 0x3e,
    0x01, 0x21, 0x11, 0x31, 0x09, 0x29, 0x19, 0x39,
    0x05, 0x25, 0x15, 0x35, 0x0d, 0x2d, 0x1d, 0x3d,
    0x03, 0x23, 0x13, 0x33, 0x0b, 0x2b, 0x1b, 0x3b,
    0x07, 0x27, 0x17, 0x37, 0x0f, 0x2f, 0x1f, 0x3f};

/**
 * Moving average filter (similar to MATLAB's fspecial('average'))
 * @param new_value New value to be filtered
 * @param history Array of historical values
 * @param index Pointer to the current history index
 * @param window_size Size of the moving window
 * @return Filtered result
 */
static float moving_average_filter(float new_value, float *history, uint32_t *index, uint32_t window_size)
{
    uint32_t i;
    // Store the new value in the history array
    history[*index] = new_value;
    *index = (*index + 1) % window_size;

    // Calculate the average
    float sum = 0.0f;
    for (i = 0; i < window_size; i++)
    {
        sum += history[i];
    }

    return sum / window_size;
}

/**
 * Remove mean
 */
static void remove_mean(float *signal, uint32_t length)
{
    float mean = 0.0f;
    uint32_t i;

    /* Compute mean */
    for (i = 0; i < length; i++)
    {
        mean += signal[i];
    }
    mean /= length;

    /* Subtract mean */
    for (i = 0; i < length; i++)
    {
        signal[i] -= mean;
    }
}

static float calc_mean(float *signal, uint32_t length)
{
    float mean = 0.0f;
    uint32_t i;

    /* Compute mean */
    for (i = 0; i < length; i++)
    {
        mean += signal[i];
    }
    mean /= length;

    return mean;
}

/**
 * Generate twiddle factors
 */
void generate_twiddle_factors(float *tw_fft, float *tw_ifft, int n)
{
    int i, j, k;
    /* Generate FFT twiddle factors */
    for (j = 1, k = 0; j <= n >> 2; j = j << 2)
    {
        for (i = 0; i < n >> 2; i += j)
        {
            tw_fft[k] = (float)sin(2 * PI * i / n);
            tw_fft[k + 1] = (float)cos(2 * PI * i / n);
            tw_fft[k + 2] = (float)sin(4 * PI * i / n);
            tw_fft[k + 3] = (float)cos(4 * PI * i / n);
            tw_fft[k + 4] = (float)sin(6 * PI * i / n);
            tw_fft[k + 5] = (float)cos(6 * PI * i / n);
            k += 6;
        }
    }

    /* Generate IFFT twiddle factors */
    for (j = 1, k = 0; j <= n >> 2; j = j << 2)
    {
        for (i = 0; i < n >> 2; i += j)
        {
            tw_ifft[k] = -(float)sin(2 * PI * i / n);
            tw_ifft[k + 1] = (float)cos(2 * PI * i / n);
            tw_ifft[k + 2] = -(float)sin(4 * PI * i / n);
            tw_ifft[k + 3] = (float)cos(4 * PI * i / n);
            tw_ifft[k + 4] = -(float)sin(6 * PI * i / n);
            tw_ifft[k + 5] = (float)cos(6 * PI * i / n);
            k += 6;
        }
    }
}

/**
 * Harmonic wavelet band-pass filter
 */
static void harmonic_wavelet_filter(float *input, float *output,
                                    float *twiddle_fft, float *twiddle_ifft,
                                    uint32_t length, float f0,
                                    float bandwidth, float fs,
                                    float *fft_input_buf, // 新增参数
                                    float *fft_output_buf // 新增参数
)
{
    int n = length;
    float detf = 1.0f / (n / fs); // Frequency resolution
    int p = (floor)((f0 - bandwidth / 2.0f) / detf);
    int q = (ceil)((f0 + bandwidth / 2.0f) / detf);
    int i;
    // 直接使用传入的缓冲区
    float *fft_input = fft_input_buf;   // size: 2*FFT_NUM
    float *fft_output = fft_output_buf; // size: 2*FFT_NUM

    /* Prepare FFT input */
    for (i = 0; i < n; i++)
    {
        fft_input[2 * i] = input[i];
        fft_input[2 * i + 1] = 0.0f;
    }

    /* Perform FFT */
    uint8_t radix = (n & 0x55555555) ? 4 : 2;
    DSPF_sp_fftSPxSP(n, fft_input, twiddle_fft, fft_output, brev, radix, 0, n);

    /* Apply harmonic band-pass window */
    for (i = 0; i < n; i++)
    {
        if (i < p || i > q)
        {
            fft_output[2 * i] = 0.0f;
            fft_output[2 * i + 1] = 0.0f;
        }
        else
        {
            /* Preserve original amplitude (phase-preserving scaling) */
            float scale = 1.0f / (bandwidth * 2.0f * PI);
            fft_output[2 * i] *= scale;
            fft_output[2 * i + 1] *= scale;
        }
    }
    /* Perform IFFT */
    DSPF_sp_ifftSPxSP(n, fft_output, twiddle_ifft, fft_input, brev, radix, 0, n);

    /* Extract real part as output */
    for (i = 0; i < n; i++)
    {
        output[i] = fft_input[2 * i] * 4.0f * PI * bandwidth;
    }
}

float lowpass_filter_process(int core_id, float *input, uint32_t length)
{
    float dec[FRAME_SIZE_100Hz];
    float output_100hz[IIR_STATE_LEN];
    int i;
    int idx = devParams.MADStopFreq;

    if (length == DATA_BLOCK_SIZE)
    {
        for (i = 0; i < 8; i++)
        {
            dec[i] = input[i * DECIM_FACTOR]; // 简单抽取（无抗混叠，不推荐）
        }
    }
    else
    { // length = BLOCK_SIZE / 2 = 2048
        for (i = 0; i < 8; i++)
        {
            dec[i] = input[i * DECIM_FACTOR / 2]; // 简单抽取（无抗混叠，不推荐）
        }
    }
    if (idx < 0 || idx > 2) idx = 0;
    // 执行滤波, 输出 output_100hz 中即为滤波后数据，状态自动保存在 y1 中供下一帧使用
    DSPF_sp_iir(y1[core_id], dec, output_100hz, hb[idx], ha[idx], FRAME_SIZE_100Hz);
    memcpy (y1[core_id], output_100hz, sizeof(float) * IIR_STATE_LEN);

    // 均值
    float mean = calc_mean(output_100hz + 4, FRAME_SIZE_100Hz);

    return mean;
}

/**
 * LMS adaptive filter (using simplified Kalman demodulation)
 * Modified: output the amplitude of the 5th point and the phase of the last point of each frame
 */
static int adaptive_LMS_filter(int core_id, float *ref, float *input,
                               float *frame_amplitude, float *frame_phase,
                               uint32_t length, float fs, float f0,
                               float step, int order, int learn_frames,
                               axis_state_t *state)
{
	if (order > MAX_FILTER_ORDER) return -1;

    uint32_t i, j;
    float *w = state->lms_coeff;      // Use coefficients from state
    float *u_buf = state->lms_buffer; // Use buffer from state
    uint32_t buf_idx = state->buffer_idx;

    // Temporary error buffer
    // float error[FFT_NUM];
    float *error = g_error[core_id - 1];       // 每核一个错误缓冲区
    float *envelope = g_envelope[core_id - 1]; // 每核一个envelope缓冲区
    float *phase = g_phase[core_id - 1];       // 每核一个phase缓冲区

    // LMS adaptive filtering
    for (j = 0; j < length; j++)
    {
        // Update input buffer (similar to u_front in MATLAB)
        // Place new reference signal value into buffer
        u_buf[buf_idx] = ref[j];

        // Calculate filter output (y = u*w)
        float y = 0.0;
        for (i = 0; i < order; i++)
        {
            int idx = (buf_idx - i + order) % order; // Circular buffer index
            y += w[i] * u_buf[idx];
        }

        // Calculate error signal
        error[j] = input[j] - y;

        // If in learning stage, update weights
        if (state->frame_count < learn_frames)
        {
            for (i = 0; i < order; i++)
            {
                int idx = (buf_idx - i + order) % order;
                w[i] += 2.0 * step * error[j] * u_buf[idx];
            }
        }

        // Update buffer index (similar to u_front in MATLAB)
        buf_idx = (buf_idx + 1) % order;
    }

    // Save buffer index
    state->buffer_idx = buf_idx;

    // Apply Kalman demodulation to error signal (using state's Kalman parameters)
    for (i = 0; i < length; i++)
    {
        float t = state->time_stamp + (float)i / fs;

        // Apply Kalman demodulation with external state
        kalman_demodulation_with_state(&error[i], &envelope[i], &phase[i],
                                       fs, f0, t,
                                       state->kalman_xk, state->kalman_P);
    }

    // Update timestamp (similar to t0 = t0 + length/fs in MATLAB)
    state->time_stamp += (float)length / fs;

    // Simulate MATLAB behavior: use last point's state to update Kalman state
    // Note: MATLAB uses sign(phi) in the update, so we simulate it here
    float last_phase = phase[length - 1];
    float last_amplitude = envelope[length - 1];

    // Update Kalman state (similar to xk_initial = [A*sin(theta)*sign(theta), A*cos(theta)*sign(theta)] in MATLAB)
    if (last_phase != 0.0)
    {
        float sign_phi = (last_phase > 0.0) ? 1.0 : -1.0;
        state->kalman_xk[0] = last_amplitude * (float)sin(last_phase * PI / 180.0) * sign_phi;
        state->kalman_xk[1] = last_amplitude * (float)cos(last_phase * PI / 180.0) * sign_phi;
    }
    else
    {
        state->kalman_xk[0] = 0.0;
        state->kalman_xk[1] = last_amplitude;
    }

    // Extract results: 5th point's amplitude and last point's phase
    // Note: MATLAB array indexing starts from 1, so 5th point is at index 4 (C starts from 0)
    *frame_amplitude = (length > 4) ? envelope[4] : envelope[length - 1];
    *frame_phase = phase[length - 1];

    // Optional: save amplitude and phase arrays (for debugging)
    // memcpy(ampArray, envelope, sizeof(float) * length);
    // memcpy(phaseArray, phase, sizeof(float) * length);

    // free(envelope);
    // free(phase);
    return 0;
}

/**
 * Kalman demodulation (full version, external state)
 * Suitable for applications that need state control
 * Modification: Added sign handling, similar to MATLAB's norm(xk)*sign(phi)
 */
void kalman_demodulation_with_state(float *signal, float *envelope, float *phase,
                                    float fs, float f0, float t,
                                    float *xk, float P[2][2])
{
    float Q[2][2] = {{0.01, 0.0}, {0.0, 0.01}};
    float R = 1.0;
    float w = 2.0 * PI * f0;

    /* State prediction */
    float xk_pred[2];
    xk_pred[0] = xk[0];
    xk_pred[1] = xk[1];

    /* Covariance prediction */
    float P_pred[2][2];
    int i, j;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            P_pred[i][j] = P[i][j] + Q[i][j];
        }
    }

    /* Kalman gain */
    float C[2];
    C[0] = (float)sin(w * t);
    C[1] = (float)cos(w * t);
    float S = 0.0;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            S += C[i] * P_pred[i][j] * C[j];
        }
    }
    S += R;

    float K[2];
    for (i = 0; i < 2; i++)
    {
        K[i] = 0.0;
        for (j = 0; j < 2; j++)
        {
            K[i] += P_pred[i][j] * C[j];
        }
        K[i] /= S;
    }

    /* State update */
    float y_pred = xk_pred[0] * C[0] + xk_pred[1] * C[1];
    float y_error = *signal - y_pred;

    for (i = 0; i < 2; i++)
    {
        xk[i] = xk_pred[i] + K[i] * y_error;
    }

    /* Covariance update */
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 2; j++)
        {
            P[i][j] = P_pred[i][j] - K[i] * C[j] * P_pred[i][j];
        }
    }

    /* Compute envelope and phase (similar to MATLAB's norm(xk)*sign(phi) ) */
    float amp = (float)sqrt(xk[0] * xk[0] + xk[1] * xk[1]);
    float phi = (float)atan2(xk[0], xk[1]) * 180.0f / PI;

    // Add sign handling: norm(xk)*sign(phi)
    *envelope = amp * ((phi >= 0.0f) ? 1.0f : -1.0f);
    *phase = phi;
}

int adaptive_filter_process(float *ref_signal, float *axis_signal,
                            float *twiddle_fft, float *twiddle_ifft,
                            uint32_t signal_length,
                            float *frame_amplitude, float *frame_phase,
                            axis_state_t *axis_state)
{

    if (signal_length != FFT_NUM_LMS || !frame_amplitude || !frame_phase || !axis_state)
    {
        return -1;
    }

    uint8_t core_id = CSL_chipReadReg(CSL_CHIP_DNUM);
    TSCL = 0;
    TSCH = 0;
    // t_start = _itoll(TSCH, TSCL);

    float *filteredRef = g_filteredRef[core_id - 1];
    float *filteredAxis = g_filteredAxis[core_id - 1];

    // Remove DC component
    remove_mean(ref_signal, signal_length);
    remove_mean(axis_signal, signal_length);

    // Harmonic wavelet filter
    shared_inv(&g_lms_f0, sizeof(uint16_t));
    shared_inv(&g_lms_order, sizeof(uint8_t));
    shared_inv(&g_lms_step_size, sizeof(float));

    harmonic_wavelet_filter(ref_signal, filteredRef, twiddle_fft, twiddle_ifft, signal_length, g_lms_f0, BANDWIDTH, FS, g_fft_input[core_id - 1], g_fft_output[core_id - 1]);
    harmonic_wavelet_filter(axis_signal, filteredAxis, twiddle_fft, twiddle_ifft, signal_length, g_lms_f0, BANDWIDTH, FS, g_fft_input[core_id - 1], g_fft_output[core_id - 1]);

    // t_end = _itoll(TSCH, TSCL);
    // printf("processing_time: %d cycles\n", (t_end - t_start));
    
    // Adaptive LMS filter and Kalman demodulation
    if (adaptive_LMS_filter(core_id, filteredRef, filteredAxis,
                            frame_amplitude, frame_phase,
                            signal_length, FS,
                            g_lms_f0, g_lms_step_size,
                            g_lms_order, MAX_LEARN_FRAMES,
                            axis_state) != 0)
    {
        return -1;
    }

    // Frame counter increment
    axis_state->frame_count++;

    return 0;
}

// 生成旋转因子（与DSPF_sp_fftSPxSP兼容）
void gen_fft_twiddle(float *tw, uint32_t n)
{
    uint32_t i, j, k;
    for (j = 1, k = 0; j <= (n >> 2); j <<= 2)
    {
        for (i = 0; i < (n >> 2); i += j)
        {
            tw[k] = (float)sin(2 * PI * i / n);
            tw[k + 1] = (float)cos(2 * PI * i / n);
            tw[k + 2] = (float)sin(4 * PI * i / n);
            tw[k + 3] = (float)cos(4 * PI * i / n);
            tw[k + 4] = (float)sin(6 * PI * i / n);
            tw[k + 5] = (float)cos(6 * PI * i / n);
            k += 6;
        }
    }
}

// 滑动FFT更新并返回幅度谱最大值
float sliding_fft_max_mag(uint32_t fft_size,
                          uint32_t update_size,
                          float *data_buffer,
                          float *twiddle,
                          float *fft_input,
                          float *fft_output,
                          float *new_data)
{
    uint32_t i;
    if (fft_size == 0 || update_size == 0 || update_size > fft_size ||
        data_buffer == NULL || twiddle == NULL || fft_input == NULL || fft_output == NULL || new_data == NULL)
    {
        return 0.0f;
    }

    uint8_t radix = (fft_size & 0x55555555) ? 4 : 2; // 与其余FFT调用保持一致
    float max_mag_sq = 0.0f;
    uint32_t max_idx = 0;

    // 1. 滑动窗口：删除前update_size个点，剩余点前移
    memmove(data_buffer, data_buffer + update_size,
            (fft_size - update_size) * sizeof(float));

    // 2. 追加新数据
    memcpy(data_buffer + fft_size - update_size, new_data,
           update_size * sizeof(float));

    // 3. 实数转复数格式
    for (i = 0; i < fft_size; i++)
    {
        fft_input[2 * i] = data_buffer[i];
        fft_input[2 * i + 1] = 0.0f;
    }

    // 4. 执行FFT
    DSPF_sp_fftSPxSP(fft_size, fft_input, twiddle, fft_output, brev,
                     radix, 0, fft_size);

    // 5. 找出最大幅度平方对应的索引（忽略直流分量时可将i从1开始）
    for (i = 2; i < fft_size; i++)
    {
        float real = fft_output[2 * i];
        float imag = fft_output[2 * i + 1];
        float mag_sq = real * real + imag * imag;
        if (mag_sq > max_mag_sq)
        {
            max_mag_sq = mag_sq;
            max_idx = i;
        }
    }

    // 6. 返回幅度最大值（开方）
    return sqrtf(max_mag_sq);
}

// 初始化：首次调用前清零状态
void iir_lowpass_init(uint8_t core_id)
{
	int i;
    for (i = 0; i < IIR_STATE_LEN; i++)
    {
        y1[core_id][i] = 0.0f;
    }
}

/**
 * 计算核心任务函数
 */
void compute_core_task(UArg arg0, UArg arg1)
{
    uint8_t core_id = CSL_chipReadReg(CSL_CHIP_DNUM);
    if (core_id < 1 || core_id > 5)
    {
        printf("compute_core_task running on unexpected core %u\n", core_id);
        while (1)
        {
            __asm(" NOP 5 ");
        }
    }
    //    if (core_id > 3) while(1) __asm(" IDLE ");

    // 进入空循环，不进行任何操作
    /*while (1) {
        Task_yield();
    }*/

    // 等待旋转因子就绪
    while (1)
    {
        shared_inv(&g_twiddle_ready, sizeof(g_twiddle_ready));
        if (g_twiddle_ready)
            break;
        // Task_sleep(1);
        __asm(" NOP 5 ");
    }

    // 获取本核缓冲区指针
    float *ref_signal = g_ref_signal[core_id - 1];
    float *axis_signal = g_axis_signal[core_id - 1];
    float *axis_signal_new = g_fft_slip_new[core_id - 1]; // 复用FFT输入缓冲区存储新数据

    // 先打印一条消息，确认任务已经启动
    // printf("Core %d: compute_core_task started\n", core_id);

    // uint8_t start_axis, end_axis;
    // axis_state_t axis_states[AXES_PER_CORE];

    int i, ret;
    int16_t *raw_data = (int16_t *)SRIO_DATA_ADDR;
    int16_t *srio_data_addr_ref = raw_data + REF_CH * DATA_BLOCK_SIZE;
    int16_t *srio_data_addr_core = raw_data + core_id * DATA_BLOCK_SIZE;
    uint32_t single_data_size = 2 * DATA_BLOCK_SIZE;

    axis_state_t *axis_states = g_axis_states + core_id - 1; // 每核一个状态

    /* 初始化轴状态 */
    // for (i = 0; i < AXES_PER_CORE; i++)
    //{
    // memset(&axis_states[i], 0, sizeof(axis_state_t));
    axis_states->kalman_xk[0] = 0.0f;
    axis_states->kalman_xk[1] = 0.0f;
    axis_states->kalman_P[0][0] = 4.0f;
    axis_states->kalman_P[0][1] = 0.0f;
    axis_states->kalman_P[1][0] = 0.0f;
    axis_states->kalman_P[1][1] = 4.0f;
    //}

    // iir_lowpass_init(core_id - 1);
    memset(y1[core_id - 1], 0, IIR_STATE_LEN * sizeof(float));
    memset(slip_history[core_id -1], 0 , SLIP_HISTORY_LEN * sizeof(float));

    while (1)
    {

        // Mailbox_pend(srio_mailbox_compute, &srio_data_addr, BIOS_WAIT_FOREVER);

        shared_inv(&srio_data_available[core_id], sizeof(uint32_t));

        if (srio_data_available[core_id] == 0)
        {
            // Task_sleep(1);
            __asm(" NOP 5 ");
            continue;
        }
        /*for(i = 0; i < core_id; i++)
            Task_yield();*/
        // t_start = _itoll(TSCH, TSCL);

        if (devParams.algorithmMode == 0)
        {
            Cache_inv((void *)srio_data_addr_ref, single_data_size, Cache_Type_ALL, CACHE_WAIT);
            Cache_inv((void *)srio_data_addr_core, single_data_size, Cache_Type_ALL, CACHE_WAIT);
            for (i = 0; i < FFT_NUM_LMS; i++)
            {
                /* 提取参考信号 */
                ref_signal[i] = (float)(srio_data_addr_ref[i] * factor);
                /* 处理每个分配的轴 */
                axis_signal[i] = (float)srio_data_addr_core[i] * factor;
            }

            if ((core_id == 1) || (core_id == 2) || (core_id == 3)) // 仅对前两个核执行MAD滤波
                mad_axis_signal[core_id - 1] = lowpass_filter_process(core_id - 1, axis_signal, DATA_BLOCK_SIZE);

            ret = adaptive_filter_process(ref_signal, axis_signal, g_twiddle_fft_LMS, g_twiddle_ifft_LMS,
                                          FFT_NUM_LMS, &g_shared_results.algorithm_amplitude[core_id - 1], &lms_phase[core_id - 1], axis_states);

            shared_inv(&g_lms_threshold, sizeof(float));
            if (g_shared_results.algorithm_amplitude[core_id - 1] > g_lms_threshold)
            {
                lms_result_counter[core_id - 1]++;
                if (g_lms_threshold != 0)
                    targetStatus.targetSNR = g_shared_results.algorithm_amplitude[core_id - 1] / g_lms_threshold;
            }
            else
                lms_result_counter[core_id - 1] = 0;

            shared_wb(&g_shared_results.algorithm_amplitude[core_id - 1], sizeof(float));
            shared_wb(&lms_result_counter[core_id - 1], sizeof(uint8_t));
        }
        else if (devParams.algorithmMode == 1)
        {
        	float *history = slip_history[core_id - 1];
            Cache_inv((void *)srio_data_addr_core, single_data_size, Cache_Type_ALL, CACHE_WAIT);
            if (devParams.fft_time_window == 2)
            {
                uint32_t sample_count = DATA_BLOCK_SIZE / 2;
                for (i = 0; i < (int)sample_count; i++)
                {
                    /* FFT path: down-sample by 2 so 65536-point sliding FFT accumulates ~2s data. */
                    axis_signal_new[i] = (float)srio_data_addr_core[i * 2] * factor;
                }
                /* MAD path: always use original 4096 points at 50kHz before low-pass to ~100Hz. */
                if ((core_id == 1) || (core_id == 2) || (core_id == 3))
                {
                    for (i = 0; i < DATA_BLOCK_SIZE; i++)
                    {
                        axis_signal[i] = (float)srio_data_addr_core[i] * factor;
                    }
                    mad_axis_signal[core_id - 1] = lowpass_filter_process(core_id - 1, axis_signal, DATA_BLOCK_SIZE);
                    shared_wb(&mad_axis_signal[core_id - 1], sizeof(float));
                }

                g_shared_results.algorithm_amplitude[core_id - 1] = sliding_fft_max_mag(FFT_NUM_SLIP, sample_count,
                		history, g_twiddle_fft_SLIP, g_fft_input[core_id - 1], g_fft_output[core_id - 1], axis_signal_new);
            }
            else
            {
                for (i = 0; i < DATA_BLOCK_SIZE; i++)
                {
                    /* 处理每个分配的轴 */
                    axis_signal_new[i] = (float)srio_data_addr_core[i] * factor;
                }
                if ((core_id == 1) || (core_id == 2) || (core_id == 3))
                {
                    mad_axis_signal[core_id - 1] = lowpass_filter_process(core_id - 1, axis_signal_new, DATA_BLOCK_SIZE);
                    shared_wb(&mad_axis_signal[core_id - 1], sizeof(float));
                }
                g_shared_results.algorithm_amplitude[core_id - 1] = sliding_fft_max_mag(FFT_NUM_SLIP, DATA_BLOCK_SIZE,
                		history, g_twiddle_fft_SLIP, g_fft_input[core_id - 1], g_fft_output[core_id - 1], axis_signal_new);
            }

            shared_inv(&g_fft_threshold, sizeof(float));
            if (g_shared_results.algorithm_amplitude[core_id - 1] > g_fft_threshold)
            {
                fft_result_counter[core_id - 1]++;
                if (g_fft_threshold != 0)
                    targetStatus.targetSNR = g_shared_results.algorithm_amplitude[core_id - 1] / g_fft_threshold;
            }
            else
            {
                fft_result_counter[core_id - 1] = 0;
            }
            shared_wb(&g_shared_results.algorithm_amplitude[core_id - 1], sizeof(float));
            shared_wb(&fft_result_counter[core_id - 1], sizeof(uint8_t));
        }

        __asm(" NOP 5 ");

        // t_end = _itoll(TSCH, TSCL);

        // t_cost = (t_end - t_start);
        // 标记本核完成
        srio_data_available[core_id] = 0;
        shared_wb(&srio_data_available[core_id], sizeof(uint32_t));
        results_ready[core_id] = 1;
        shared_wb(&results_ready[core_id], sizeof(uint32_t));

        while (1)
        {
            shared_inv(&results_ready[core_id], sizeof(uint32_t));
            if (results_ready[core_id] == 0)
                break;
            // Task_yield();
            __asm(" NOP 5 ");
        }
    }
}
