/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file c66x_eeprom.h
 *
 * @brief This file is the header file for EEPROM.
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2020-08-09
 *
 **/

#ifndef C66X_EEPROM_H
#define C66X_EEPROM_H

/** EEPROM page writer buffer size */
#define EEPROM_PAGE_SIZE 256

int16_t c66x_eeprom_page_write(uint16_t byte_addr, uint8_t uchEepromI2cAddress, uint8_t *puiData, \
    uint32_t uiNumBytes, uint32_t uiEndBusState);
int16_t c66x_eeprom_sequential_read ( uint16_t byte_addr, uint32_t uiNumBytes, \
    uint8_t *puiData, uint8_t uchEepromI2cAddress);

#endif
