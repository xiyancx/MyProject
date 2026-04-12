/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file c66x_nand.h
 *
 * @brief This file is the header file for NAND module.
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2020-08-09
 *
 **/

#ifndef C66X_NAND_H
#define C66X_NAND_H

typedef struct {
    uint32_t column_addr;
    uint32_t page_addr;
    uint32_t block_addr;
} nand_addr;

typedef struct {
    uint32_t device_id;     /* device id */
    uint32_t manufacturer_id;   /* manufacturer id */
    uint32_t block_count;   /* number of blocks per device */
    uint32_t page_size;     /* number of bytes per device */
    uint32_t page_count;    /* number of pages per block */
} nand_device_info;

int32_t nand_read_page(nand_addr address, uint8_t* puchBuffer);
int32_t nand_write_page(nand_addr address, uint8_t* puchBuffer);
uint32_t nand_get_info(nand_device_info *nand_info);
int32_t nand_erase_block(uint32_t uiBlockNumber);
int8_t nand_init();

#endif
