/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file tcpServer.h
 *
 * @brief Header file for TCP Server
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2020-08-09
 *
 **/

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

/* C standard Header files */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* NDK include */
#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/_stack.h>
#include <ti/ndk/inc/tools/console.h>
#include <ti/ndk/inc/tools/servers.h>


#ifdef C66_PLATFORMS
#include <ti/csl/csl_cpsgmii.h>
#include <ti/csl/csl_cpsgmiiAux.h>
#endif
#include "system/resource_mgr.h"

// Common phy register
#define PHY_IDENTIFIER_1_REG                    2
#define PHY_IDENTIFIER_2_REG                    3

// Phy identification
#define PHY_MOTORCOMM_IDENTIFIER                0x0
#define PHY_MOTORCOMM_MANUFACTURER              0x011a

// YT8521SH phy register
#define YT8521SH_EXT_REG_ADDR_OFFSET_REG        0x1E
#define YT8521SH_EXT_REG_DATA_REG               0x1F
#define YT8521SH_EXT_LED2_CFG                   0xA00E
#define YT8521SH_EXT_LED1_CFG                   0xA00D

/* TCP & UDP socket port id */
#define TCP_SERVER_PORT     1001

/* TCP & UDP single transmit size, the unit is Byte */
#define TCP_SERVER_BUFSIZE  (1024 * 1024)


/* Data Server definitions */
#define FRAME_COUNTER_SIZE      8
#define DATA_SERVER_PORT        2002
#define DATA_BUFFER_SIZE        16384  // 16KB = 8ch × 2048Bytes
#define DATA_HEADER_SIZE        2      // 0x55 0xAB
#define DATA_CRC_SIZE          2
#define FRAME_COUNTER_SIZE     8
#define DATA_TOTAL_SIZE        (DATA_HEADER_SIZE+ FRAME_COUNTER_SIZE + DATA_BUFFER_SIZE + DATA_CRC_SIZE)

// data control command definitions
#define CMD_START_SEND         0x00
#define CMD_STOP_SEND          0x01

// data control command structure

extern char *LocalIPAddr;
extern char *LocalIPMask;
extern char *GatewayIP;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef C66_PLATFORMS
void init_sgmii(uint32_t macPortNum);
#endif

// data server task function
//int DataServerTask_00(void);
void ProtocolServerTask(void);
void DataServerTask(void);
void ServiceReport(uint Item, uint Status, uint Report, HANDLE h);
void netTask(UArg arg0, UArg arg1);
uint16_t phy_reg_read(uint8_t phy_addr, uint16_t reg_addr);
void phy_reg_write(uint8_t phy_addr, uint16_t reg_addr, uint16_t data);
void yt8521sh_phy_config(uint8_t phy_addr);
void phy_config(uint8_t phy_addr);

#ifdef __cplusplus
}
#endif

#endif // TCP_SERVER_H
