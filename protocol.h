#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "srio.h"

// Protocol constant definitions

#define PROTOCOL_BUFFER_SIZE 2048
#define MAX_DATA_LEN         (PROTOCOL_BUFFER_SIZE - 28)  // 信息单元内容最大长度
#define RESULT_NUM_ONCE_READ 50

// 网络报文帧头常量
#define NET_FRAME_HEAD       0xA5A5

#define NET_FRAME_HEAD_LEN   2
#define NET_TOTAL_LEN_LEN    2
#define NET_TIMESTAMP_LEN    8      // 年1+月1+日1+时1+分1+秒1+毫秒2 = 8
#define NET_FLAGS_LEN        1
#define NET_SEQ_NUM_LEN      1
#define NET_ACK_NUM_LEN      1
#define NET_RESERVED_LEN     1
#define NET_IP_LEN           4

#define NET_CRC_LEN          2
#define NET_INFO_UNIT_MIN_LEN 4     // 信息单元标识1+类型1+信息单元内容2，至少4字节
#define NET_FRAME_MIN_LEN     28 //24+2+2 // 最小帧长度

// 报文标志位定义
#define NET_FLAG_NEED_ACK    (1 << 0)
#define NET_FLAG_IS_ACK      (1 << 1)
#define NET_FLAG_RETRY_MASK  (3 << 2)
#define NET_FLAG_RETRY_SHIFT 2

// 信息单元类型
#define INFO_HEARTBEAT       0x00    // 心跳
#define INFO_TYPE_CMD        0x01    // 指令
#define INFO_TYPE_RESP       0x02    // 回令
#define INFO_TYPE_DATA       0x03    // 数据（用于内记结果数据包）

#define HEARTBEAT_STATUS_OK       0x10

// 新协议网络帧头结构体
#pragma pack(push, 1)
typedef struct {
    uint16_t frameHead;      // 0xA5A5
    uint16_t totalLen;       // 总长度 = 24(帧头) + N(N为信息单元内容长度) + 2 (CRC长度)
    uint8_t  year;           // 年-2000
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint16_t millisec;
    uint8_t  flags;          // 报文标志
    uint8_t  seqNum;         // 序列号
    uint8_t  ackNum;         // 确认号
    uint8_t  reserved;       // 备用
    uint32_t srcIp;          // 信源IP（网络字节序）
    uint32_t dstIp;          // 信宿IP（网络字节序）
    uint8_t  infoId;         // 信息单元标识
    uint8_t  infoType;       // 信息单元类型
    // uint8_t  infoContent[];  // 信息单元内容，变长
    //uint16_t crc;            // CRC校验码
} NetFrameHeader;
#pragma pack(pop)

// Command code definitions
#define CMD_STANDBY         0x01        // Standby command
#define CMD_ENABLE          0x02        // Enable command
#define CMD_TIME_SYNC       0x03        // Time sync command
#define CMD_SELF_CHECK      0x04        // Self-check command
#define CMD_PARAM_SET       0x05        // Parameter set command
#define CMD_PARAM_READ      0x06        // Parameter read command
#define CMD_DETECT_QUERY    0x07        // Detection result query command
#define CMD_HEARTBEAT       0x08        // Heartbeat query command
#define CMD_DATA_CLEAR      0x09        // Data clear command
#define CMD_RESULT_READ     0x0A        // Result data read command
#define CMD_COLLECT_MODE    0x0B        // Data collection mode command

// Status code definitions
#define STATUS_SUCCESS      0x00        // Execution succeeded
#define STATUS_PARAM_ERR    0x01        // Parameter error
#define STATUS_CRC_ERR      0x02        // Checksum failure
#define STATUS_UNKNOWN_ERR  0xFF        // Unknown error

// Device status
#define DEV_STATUS_STANDBY  0x00        // Standby status
#define DEV_STATUS_ENABLE   0x01        // Enable status

#define TARGET_FOUND        0x01        // Target found
#define NO_TARGET_FOUND     0x00        // No target found



#define SELF_CHECK_NORMAL 0xE5
#define SELF_CHECK_ERROR 0xE6

// Function declarations
#ifdef __cplusplus
extern "C" {
#endif

// Protocol handling functions
int32_t protocol_parse_frame(uint8_t *recv_buf, uint16_t recv_len,
                             uint8_t *cmd_code, uint8_t *info_type, uint8_t *data_buf, uint16_t *data_len);

uint16_t protocol_build_frame(uint8_t *send_buf, uint8_t cmd_code,
                              uint8_t *data_buf, uint16_t data_len,
                              uint8_t info_type, uint8_t seq_num, uint8_t ack_num, uint8_t flags);

uint16_t protocol_process_command(uint8_t cmd_code, const uint8_t *req_data,
                                 uint8_t  req_len, uint8_t *resp_data, uint16_t *resp_len,
                                 uint16_t resp_buf_size);

// Device status management
uint8_t protocol_get_device_status(void);
void protocol_set_device_status(uint8_t status);
void protocol_get_device_params(DeviceParams *params);
void protocol_set_device_params(DeviceParams *params);
uint8_t protocol_get_detect_result(void);
void protocol_set_detect_result(uint8_t result);

//extern volatile uint32_t total_restore_frames;
extern volatile uint32_t total_send_times;
extern volatile uint32_t process_frame_counter;

#ifdef __cplusplus
}
#endif

#endif // PROTOCOL_H
