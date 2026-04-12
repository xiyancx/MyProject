/* C standard Header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c6x.h>

/* XDCtools Header files */
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

/* BIOS6 include */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/hal/Hwi.h>
#include <ti/sysbios/knl/Semaphore.h>

/* NDK include */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>
#include <ti/ndk/inc/tools/console.h>
#include <ti/ndk/inc/tools/servers.h>

#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/stkmain.h>

/* csl include */
#include <ti/csl/csl_psc.h>
#include <ti/csl/csl_pscAux.h>
#include <ti/csl/csl_chipAux.h>
#include <ti/csl/csl_bootcfgAux.h>
#include <ti/csl/csl_mdio.h>
#include <ti/csl/csl_mdioAux.h>

#include <ti/csl/cslr_device.h>

#ifdef C66_PLATFORMS
#include <ti/csl/csl_cpsgmii.h>
#include <ti/csl/csl_cpsgmiiAux.h>
#include <ti/sysbios/family/c66/Cache.h>
#endif

#include <errno.h>
#include <time.h>

#include "system/resource_mgr.h"
#include <xdc/runtime/Timestamp.h>

#include "net_init.h"
#include "protocol.h" // Added: include protocol header
#include "shared_mem.h"
#include "nand_op.h"

/* Common phy register */
#define PHY_IDENTIFIER_1_REG 2
#define PHY_IDENTIFIER_2_REG 3

/* Phy identification */
#define PHY_MOTORCOMM_IDENTIFIER 0x0
#define PHY_MOTORCOMM_MANUFACTURER 0x011a

/* YT8521SH phy register */
#define YT8521SH_EXT_REG_ADDR_OFFSET_REG 0x1E
#define YT8521SH_EXT_REG_DATA_REG 0x1F
#define YT8521SH_EXT_LED2_CFG 0xA00E
#define YT8521SH_EXT_LED1_CFG 0xA00D

/*
 * benchmark test data size = TEST_DATA_SIZE * MB
 * the unit is Byte
 */
#define TEST_DATA_SIZE 1024
#define MB (1024 * 1024)

// Added: protocol server port definition
#define PROTOCOL_SERVER_PORT 10001

// #define PROTOCOL_BUFFER_SIZE 256

// NTP server definition
#define NTP_SERVER_IP "192.168.1.7"
#define NTP_SERVER_PORT 123
#define NTP_PACKET_SIZE 48
#define NTP_TIMESTAMP_DELTA 2208988800u

typedef unsigned int socklen_t;

typedef struct
{
    uint8_t li_vn_mode;       // 8 bits: Leap Indicator, Version Number, Mode
    uint8_t stratum;          // 8 bits: Stratum
    uint8_t poll;             // 8 bits: Poll
    uint8_t precision;        // 8 bits: Precision
    uint32_t root_delay;      // 32 bits
    uint32_t root_dispersion; // 32 bits
    uint32_t ref_id;          // 32 bits
    uint32_t ref_ts_sec;      // 32 bits: Reference Timestamp (Seconds)
    uint32_t ref_ts_frac;     // 32 bits: Reference Timestamp (Fraction)
    uint32_t orig_ts_sec;     // 32 bits: Origin Timestamp (Seconds)
    uint32_t orig_ts_frac;    // 32 bits: Origin Timestamp (Fraction)
    uint32_t recv_ts_sec;     // 32 bits: Receive Timestamp (Seconds)
    uint32_t recv_ts_frac;    // 32 bits: Receive Timestamp (Fraction)
    uint32_t trans_ts_sec;    // 32 bits: Transmit Timestamp (Seconds)
    uint32_t trans_ts_frac;   // 32 bits: Transmit Timestamp (Fraction)
} NtpPacket;

// uint8_t send_buf[PROTOCOL_BUFFER_SIZE];
// uint8_t recv_buf[PROTOCOL_BUFFER_SIZE];
uint8_t g_send_buffer[DATA_TOTAL_SIZE]; // Global send buffer

uint32_t axis_payload_len = 14;

char *HostName = "tidsp";
char *LocalIPAddr = "192.168.2.69";
char *LocalIPMask = "255.255.255.0";
char *GatewayIP = "192.168.2.1";
char *DomainName = "demo.net";
char *DNSServer = "0.0.0.0";

/* Service Status Reports */
/* Here's a quick example of using service status updates */
char *TaskName[] = {"Telnet", "HTTP", "NAT", "DHCPS", "DHCPC", "DNS"};
char *StatusStr[] = {"Disabled", "Waiting", "IPTerm", "Failed", "Enabled"};
char *ReportStr[] = {"", "Running", "Updated", "Complete", "Fault"};
char *VerStr = "\nTCP/UDP Example Client\n";

/* Data server global variables */
// volatile uint8_t data_send_enabled = 0;
extern Semaphore_Handle srio_data_sem;
// e//xtern volatile uint32_t srio_data_available;

UINT8 DHCP_OPTIONS[] = {DHCPOPT_SERVER_IDENTIFIER, DHCPOPT_ROUTER};

HANDLE hEcho = 0, hEchoUdp = 0;

unsigned long long get_ntp_timestamp(void)
{
    // 假设我们定义 CMD_GET_NTP_TIME = 0x50
    SOCKET ntp_sock;
    struct sockaddr_in ntp_addr;
    NtpPacket packet;
    int recv_len;
    time_t unix_time;
    uint32_t ntp_time_sec;

    printf("Received NTP Sync/Enable Command\n");

    // 1. 创建 UDP Socket
    ntp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ntp_sock == INVALID_SOCKET)
    {
        printf("NTP: Create socket failed\n");
        // 发送错误响应
        return -1;
    }

    // 2. 配置服务器地址
    memset(&ntp_addr, 0, sizeof(ntp_addr));
    ntp_addr.sin_family = AF_INET;
    ntp_addr.sin_port = htons(NTP_SERVER_PORT);
    ntp_addr.sin_addr.s_addr = inet_addr(NTP_SERVER_IP);

    // 3. 构建 NTP 请求包 (48字节全0，仅修改第一个字节)
    memset(&packet, 0, sizeof(packet));
    packet.li_vn_mode = 0x1B; // LI=0, VN=3, Mode=3 (Client)

    // 4. 发送请求
    if (sendto(ntp_sock, (char *)&packet, sizeof(packet), 0,
               (struct sockaddr *)&ntp_addr, sizeof(ntp_addr)) <= 0)
    {
        printf("NTP: Send failed\n");
        fdClose(ntp_sock);
        // 发送错误响应
        return -1;
    }

    // 5. 接收响应 (设置超时防止死锁)
    struct timeval timeout;
    timeout.tv_sec = 5; // 3秒超时
    timeout.tv_usec = 0;
    setsockopt(ntp_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    recv_len = recvfrom(ntp_sock, (char *)&packet, sizeof(packet), 0, NULL, NULL);

    if (recv_len < 48)
    {
        printf("NTP: Receive error or timeout\n");
        fdClose(ntp_sock);
        // 发送错误响应
        return -1;
    }
    fdClose(ntp_sock);

    // 6. 解析时间 (关键步骤)
    // NTP 时间戳是大端序 (Big-Endian)，需要转换
    // 服务器发送时间戳 (Transmit Timestamp) 位于第 40-43 字节
    // 方法1: 使用 ntohl (推荐，如果 NDK 库支持)
    // ntp_time_sec = ntohl(packet.trans_ts_sec);

    // 方法2: 手动移位 (最稳妥，不依赖库函数)
    ntp_time_sec = (packet.trans_ts_sec >> 24) & 0xFF;
    ntp_time_sec |= (packet.trans_ts_sec >> 8) & 0xFF00;
    ntp_time_sec |= (packet.trans_ts_sec << 8) & 0xFF0000;
    ntp_time_sec |= (packet.trans_ts_sec << 24) & 0xFF000000;

    // 转换为 Unix 时间戳 (1970基准)
    unix_time = ntp_time_sec - NTP_TIMESTAMP_DELTA;

    printf("NTP Sync Success! Unix Time: %lu\n", (unsigned long)unix_time);

    // --- 这里进行 DSP 时钟同步 ---
    // SyncDspClock(unix_time);
    return (unsigned long long)unix_time;
}

uint64_t get_cycles(void)
{
    uint64_t cycles;
    cycles = _itoll(TSCH, TSCL); // 将高32位(TSCH)和低32位(TSCL)合并为一个64位整数
    return cycles;
}
int queue_manager_init(void)
{
    QMSS_CFG_T qmss_cfg;
    CPPI_CFG_T cppi_cfg;

    /* Initialize the components required to run this application:
     *  (1) QMSS
     *  (2) CPPI
     *  (3) Packet Accelerator
     */
    /* Initialize QMSS */
    if (CSL_chipReadDNUM() == 0)
    {
        qmss_cfg.master_core = 1;
    }
    else
    {
        qmss_cfg.master_core = 0;
    }
    qmss_cfg.max_num_desc = MAX_NUM_DESC;
    qmss_cfg.desc_size = MAX_DESC_SIZE;
    qmss_cfg.mem_region = Qmss_MemRegion_MEMORY_REGION0;

    if (res_mgr_init_qmss(&qmss_cfg) != 0)
    {
        printf("Failed to initialize the QMSS subsystem \n");
        return -1;
    }
    else
    {
        printf("QMSS successfully initialized \n");
    }

    /* Initialize CPPI */
    if (CSL_chipReadDNUM() == 0)
    {
        cppi_cfg.master_core = 1;
    }
    else
    {
        cppi_cfg.master_core = 0;
    }
    cppi_cfg.dma_num = Cppi_CpDma_PASS_CPDMA;
    cppi_cfg.num_tx_queues = NUM_PA_TX_QUEUES;
    cppi_cfg.num_rx_channels = NUM_PA_RX_CHANNELS;
    if (res_mgr_init_cppi(&cppi_cfg) != 0)
    {
        printf("Failed to initialize CPPI subsystem \n");
        return -1;
    }
    else
    {
        printf("CPPI successfully initialized \n");
    }

    if (res_mgr_init_pass() != 0)
    {
        printf("Failed to initialize the Packet Accelerator \n");
        return -1;
    }
    else
    {
        printf("PA successfully initialized \n");
    }

    return 0;
}

void init_sgmii(uint32_t macPortNum)
{
    int32_t wait_time;
    CSL_SGMII_ADVABILITY sgmiiCfg;
    CSL_SGMII_STATUS sgmiiStatus;

    /* Configure the SERDES, set MPY as 10x mode */
    CSL_BootCfgSetSGMIIConfigPLL(0x00000051);

    /* delay 100 cycles */
    cpu_delaycycles(100);
    /*
     * ENRX: 0x1 - Enable Receive Channel
     * RATE: 0x10 - PLL output clock rate by a factor of 2x
     * ALIGN: 0x01 - Enable Comma alignment
     * EQ : 0xC - Set to 1100b when RATE = 0x10
     * ENOC: 0x1 - Enable offset compensation
     */
    CSL_BootCfgSetSGMIIRxConfig(macPortNum, 0x00700621);
    /*
     * ENRX: 0x1 - Enable Transmit channel
     * RATE: 0x10 - PLL output clock rate by a factor of 2x
     * INVPAIR: 0x0 - Normal polarity
     */
    CSL_BootCfgSetSGMIITxConfig(macPortNum, 0x000108A1);

    /* Wait sgmii serdes configure complete time set as 1ms */
    wait_time = 1000;
    while (wait_time)
    {
        CSL_SGMII_getStatus(macPortNum, &sgmiiStatus);
        if (sgmiiStatus.bIsLocked == 1)
        {
            break;
        }
        else
        {
            /* delay 1000 cycles */
            cpu_delaycycles(1000);
            wait_time--;
        }
    }

    if (wait_time == 0)
    {
        printf("configure sgmii0 serdes time out!\n");
        return;
    }

    /* Reset the port before configuring it */
    CSL_SGMII_doSoftReset(macPortNum);
    while (CSL_SGMII_getSoftResetStatus(macPortNum) != 0)
        ;

    /*
     * Hold the port in soft reset and set up
     * the SGMII control register:
     *      (1) Disable Master Mode
     *      (2) Enable Auto-negotiation
     */
    CSL_SGMII_startRxTxSoftReset(macPortNum);
    CSL_SGMII_disableMasterMode(macPortNum);
    CSL_SGMII_enableAutoNegotiation(macPortNum);
    CSL_SGMII_endRxTxSoftReset(macPortNum);

    /*
     * Setup the Advertised Ability register for this port:
     *      (1) Enable Full duplex mode
     *      (2) Enable Auto Negotiation
     *      (3) Enable the Link
     */
    sgmiiCfg.linkSpeed = CSL_SGMII_100_MBPS; // CSL_SGMII_1000_MBPS;
    sgmiiCfg.duplexMode = CSL_SGMII_FULL_DUPLEX;
    sgmiiCfg.bLinkUp = 1;
    CSL_SGMII_setAdvAbility(macPortNum, &sgmiiCfg);

    do
    {
        CSL_SGMII_getStatus(macPortNum, &sgmiiStatus);
    } while (sgmiiStatus.bIsLinkUp != 1);

    /* All done with configuration. Return Now. */
    return;
}
static uint8_t data_buf[MAX_DATA_LEN];
static uint8_t recv_buf[PROTOCOL_BUFFER_SIZE];
static uint8_t send_buf[PROTOCOL_BUFFER_SIZE]; // 假设这是你定义的发送缓冲区
static uint8_t resp_data[64];
// long long resultFrameCounter= 0;
void ProtocolServerTask(void)
{
    SOCKET sock; // UDP 只需要一个 Socket，不需要 client_sock
    struct sockaddr_in server_addr, client_addr;
    int ret;

    uint8_t cmd_code, cmd_type;
    uint16_t data_len, resp_len;
    uint32_t client_ip;
    uint16_t client_port;
    socklen_t addr_len; // UDP 需要这个长度参数
    int i;

    static struct sockaddr_in last_client_addr; // 记录上次通信的客户端
    static uint64_t last_recv_time = 0;         // 上次收到数据的时间（ms）
    uint64_t current_time;
    uint64_t keepalive_interval = 30000000000; // 30 秒主动探测间隔

    if (CSL_chipReadReg(CSL_CHIP_DNUM) != 0)
    {
        printf("Error: ProtocolServerTask should only run on Core 0\n");
        return;
    }

    printf("Protocol server (port %d) starting\n", PROTOCOL_SERVER_PORT);

    /* Allocate the file environment for this task */
    fdOpenSession(TaskSelf());

    /* 1. Create UDP socket */
    // 注意：这里使用 SOCK_DGRAM，去掉了 TCP 特有的 NC (No Copy) 标志
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
    {
        printf("Failed to create socket: %d\n", fdError());
        goto exit;
    }

    /* Set socket options (Optional for UDP, but ReuseAddr is useful) */
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 设置接收超时 3 秒，避免永久阻塞
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    int rcvbuf = 256 * 1024; // 256KB
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    /* 2. Bind socket */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PROTOCOL_SERVER_PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Failed to bind socket: %d\n", fdError());
        fdClose(sock);
        goto exit;
    }

    printf("Protocol server listening on port %d (UDP)\n", PROTOCOL_SERVER_PORT);

    /* UDP 是无连接的，不需要 listen 和 accept */
    while (1)
    {
        /* 3. Receive data from any client */
        addr_len = sizeof(client_addr);
        int recv_len = recvfrom(sock, (char *)recv_buf, sizeof(recv_buf), 0,
                                (struct sockaddr *)&client_addr, &addr_len);

        if (recv_len <= 0)
        {
            int err = fdError();

            printf("recvfrom returned %d, errno=%d (%s)\n", recv_len, err, strerror(err));

            // 打印 socket 状态（可选）
            int sock_err;
            socklen_t len = sizeof(sock_err);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &sock_err, &len);
            printf("socket SO_ERROR: %d\n", sock_err);

            // 根据错误码分别处理
            switch (err)
            {
            case EWOULDBLOCK:
            case ETIMEDOUT:
                // 超时，正常
                break;
            case ENOBUFS:
                // printf("!!! No buffer space available - socket may be dead !!!\n");
                break;
            case EMSGSIZE:
                // printf("!!! Message too large for receive buffer !!!\n");
                break;
            case EBADF:
                // printf("!!! Socket file descriptor invalid !!!\n");
                break;
            default:
                // printf("!!! Unexpected error !!!\n");
                break;
            }

            if (err == EWOULDBLOCK || err == ETIMEDOUT)
            {
                // 超时：检查是否需要发送保活包
                current_time = get_cycles();
                if (last_client_addr.sin_addr.s_addr != 0 &&
                    (current_time - last_recv_time) > keepalive_interval)
                {
                    // 向最后一个客户端发送一个空 UDP 包（或心跳请求）
                    uint8_t dummy_packet[1] = {0};
                    sendto(sock, dummy_packet, 0, 0,
                           (struct sockaddr *)&last_client_addr, sizeof(last_client_addr));
                    last_recv_time = current_time; // 重置计时器，避免频繁发送
                    printf("Send keepalive to client\n");
                }
                Task_sleep(10); // 避免空循环占用 CPU
                continue;
            }
            printf("UDP recvfrom error: %d\n", err);
            Task_sleep(10);
            continue;
        }

        /* Directly extract and print Client IP and Port */
        client_ip = ntohl(client_addr.sin_addr.s_addr);
        client_port = ntohs(client_addr.sin_port); // 获取客户端端口
        printf("Packet from: %d.%d.%d.%d:%d, Length: %d\n",
               (client_ip >> 24) & 0xFF, (client_ip >> 16) & 0xFF,
               (client_ip >> 8) & 0xFF, client_ip & 0xFF,
               client_port, recv_len);

        last_client_addr = client_addr;
        last_recv_time = get_cycles(); // 需要实现毫秒级时间函数

        /* Parse protocol frame */
        ret = protocol_parse_frame(recv_buf, recv_len, &cmd_code, &cmd_type, data_buf, &data_len);
        printf("Received command: 0x%02X, type: 0x%02X, data length: %d\n", cmd_code, cmd_type, data_len);

        if (ret != 0)
        {
            printf("Protocol parse error: %d\n", ret);
            /* Send error response back to the sender */
            uint16_t err_resp = STATUS_CRC_ERR; // Example error response
            resp_len = protocol_build_frame(send_buf, cmd_code, (uint8_t *)&err_resp, sizeof(err_resp), INFO_TYPE_RESP, 0, 0, 0x00);
        }
        else
        {
            printf("Processing command: 0x%02X, type: 0x%02X, data length: %d\n", cmd_code, cmd_type, data_len);
            uint16_t resp_data_len = 0;
            if (cmd_type != INFO_HEARTBEAT)
            {
                // 处理命令
                uint16_t status = protocol_process_command(cmd_code, data_buf, data_len,
                                                           resp_data, &resp_data_len);

                // 构建正常响应
                resp_len = protocol_build_frame(send_buf, cmd_code, resp_data, resp_data_len, INFO_TYPE_RESP, 0, 0, 0x00);
            }
            else
            {
                // 心跳包特殊处理：直接回复一个简单的 ACK
                uint8_t ack_resp[2] = {HEARTBEAT_STATUS_OK, 0}; // Example ACK response
                resp_len = protocol_build_frame(send_buf, cmd_code, ack_resp, 2, INFO_HEARTBEAT, 0, 0, 0x00);
            }
        }

        /* 4. Send response back to the client */
        // 必须使用 sendto，并指定 client_addr
        if (sendto(sock, send_buf, resp_len, 0, (struct sockaddr *)&client_addr, addr_len) != resp_len)
        {
            printf("Failed to send response\n");
        }
        else
        {
            printf("Response sent to client\n");
        }
        if (cmd_code == CMD_RESULT_READ)
        {
            Task_sleep(100);    
            uint32_t total_frames = 0;
            #ifdef RESTORE_RAM
            shared_inv(&process_frame_counter, sizeof(process_frame_counter));
            total_frames = process_frame_counter;
            #endif
            #ifdef RESTORE_NAND
            if (load_frame_counter(&total_frames) != 0)
            {
                printf("Failed to load frame counter\n");
                // 发送错误响应
                return;
            }
            #endif
            uint32_t frames_per_packet = RESULT_NUM_ONCE_READ;
            uint32_t bytes_per_read = frames_per_packet * sizeof(SharedResults_t);
            uint32_t total_packets = (total_frames + frames_per_packet - 1) / frames_per_packet;
            printf("Total frames to send: %u, Frames per packet: %u, Total packets: %u\n", total_frames, frames_per_packet, total_packets);
            for (i = 0; i < total_packets; i++)
            {
                uint32_t start_frame = i * frames_per_packet;
                uint32_t frames_this = (start_frame + frames_per_packet > total_frames) ? (total_frames - start_frame) : frames_per_packet;
                uint32_t bytes_this = frames_this * sizeof(SharedResults_t);
                memcpy(data_buf, &i, sizeof(uint32_t)); // 将包序号写入数据缓冲区
                // nandflash_read(NAND_RESULT_START_ADDR + i * bytes_per_read, bytes_per_read, data_buf + sizeof(uint32_t)); // 从 NAND 读取结果数据
                // 使用新函数按逻辑帧读取
                #ifdef RESTORE_NAND
                if (nand_read_result_frames(start_frame, frames_this, data_buf + sizeof(uint32_t)) != 0)
                {
                    printf("Failed to read result frames at packet %u\n", i);
                    break;
                }
                #endif

                #ifdef RESTORE_RAM
                memcpy(data_buf + sizeof(uint32_t), (SharedResults_t *)resultPtr + start_frame, bytes_this); // 从 RAM 读取结果数据
                #endif

                resp_len = protocol_build_frame(send_buf, cmd_code, data_buf, bytes_per_read + sizeof(uint32_t), INFO_TYPE_DATA, 0, 0, 0x00);
                if (sendto(sock, send_buf, resp_len, 0, (struct sockaddr *)&client_addr, addr_len) != resp_len)
                {
                    printf("Failed to send result data\n");
                }
                else
                {
                    printf("Result data sent to client, seqNum: %d, data length: %d\n", i, bytes_this + sizeof(uint32_t));
                }
                Task_sleep(1);
            }
            shared_inv(resultPtr, sizeof(resultPtr)); // 使缓存无效，确保数据一致
            if(resultPtr != NULL)
            {
                free((void *)resultPtr);
                resultPtr = NULL;
            }
            shared_wb(resultPtr, sizeof(resultPtr)); // 写回缓存，确保数据一致
        }
        Task_sleep(1);
    }

exit:
    fdCloseSession(TaskSelf());
    Task_exit();
}

/* NetworkOpen */
/* This function is called after the configuration has booted */
static void NetworkOpen()
{
    return;
}

/* NetworkClose */
/*
 * This function is called when the network is shutting down,
 * or when it no longer has any IP addresses assigned to it.
 */
static void NetworkClose()
{
    return;
}

/* NetworkIPAddr */
/*
 * This function is called whenever an IP address binding is
 * added or removed from the system.
 */
static volatile int g_servers_started = 0;
static void NetworkIPAddr(IPN IPAddr, uint IfIdx, uint fAdd)
{
    static uint fAddGroups = 0;
    IPN IPTmp;

    if (fAdd)
    {
        printf("Network Added: ");
    }
    else
    {
        printf("Network Removed: ");
    }

    /* Print a message */
    IPTmp = ntohl(IPAddr);
    printf("If-%d:%d.%d.%d.%d\n", IfIdx,
           (UINT8)(IPTmp >> 24) & 0xFF, (UINT8)(IPTmp >> 16) & 0xFF,
           (UINT8)(IPTmp >> 8) & 0xFF, (UINT8)IPTmp & 0xFF);

    /* This is a good time to join any multicast group we require */
    if (fAdd && !fAddGroups)
    {
        fAddGroups = 1;
    }

    // Added: start protocol server task (start once when network is added)
    if (fAdd && !g_servers_started)
    {
        g_servers_started = 1;

        Task_Params taskParams;

        // Protocol Server
        Task_Params_init(&taskParams);
        taskParams.stackSize = 0x20000; // 16KB
        taskParams.priority = 8;        // 7    // 5;
        Task_create(ProtocolServerTask, &taskParams, NULL);
        printf("Started Protocol servers\n");

        // Data Server
        /*Task_Params_init(&taskParams);
        taskParams.stackSize = 0x8000;
        taskParams.priority = 8;//5;
        Task_create(DataServerTask, &taskParams, NULL);

        printf("Started Data servers\n");*/

        /*// Start protocol server (port 2001)
        (void)TaskCreate(ProtocolServerTask, "ProtocolServer",
                         OS_TASKPRINORM, 0x2000, 0, 0, 0);

        // Start data server (port 2002)
        (void)TaskCreate(DataServerTask, "DataServer",
                         OS_TASKPRINORM, 0x2000, 0, 0, 0);
        printf("Started both Protocol and Data servers\n");
        */
    }

    return;
}

/* DHCP_reset() */
/*
 * Code to reset DHCP client by removing it from the active config,
 * and then reinstalling it.
 *
 * Called with:
 * IfIdx    set to the interface (1-n) that is using DHCP.
 * fOwnTask set when called on a new task thread (via TaskCreate()).
 */
void DHCP_reset(uint IfIdx, uint fOwnTask)
{
    CI_SERVICE_DHCPC dhcpc;
    HANDLE h;
    int rc, tmp;
    uint idx;

    // If we were called from a newly created task thread, allow
    // the entity that created us to complete
    if (fOwnTask)
        TaskSleep(500);

    // Find DHCP on the supplied interface
    for (idx = 1;; idx++)
    {
        // Find a DHCP entry
        rc = CfgGetEntry(0, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT,
                         idx, &h);
        if (rc != 1)
            goto RESET_EXIT;

        // Get DHCP entry data
        tmp = sizeof(dhcpc);
        rc = CfgEntryGetData(h, &tmp, (UINT8 *)&dhcpc);

        // If not the right entry, continue
        if ((rc <= 0) || dhcpc.cisargs.IfIdx != IfIdx)
        {
            CfgEntryDeRef(h);
            h = 0;
            continue;
        }

        // Remove the current DHCP service
        CfgRemoveEntry(0, h);

        // Specify DHCP Service on specified IF
        bzero(&dhcpc, sizeof(dhcpc));
        dhcpc.cisargs.Mode = CIS_FLG_IFIDXVALID;
        dhcpc.cisargs.IfIdx = IfIdx;
        dhcpc.cisargs.pCbSrv = &ServiceReport;
        CfgAddEntry(0, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0,
                    sizeof(dhcpc), (UINT8 *)&dhcpc, 0);
        break;
    }

RESET_EXIT:
    // If we are a function, return, otherwise, call TaskExit()
    if (fOwnTask)
        TaskExit();
}

void ServiceReport(uint Item, uint Status, uint Report, HANDLE h)
{
    printf("Service Status: %-9s: %-9s: %-9s: %03d\n",
           TaskName[Item - 1], StatusStr[Status],
           ReportStr[Report / 256], Report & 0xFF);

    if (Item == CFGITEM_SERVICE_DHCPCLIENT &&
        Status == CIS_SRV_STATUS_ENABLED &&
        (Report == (NETTOOLS_STAT_RUNNING | DHCPCODE_IPADD) ||
         Report == (NETTOOLS_STAT_RUNNING | DHCPCODE_IPRENEW)))
    {

        IPN IPTmp;

        // Manually add the DNS server when specified
        IPTmp = inet_addr(DNSServer);
        if (IPTmp)
            CfgAddEntry(0, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                        0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0);
    }

    // Reset DHCP client service on failure
    if (Item == CFGITEM_SERVICE_DHCPCLIENT &&
        (Report & ~0xFF) == NETTOOLS_STAT_FAULT)
    {
        CI_SERVICE_DHCPC dhcpc;
        int tmp;

        // Get DHCP entry data (for index to pass to DHCP_reset).
        tmp = sizeof(dhcpc);
        CfgEntryGetData(h, &tmp, (UINT8 *)&dhcpc);

        TaskCreate(DHCP_reset, "DHCPreset", OS_TASKPRINORM, 0x1000,
                   dhcpc.cisargs.IfIdx, 1, 0);
    }
}

/**
 * @brief BIOS task function
 *
 * @details get IP address and open http server
 *
 * @param arg0 the first arg
 *
 * @param arg1 the second arg
 *
 * @return NULL
 */
void netTask(UArg arg0, UArg arg1)
{
    int rc;
    HANDLE hCfg;

    printf("Core %d: netTask pri=%d\n", CSL_chipReadReg(CSL_CHIP_DNUM), Task_getPri(Task_self()));

    Error_Block eb;
    /*
     * Set L2 cache size as 64KB
     * equivalent to the l2Mode configuration in the Platform.xdc file
     */
    CACHE_setL2Size(CACHE_64KCACHE);

    /* Start TCSL so its free running */
    CSL_chipWriteTSCL(0);

    /* configure network phy chip */
    phy_config(0x0);
    // phy_config(0x1);

    /* Unlock the chip configuration registers */
    CSL_BootCfgUnlockKicker();

#ifdef C66_PLATFORMS
    /* initialize the sgmii network subsystem */
    init_sgmii(0);
    // init_sgmii(1);


    //add 20260411
    // 复位 QMSS 全局（具体寄存器参考芯片手册）
    CSL_BootCfgUnlockKicker();
    // 设置 QMSS 软件复位
    //CSL_QmssEnableReset();   // 需要自行实现或调用 PDK 接口
    //CSL_QmssDisableReset();
    QMSS_reset();
    // 复位 CPPI 和 PA 同理
    CSL_CppiReset();
    CSL_PaReset();
    CSL_BootCfgLockKicker();
    //add finish


    if (queue_manager_init() != 0)
        return;
#endif

    /* Lock the chip configuration registers */
    CSL_BootCfgLockKicker();

    /* Variable initialization */
    Error_init(&eb);

    /*
     * THIS MUST BE THE ABSOLUTE FIRST THING DONE IN AN APPLICATION before
     *  using the stack!!
     */
    rc = NC_SystemOpen(NC_PRIORITY_LOW, NC_OPMODE_INTERRUPT);
    if (rc)
    {
        printf("NC_SystemOpen Failed (%d)\n", rc);
        goto task_exit;
    }

    /* Create and build the system configuration from scratch. */
    /* Create a new configuration */
    hCfg = CfgNew();
    if (!hCfg)
    {
        printf("Unable to create configuration\n");
        goto task_exit;
    }

    /* We better validate the length of the supplied names */
    if (strlen(DomainName) >= CFG_DOMAIN_MAX ||
        strlen(HostName) >= CFG_HOSTNAME_MAX)
    {
        printf("Names too long\n");
        goto task_exit;
    }

    /* Add our global hostname to hCfg (to be claimed in all connected domains) */
    CfgAddEntry(hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_HOSTNAME, 0,
                strlen(HostName), (UINT8 *)HostName, 0);

    /* if LocalIPAddr set to 0.0.0.0, use dhcp to get ip address */
    if (inet_addr(LocalIPAddr))
    {
        CI_IPNET NA;
        CI_ROUTE RT;
        IPN IPTmp;

        /* Setup manual IP address */
        bzero(&NA, sizeof(NA));
        NA.IPAddr = inet_addr(LocalIPAddr);
        NA.IPMask = inet_addr(LocalIPMask);
        strcpy(NA.Domain, DomainName);
        NA.NetType = 0;

        /* Add the address to interface 2 */
        CfgAddEntry(hCfg, CFGTAG_IPNET, 1, 0,
                    sizeof(CI_IPNET), (UINT8 *)&NA, 0);

        /*
         * Add the default gateway. Since it is the default, the
         * destination address and mask are both zero (we go ahead
         * and show the assignment for clarity).
         */
        bzero(&RT, sizeof(RT));
        RT.IPDestAddr = 0;
        RT.IPDestMask = 0;
        RT.IPGateAddr = inet_addr(GatewayIP);

        /* Add the route */
        CfgAddEntry(hCfg, CFGTAG_ROUTE, 0, 0,
                    sizeof(CI_ROUTE), (UINT8 *)&RT, 0);

        /* Manually add the DNS server when specified */
        IPTmp = inet_addr(DNSServer);
        if (IPTmp)
            CfgAddEntry(hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                        0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0);
    }
    else
    {
        /*
        CI_SERVICE_DHCPC dhcpc;

        printf("Configuring DHCP client\n");

        // Specify DHCP Service on IF-2
        bzero(&dhcpc, sizeof(dhcpc));
        dhcpc.cisargs.Mode = CIS_FLG_IFIDXVALID;
        dhcpc.cisargs.IfIdx = 1;
        dhcpc.cisargs.pCbSrv = &ServiceReport;
        dhcpc.param.pOptions = DHCP_OPTIONS;
        dhcpc.param.len = 2;

        CfgAddEntry(hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0,
                     sizeof(dhcpc), (UINT8 *)&dhcpc, 0);*/
        printf("please first set ipAddress.\n");
    }

    /* Specify TELNET service for our Console example */
    /*bzero(&telnet, sizeof(telnet));
    telnet.cisargs.IPAddr = INADDR_ANY;
    telnet.cisargs.pCbSrv = &ServiceReport;
    telnet.param.MaxCon = 2;
    telnet.param.Callback = &ConsoleOpen;
    CfgAddEntry(hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_TELNET, 0,
                 sizeof(telnet), (UINT8 *)&telnet, 0);*/

    /*
     * This code sets up the TCP and UDP buffer sizes
     * (Note 8192 is actually the default. This code is here to
     * illustrate how the buffer and limit sizes are configured.)
     */

    /* TCP Transmit buffer size */
    rc = 64000;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPTXBUF,
                CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    /* TCP Receive buffer size (copy mode) */
    rc = 64000;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXBUF,
                CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    /* TCP Receive limit (non-copy mode) */
    rc = 64000;
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXLIMIT,
                CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0);

    // 延长路由缓存时间 (单位: 秒)
    uint32_t keepalive_timeout = 1800; // 30分钟
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_RTKEEPALIVETIME, CFG_ADDMODE_UNIQUE,
                sizeof(uint32_t), (UINT8 *)&keepalive_timeout, 0);

    uint32_t arp_inactivity = 1200; // 20分钟
    CfgAddEntry(hCfg, CFGTAG_IP, CFGITEM_IP_RTARPINACTIVITY, CFG_ADDMODE_UNIQUE,
                sizeof(uint32_t), (UINT8 *)&arp_inactivity, 0);
    /*
     * Boot the system using this configuration
     *
     * We keep booting until the function returns 0. This allows
     * us to have a "reboot" command.
     */
    do
    {
        rc = NC_NetStart(hCfg, NetworkOpen, NetworkClose, NetworkIPAddr);
    } while (rc > 0);

    /* Delete Configuration*/
    CfgFree(hCfg);

    /* Close the OS */
task_exit:
    NC_SystemClose();
    Task_exit();
    return;
}

uint16_t phy_reg_read(uint8_t phy_addr, uint16_t reg_addr)
{
    hMdioRegs->USER_GROUP[0].USER_ACCESS_REG = (CSL_FMK(MDIO_USER_ACCESS_REG_PHYADR, phy_addr) |
                                                CSL_FMK(MDIO_USER_ACCESS_REG_REGADR, reg_addr) |
                                                CSL_FMK(MDIO_USER_ACCESS_REG_WRITE, 0) |
                                                CSL_FMK(MDIO_USER_ACCESS_REG_GO, 1));

    cpu_delaycycles(50000000);

    return CSL_FEXT(hMdioRegs->USER_GROUP[0].USER_ACCESS_REG, MDIO_USER_ACCESS_REG_DATA);
}

void phy_reg_write(uint8_t phy_addr, uint16_t reg_addr, uint16_t data)
{
    hMdioRegs->USER_GROUP[0].USER_ACCESS_REG = (CSL_FMK(MDIO_USER_ACCESS_REG_DATA, data) |
                                                CSL_FMK(MDIO_USER_ACCESS_REG_PHYADR, phy_addr) |
                                                CSL_FMK(MDIO_USER_ACCESS_REG_REGADR, reg_addr) |
                                                CSL_FMK(MDIO_USER_ACCESS_REG_WRITE, 1) |
                                                CSL_FMK(MDIO_USER_ACCESS_REG_GO, 1));
}

void yt8521sh_phy_config(uint8_t phy_addr)
{
    /* Config LED2 to ON when phy link up */
    phy_reg_write(phy_addr, YT8521SH_EXT_REG_ADDR_OFFSET_REG, YT8521SH_EXT_LED2_CFG);
    cpu_delaycycles(50000000);
    phy_reg_write(phy_addr, YT8521SH_EXT_REG_DATA_REG, 0x0070);
    cpu_delaycycles(50000000);

    /* Config LED1 to BLINK when phy link up and tx rx active */
    phy_reg_write(phy_addr, YT8521SH_EXT_REG_ADDR_OFFSET_REG, YT8521SH_EXT_LED1_CFG);
    cpu_delaycycles(50000000);
    phy_reg_write(phy_addr, YT8521SH_EXT_REG_DATA_REG, 0x0670);
    cpu_delaycycles(50000000);

    // 禁用 EEE（如果需要）
    phy_reg_write(phy_addr, 0x1E, 0x0A);   // 扩展寄存器页选择
    phy_reg_write(phy_addr, 0x1F, 0x0000); // 根据芯片手册设置禁用 EEE

    return;
}

void phy_config(uint8_t phy_addr)
{
    uint16_t phy_identity = 0;
    uint16_t manufacturer_id = 0;

    /* enable the MDIO state machine */
    CSL_MDIO_enableStateMachine();

    phy_identity = phy_reg_read(phy_addr, PHY_IDENTIFIER_1_REG);
    manufacturer_id = phy_reg_read(phy_addr, PHY_IDENTIFIER_2_REG);

    /*
     * Select different initialization APIs according to phy id
     * At present, only the YT8521SH phy chip is initialized
     * and the rest of the phy chips are not initialized
     */
    if (phy_identity == PHY_MOTORCOMM_IDENTIFIER &&
        manufacturer_id == PHY_MOTORCOMM_MANUFACTURER)
    {
        yt8521sh_phy_config(phy_addr);
    }
}
