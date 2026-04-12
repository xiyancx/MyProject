#ifndef NAND_OP_H
#define NAND_OP_H

#include <stdint.h>

int nand_intialize(void);
int erase_result_block(void);
int nandflash_write(uint32_t addr, uint32_t size, uint8_t *write_buffer);
int nandflash_read(uint32_t addr, uint32_t size, uint8_t *read_buffer);
int nand_read_result_frames(uint32_t start_frame, uint32_t num_frames, uint8_t *buffer);
int save_frame_counter(uint32_t counter);
int load_frame_counter(uint32_t *counter);
int save_timestamp(uint64_t timestamp);
int load_timestamp(uint64_t *timestamp);

#endif
