#include <string.h>
#include <stdio.h>
#include <ti/sysbios/knl/Task.h>
#include "protocol.h"
#include "shared_mem.h"
#include "nand_op.h"

// Device global state
static uint16_t devStatus = DEV_STATUS_STANDBY;
volatile float madThresholdSquare;

volatile TargetStatus targetStatus;

#pragma DATA_SECTION(resultPtr, ".shared_msmc")
float *resultPtr = NULL;

// 在 protocol.c 顶部添加
// #pragma DATA_SECTION(total_restore_frames, ".shared_msmc")
// volatile uint32_t total_restore_frames = 0;

#pragma DATA_SECTION(total_send_times, ".shared_msmc")
volatile uint32_t total_send_times = 0;

#pragma DATA_SECTION(first_frame_timestamp, ".shared_msmc")
unsigned long long first_frame_timestamp = 0;

uint64_t timestamp;

extern void flush_all_results(void);

// CRC-16/CCITT查找表（多项式0x1021，初始值0xFFFF）
static const uint16_t crc16_ccitt_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0};
// Optimized CRC calculation function
static inline uint16_t calculate_crc16_fast(const uint8_t *data, uint32_t length)
{
    uint16_t crc = 0xFFFF;

    while (length--)
    {
        crc = (crc << 8) ^ crc16_ccitt_table[((crc >> 8) ^ *data++) & 0xFF];
    }
    return crc;
}

// Parse received frame
int32_t protocol_parse_frame(uint8_t *recv_buf, uint16_t recv_len,
                             uint8_t *cmd_code, uint8_t *info_type, uint8_t *data_buf, uint16_t *data_len)
{
    if (recv_buf == NULL || cmd_code == NULL || info_type == NULL || data_buf == NULL || data_len == NULL)
    {
        printf("Parse error: Invalid input parameters\n");
        return -1;
    }

    // Check minimum length: header(2) + length(1) + cmd(1) + checksum(1) + tail(2) = 7 bytes
    if (recv_len < NET_FRAME_MIN_LEN)
    {
        printf("Parse error: Frame too short (%d bytes, min %d bytes)\n", recv_len, NET_FRAME_MIN_LEN);
        return -2;
    }

    // Check frame header
    NetFrameHeader *header = (NetFrameHeader *)recv_buf;
    // 检查帧头
    if (header->frameHead != NET_FRAME_HEAD)
    {
        printf("Parse error: Invalid frame header 0x%04X (expected 0x%04X)\n",
               header->frameHead, NET_FRAME_HEAD);
        return -3;
    }
    // 检查总长度
    uint16_t total_len = header->totalLen;
    if (total_len != recv_len)
    {
        printf("Parse error: Length mismatch: header says %d, received %d\n", total_len, recv_len);
        return -4;
    }
    // 信息单元内容长度 = total_len - 26
    uint16_t info_content_len = total_len - 26 - 2; // 减去帧头长度和CRC长度
    if (info_content_len > MAX_DATA_LEN)
    {
        printf("Parse error: Info content length %d exceeds max %d\n", info_content_len, MAX_DATA_LEN);
        return -5;
    }
    // 提取信息单元内容（即原来的命令帧数据）
    uint8_t *info_content = (uint8_t *)(recv_buf + sizeof(NetFrameHeader));
    if (info_content_len < 1)
    {
        printf("Parse error: Info content too short\n");
        return -6;
    }
    // 信息单元内容的第一字节是原命令码
    *cmd_code = header->infoId;    // 沿用原命令码作为信息单元标识
    *info_type = header->infoType; // 信息单元类型
    *data_len = info_content_len;  // 减去命令码和信息单元类型占用的2字节
    if (*data_len > 0)
        memcpy(data_buf, info_content, *data_len);
    // 验证 CRC16（校验范围：从帧头到信息单元内容结束，不包括CRC本身）
    uint16_t calc_crc = calculate_crc16_fast(recv_buf, total_len - NET_CRC_LEN);
    uint16_t recv_crc = *(uint16_t *)(recv_buf + total_len - NET_CRC_LEN);
    if (calc_crc != recv_crc)
    {
        printf("Parse error: CRC mismatch (calc 0x%04X, got 0x%04X)\n", calc_crc, recv_crc);
        return -7;
    }
    return 0;
}

// Build transmit frame
uint16_t protocol_build_frame(uint8_t *send_buf, uint8_t cmd_code,
                              uint8_t *data_buf, uint16_t data_len,
                              uint8_t info_type, uint8_t seq_num, uint8_t ack_num, uint8_t flags)
{

    if (send_buf == NULL)
    {
        printf("Error: send_buf is NULL\n");
        return 0;
    }

    // 信息单元内容 = cmd_code(1) + info_type(1) + data_buf(data_len)

    if (data_len > MAX_DATA_LEN)
    {
        printf("Warning: Truncating info content from %d to %d\n", data_len, MAX_DATA_LEN);
        return 0;
    }
    uint16_t total_len = sizeof(NetFrameHeader) + data_len + NET_CRC_LEN; // 24字节固定头 + 信息单元内容 + CRC校验
    if (total_len > PROTOCOL_BUFFER_SIZE)
    {
        printf("Error: Frame too large for buffer\n");
        return 0;
    }

    NetFrameHeader *header = (NetFrameHeader *)send_buf;
    memset(header, 0, sizeof(NetFrameHeader));
    // 时戳需要从外部获取，这里先用0填充
    header->frameHead = NET_FRAME_HEAD;
    header->totalLen = total_len;
    header->year = 0;
    header->month = 0;
    header->day = 0;
    header->hour = 0;
    header->minute = 0;
    header->second = 0;
    header->millisec = 0;
    header->flags = flags;
    header->seqNum = seq_num;
    header->ackNum = ack_num;
    header->reserved = 0;
    // IP地址需要外部设置，这里先填0
    header->srcIp = 0;
    header->dstIp = 0;
    header->infoId = cmd_code;    // 信息单元标识沿用原命令码
    header->infoType = info_type; // 0x01指令，0x02回令,0X00心跳
    // 填充信息单元内容
    uint8_t *info_content = send_buf + sizeof(NetFrameHeader);
    if (data_len > 0 && data_buf != NULL)
        memcpy(info_content, data_buf, data_len);
    // 计算CRC
    uint16_t crc = calculate_crc16_fast(send_buf, total_len - NET_CRC_LEN);
    *(uint16_t *)(send_buf + total_len - NET_CRC_LEN) = crc;
    return total_len;
}

// Handle standby command
static uint8_t handle_standby_cmd(const uint8_t *req_data, uint8_t req_len,
                                  uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 2)
    {
        return STATUS_PARAM_ERR;
    }
    devStatus = DEV_STATUS_STANDBY;

    // nandflash_write(NAND_FRAME_NUMBER_OFFSET, sizeof(uint32_t), (uint8_t *)&process_frame_counter);

    uint16_t execution_result = STATUS_SUCCESS;
    memcpy((uint16_t *)resp_data, &execution_result, sizeof(uint16_t));
    *resp_len = 2;
    return STATUS_SUCCESS;
}

// Handle enable command
static uint8_t handle_enable_cmd(const uint8_t *req_data, uint8_t req_len,
                                 uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 2)
    {
        return STATUS_PARAM_ERR;
    }
    uint16_t enable_status, execution_result;
    memcpy(&enable_status, req_data, sizeof(uint16_t));

    if (enable_status == 0x01)
    {
        timestamp = get_ntp_timestamp(); // 启用时同步时间
        shared_wb(&timestamp, sizeof(uint64_t));

        #ifdef RESTORE_NAND
        if (save_timestamp(timestamp) != 0)
        {
            printf("Warning: failed to save timestamp\n");
        }
        #endif

        memset((void *)fpgaParams.frameHead, 0x5A, 8);
        fpgaParams.ADC_sampEnable = DEV_STATUS_ENABLE;
        fpgaParams.DAC_outEnable = DEV_STATUS_ENABLE;
        fpgaParams.DAC_freq = g_lms_f0 / 10;
        fpgaParams.DAC_amp = devParams.effectiveValue * 1000 / 39;
        // int i;
        // for (i = 0; i < 51; i++)
        //     fpgaParams.reserved[i] = i;

        srio_send();

        #ifdef RESTORE_RAM
        shared_inv(resultPtr, sizeof(resultPtr)); // 使缓存无效，确保数据一致
        if(resultPtr == NULL)
            resultPtr = (float *)malloc(MAX_RESULE_NUMBER * sizeof(SharedResults_t));
        if (resultPtr == NULL)
        {
            execution_result = STATUS_UNKNOWN_ERR;
            printf("Error: failed to allocate memory for resultPtr\n");
        }
        else
        {
            //memset(resultPtr, 0, MAX_RESULE_NUMBER * sizeof(SharedResults_t)); // 初始化内存
            printf("Memory allocated successfully, resultPtr = %p\n", resultPtr);
            execution_result = STATUS_SUCCESS;
            shared_wb(resultPtr, sizeof(resultPtr));     
        }
        #endif

        #ifdef RESTORE_NAND
        if (erase_result_block() != 0)
        {
            printf("Error: failed to erase result area, some blocks may be bad.\n");
            // 可选择返回错误或继续，但继续可能写入失败
            execution_result = STATUS_UNKNOWN_ERR;
        }
        else
        {
            execution_result = STATUS_SUCCESS;
        }
        #endif

        process_frame_counter = 0;
        shared_wb(&process_frame_counter, sizeof(uint32_t));

        devStatus = DEV_STATUS_ENABLE;
        printf("Device enabled\n");
    }
    else if (enable_status == 0x00)
    {
        // int i;
        memset((void *)fpgaParams.frameHead, 0x5A, 8);
        fpgaParams.ADC_sampEnable = DEV_STATUS_STANDBY;
        fpgaParams.DAC_outEnable = DEV_STATUS_STANDBY;
        fpgaParams.DAC_freq = g_lms_f0 / 10;
        fpgaParams.DAC_amp = devParams.effectiveValue * 1000 / 39;
        // for (i = 0; i < 51; i++)
        //     fpgaParams.reserved[i] = i;
        srio_send();

        #ifdef RESTORE_RAM
        shared_inv(&process_frame_counter, sizeof(process_frame_counter));
        printf("Frame counter saved: %u\n", process_frame_counter);
        #endif

        #ifdef RESTORE_NAND     
        flush_all_results();

        // 使用专用函数保存帧计数
        if (save_frame_counter(process_frame_counter) == 0)
        {
            printf("Frame counter saved: %u\n", process_frame_counter);
        }
        else
        {
            printf("Failed to save frame counter\n");
        }
        #endif
        
        execution_result = STATUS_SUCCESS;

        devStatus = DEV_STATUS_STANDBY;
        printf("Device standby\n");
    }
    else
    {
        execution_result = STATUS_PARAM_ERR;
    }

    memcpy((uint16_t *)resp_data, &execution_result, sizeof(uint16_t));
    *resp_len = 2;
    printf("handle_enable_cmd: before return.\n");
    return STATUS_SUCCESS;
}

// Handle self-check command
static uint8_t handle_self_check_cmd(const uint8_t *req_data, uint8_t req_len,
                                     uint8_t *resp_data, uint16_t *resp_len)
{
    uint16_t execution_result;
    if (req_len != 2)
    {
        return STATUS_PARAM_ERR;
    }

    uint16_t self_check_result = 0xE5; // Self-check normal
    execution_result = STATUS_SUCCESS;
    memcpy((uint16_t *)resp_data, &execution_result, sizeof(uint16_t));
    memcpy((uint16_t *)(resp_data + 2), &self_check_result, sizeof(uint16_t));

    *resp_len = 4;
    printf("Self-check executed, result: 0x%04X\n", self_check_result);
    return STATUS_SUCCESS;
}

// Handle parameter set command
DeviceParams *newParams = NULL;
static uint8_t handle_param_set_cmd(const uint8_t *req_data, uint8_t req_len,
                                    uint8_t *resp_data, uint16_t *resp_len)
{
    uint16_t execution_result;
    // if (req_len != sizeof(DeviceParams))
    if (req_len != 54)
    {
        execution_result = STATUS_PARAM_ERR;
        memcpy((uint16_t *)resp_data, &execution_result, sizeof(uint16_t));
        *resp_len = 2;
        return STATUS_PARAM_ERR;
    }

    memcpy(&devParams, req_data, sizeof(DeviceParams));

    g_mad_threshold = devParams.MADThreshold / 1000.0f;
    g_fft_threshold = devParams.FFTThreshold / 1000.0f;
    g_lms_threshold = devParams.LMSThreshold / 1000.0f;
    g_lms_step_size = devParams.LMSStep / 1000.0f;
    g_lms_order = devParams.LMSOrder;
    g_lms_f0 = devParams.signalFrequency;

    shared_wb(&g_mad_threshold, sizeof(float));
    shared_wb(&g_fft_threshold, sizeof(float));
    shared_wb(&g_lms_threshold, sizeof(float));
    shared_wb(&g_lms_step_size, sizeof(float));
    shared_wb(&g_lms_order, sizeof(uint8_t));
    shared_wb(&g_lms_f0, sizeof(uint16_t));

    // Return original parameters + status code
    // memcpy(resp_data, req_data, 8);
    execution_result = STATUS_SUCCESS;
    memcpy((uint16_t *)resp_data, &execution_result, sizeof(uint16_t));
    *resp_len = 2;

    printf("Parameters set: mode=%d, fft_time_window=%d, MADThreshold=%d, LMSThreshold=%d, FFTThreshold=%d, "
           "MADTimes=%d, LMSTimes=%d, FFTTimes=%d, "
           "signalFrequency=%d, effectiveValue=%d, LMSOrder=%d, LMSStep=%d, MADStopFreq=%d\n",
           devParams.algorithmMode, devParams.fft_time_window, devParams.MADThreshold, devParams.LMSThreshold,
           devParams.FFTThreshold, devParams.MADTimes, devParams.LMSTimes, devParams.FFTTimes,
           devParams.signalFrequency, devParams.effectiveValue, devParams.LMSOrder, devParams.LMSStep, devParams.MADStopFreq);

    return STATUS_SUCCESS;
}

// Handle parameter read command
static uint8_t handle_param_read_cmd(const uint8_t *req_data, uint8_t req_len,
                                     uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 2)
    {
        return STATUS_PARAM_ERR;
    }
    memcpy((DeviceParams *)resp_data, &devParams, sizeof(DeviceParams));

    *resp_len = 54; // Return full size of DeviceParams

    printf("Parameters read: mode=%d, fft_time_window=%d, MADThreshold=%d, LMSThreshold=%d, FFTThreshold=%d, "
           "MADTimes=%d, LMSTimes=%d, FFTTimes=%d, "
           "signalFrequency=%d, effectiveValue=%d, LMSOrder=%d, LMSStep=%d, MADStopFreq=%d\n",
           devParams.algorithmMode, devParams.fft_time_window, devParams.MADThreshold, devParams.LMSThreshold,
           devParams.FFTThreshold, devParams.MADTimes, devParams.LMSTimes, devParams.FFTTimes,
           devParams.signalFrequency, devParams.effectiveValue, devParams.LMSOrder, devParams.LMSStep, devParams.MADStopFreq);
    return STATUS_SUCCESS;
}

// Handle detection result query
static uint8_t handle_detect_query_cmd(const uint8_t *req_data, uint8_t req_len,
                                       uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 2)
    {
        return STATUS_PARAM_ERR;
    }
    uint8_t i;
    // Fetch actual detection result
    // For example: obtain detection result from algorithm module

    if (mad_result_counter > devParams.MADTimes)
    { // MAD result in bit1, LMS/FFT result in bit0
        if (devParams.algorithmMode == 0)
        {
            for (i = 0; i < CAL_CORE_NUM; i++)
            {
                if (lms_result_counter[i] > devParams.LMSTimes)
                {
                    targetStatus.targetStatus = TARGET_FOUND; // MAD + LMS both detect
                    break;
                }
            }
            if (i == CAL_CORE_NUM)
            {
                targetStatus.targetStatus = NO_TARGET_FOUND; // No target detected
            }
        }
        else if (devParams.algorithmMode == 1)
        {
            for (i = 0; i < CAL_CORE_NUM; i++)
            {
                if (fft_result_counter[i] > devParams.FFTTimes)
                {
                    targetStatus.targetStatus = TARGET_FOUND; // MAD + FFT both detect
                    break;
                }
            }
            if (i == CAL_CORE_NUM)
            {
                targetStatus.targetStatus = NO_TARGET_FOUND; // No target detected
            }
        }
    }
    else
    {
        targetStatus.targetStatus = NO_TARGET_FOUND; // No target detected
    }

    if (targetStatus.targetStatus == TARGET_FOUND)
        targetStatus.targetSNR = 1; // Example SNR value, should be calculated based on actual detection results;
    else
        targetStatus.targetSNR = 0;
    memcpy((uint16_t *)resp_data, (uint16_t *)&targetStatus, sizeof(TargetStatus));
    *resp_len = sizeof(TargetStatus); // Return full size of TargetStatus

    printf("Detect result query: 0x%02X, SNR=0x%02X\n", resp_data[0], resp_data[1]);

    return STATUS_SUCCESS;
}

// Handle heartbeat command
static uint8_t handle_heartbeat_cmd(const uint8_t *req_data, uint8_t req_len,
                                    uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 0)
    {
        return STATUS_PARAM_ERR;
    }

    resp_data[0] = HEARTBEAT_STATUS_OK;
    *resp_len = 2;
    printf("Heartbeat: status=0x%02X\n", resp_data[0]);

    return STATUS_SUCCESS;
}

// Handle time sync command
static uint8_t handle_time_sync_cmd(const uint8_t *req_data, uint8_t req_len,
                                    uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 8)
    {
        return STATUS_PARAM_ERR;
    }

    // Parse time data
    uint16_t year = req_data[0];
    uint8_t month = req_data[1];
    uint8_t day = req_data[2];
    uint8_t hour = req_data[3];
    uint8_t minute = req_data[4];
    uint8_t second = req_data[5];
    uint8_t millisec = req_data[6] << 8 | req_data[7];

    // TODO: Set RTC time here
    // set_rtc_time(year, month, day, hour, minute, second, millisec);

    // Return original time data + status code
    memcpy(resp_data, req_data, 8);
    resp_data[8] = STATUS_SUCCESS;
    *resp_len = 10;

    printf("Time sync: %04d-%02d-%02d %02d:%02d:%02d.%04d\n",
           year, month, day, hour, minute, second, millisec);

    return STATUS_SUCCESS;
}

// Handle data clear command
static uint8_t handle_data_clear_cmd(const uint8_t *req_data, uint8_t req_len,
                                     uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 2)
    {
        return STATUS_PARAM_ERR;
    }

    // TODO: Clear data logic
    // clear_detection_data();

    uint16_t execution_result = STATUS_SUCCESS;
    memcpy((uint16_t *)resp_data, &execution_result, sizeof(uint16_t));
    *resp_len = 2;

    printf("Data cleared\n");

    return STATUS_SUCCESS;
}

// Handle result data readback command
static uint8_t handle_result_read_cmd(const uint8_t *req_data, uint8_t req_len,
                                      uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 2)
        return STATUS_PARAM_ERR;

    uint16_t resp_pos = 0;
    uint16_t execution_result = STATUS_SUCCESS;
    memcpy(resp_data, &execution_result, sizeof(execution_result));
    resp_pos += 2;
    uint32_t total_restore_frames = 0;
    // 正确读取帧数
    // uint32_t j = 300;
    #ifdef RESTORE_NAND  
    // nandflash_write(NAND_FRAME_NUMBER_OFFSET, sizeof(j), (uint8_t *)&j);
    load_frame_counter(&total_restore_frames);
    #endif

    #ifdef RESTORE_RAM
    shared_inv(&process_frame_counter, sizeof(process_frame_counter));
    total_restore_frames = process_frame_counter;
    #endif

    memcpy(resp_data + resp_pos, (void *)&total_restore_frames, sizeof(total_restore_frames));
    resp_pos += sizeof(total_restore_frames);
    printf("Total restore frames: %u\n", total_restore_frames);

    total_send_times = (total_restore_frames + RESULT_NUM_ONCE_READ - 1) / RESULT_NUM_ONCE_READ;
    memcpy(resp_data + resp_pos, (void *)&total_send_times, sizeof(total_send_times));
    resp_pos += sizeof(total_send_times);
    printf("Total send times: %u\n", total_send_times);

    #ifdef RESTORE_NAND
    load_timestamp(&first_frame_timestamp);
    #endif
    #ifdef RESTORE_RAM
    first_frame_timestamp = timestamp;
    #endif
    memcpy(resp_data + resp_pos, (void *)&first_frame_timestamp, sizeof(first_frame_timestamp));
    resp_pos += sizeof(first_frame_timestamp);

    *resp_len = resp_pos;
    return STATUS_SUCCESS;
}

// Handle data collection mode command
static uint8_t handle_collect_mode_cmd(const uint8_t *req_data, uint8_t req_len,
                                       uint8_t *resp_data, uint16_t *resp_len)
{
    if (req_len != 2)
    {
        return STATUS_PARAM_ERR;
    }

    uint8_t mode = req_data[0];
    uint8_t param = req_data[1];

    // TODO: Set collection mode logic

    // Return status code
    resp_data[0] = STATUS_SUCCESS;
    *resp_len = 2;

    printf("Collection mode set: mode=%d, param=%d\n", mode, param);

    return STATUS_SUCCESS;
}

// Main command handler
uint16_t protocol_process_command(uint8_t cmd_code, const uint8_t *req_data,
                                  uint8_t req_len, uint8_t *resp_data, uint16_t *resp_len)
{
    uint16_t status = STATUS_SUCCESS;
    printf("Processing command 0x%02X, data length %d\n", cmd_code, req_len);

    switch (cmd_code)
    {
    case CMD_STANDBY:
        status = handle_standby_cmd(req_data, req_len, resp_data, resp_len);
        break;
    case CMD_ENABLE:
        status = handle_enable_cmd(req_data, req_len, resp_data, resp_len);
        printf("Enable command processed, status: 0x%02X\n", status);
        break;
    case CMD_TIME_SYNC:
        status = handle_time_sync_cmd(req_data, req_len, resp_data, resp_len);
        break;
    case CMD_SELF_CHECK:
        status = handle_self_check_cmd(req_data, req_len, resp_data, resp_len);
        break;
    case CMD_PARAM_SET:
        status = handle_param_set_cmd(req_data, req_len, resp_data, resp_len);
        break;
    case CMD_PARAM_READ:
        status = handle_param_read_cmd(req_data, req_len, resp_data, resp_len);
        break;
    case CMD_DETECT_QUERY:
        status = handle_detect_query_cmd(req_data, req_len, resp_data, resp_len);
        break;
    // case CMD_HEARTBEAT:
    //     status = handle_heartbeat_cmd(req_data, req_len, resp_data, resp_len);
    //     break;
    case CMD_DATA_CLEAR:
        status = handle_data_clear_cmd(req_data, req_len, resp_data, resp_len);
        break;
    case CMD_RESULT_READ:
        status = handle_result_read_cmd(req_data, req_len, resp_data, resp_len);
        break;
    case CMD_COLLECT_MODE:
        status = handle_collect_mode_cmd(req_data, req_len, resp_data, resp_len);
        break;
    default:
        printf("Unknown command: 0x%02X\n", cmd_code);
        *resp_len = 2;
        status = STATUS_UNKNOWN_ERR;
        memcpy((uint16_t *)resp_data, &status, sizeof(uint16_t));
        break;
    }

    if (status != STATUS_SUCCESS)
    {
        printf("Command 0x%02X failed with status 0x%02X\n", cmd_code, status);
        memcpy((uint16_t *)resp_data, &status, sizeof(uint16_t));
        *resp_len = sizeof(uint16_t);
    }
    return status;
}
