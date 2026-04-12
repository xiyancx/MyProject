/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file c66x_nor.h
 *
 * @brief This file is the header file for NOR module.
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2020-08-09
 *
 **/

#ifndef C66X_NOR_H
#define C66X_NOR_H

typedef struct {
    uint32_t manufacturer_id; /* manufacturer id */
    uint32_t memory_type;   /* memory type */
    uint32_t memory_capacity; /* memory capacity */
    uint32_t page_size; /* number of bytes in a data page */
    uint32_t sector_count; /* total number of sectors */
    uint32_t sector_size; /* number of bytes in a data sector */
    uint32_t max_flash_size; /* total number of bytes in device */
} nor_device_info;

int32_t c66x_nor_read(nor_device_info *info, uint32_t addr, uint32_t len, uint8_t *buf);
int32_t c66x_nor_write(nor_device_info *info, uint32_t addr, uint32_t len, uint8_t *buf);
int32_t c66x_nor_erase_sector(nor_device_info *info, uint32_t sector_number);
int32_t c66x_nor_erase_bulk(void);
int32_t c66x_nor_get_info(nor_device_info *info);

#endif
