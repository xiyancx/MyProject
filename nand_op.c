#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "nand_op.h"
#include "system/platform.h"
#include "driver/c66x_nand.h"
#include "shared_mem.h"

nand_device_info nand_info;

// 涓存椂椤电紦鍐插尯锛屽ぇ灏忚嚦灏戠瓑浜�nand_info.page_size锛堥�甯镐负 2048 鎴�4096锛�
// 澹版槑涓洪潤鎬侊紝閬垮厤澶氫换鍔″苟鍙戝啿绐侊紙濡傞渶骞跺彂璇峰姞閿侊級
static uint8_t s_page_buffer[4096];

int nand_intialize(void)
{
    int8_t ret;
    ret = nand_init();
    if (ret != 0)
    {
        return -1;
    }

    ret = nand_get_info(&nand_info);
    if (ret != 0)
    {
        return -1;
    }

    // 纭繚涓存椂缂撳啿鍖鸿冻澶熷ぇ
    if (nand_info.page_size > sizeof(s_page_buffer))
    {
        printf("Page size %d exceeds buffer size %d\n", nand_info.page_size, (int)sizeof(s_page_buffer));
        return -1;
    }
    return 0;
}

int erase_result_block(void)
{
    uint32_t block;
    uint32_t start_block = NAND_RESULT_START_BLOCK;
    uint32_t end_block = start_block + (NAND_RESULT_SIZE / NAND_BLOCK_SIZE);
    int ret = 0;  // 假设成功

    for (block = start_block; block < end_block; block++)
    {
        if (nand_erase_block(block) != 0)
        {
            printf("Warning: erase block %d failed (may be bad block)\n", block);
            ret = -1;   // 标记失败，但继续尝试擦除其他块（以便发现所有坏块）
        }
    }
    return ret;
}

// 淇濆瓨甯ц鏁板埌涓撶敤鍧楋紙姣忔鍐欏墠鎿﹂櫎鏁翠釜鍧楋級
int save_frame_counter(uint32_t counter)
{
    nand_addr addr;
    addr.block_addr = NAND_COUNTER_BLOCK;
    addr.page_addr = 0;   // 浣跨敤鍧楀唴绗�椤�
    addr.column_addr = 0; // 浠庨〉璧峰浣嶇疆鍐欏叆

    // 1. 鎿﹂櫎鏁翠釜鍧�
    if (nand_erase_block(addr.block_addr) != 0)
    {
        printf("Error: erase block %d failed\n", addr.block_addr);
        return -1;
    }

    // 2. 鍐欏叆鏁版嵁锛堟暣椤靛啓鍏ワ級
    if (nand_write_page(addr, (uint8_t *)&counter) != 0)
    {
        printf("Error: write page failed at block %d page %d\n", addr.block_addr, addr.page_addr);
        return -1;
    }

    return 0;
}

// 浠庝笓鐢ㄥ潡璇诲彇甯ц鏁�
// 假设已有 static uint8_t s_page_buffer[4096];
int load_frame_counter(uint32_t *counter) {
    nand_addr addr;
    addr.block_addr = NAND_COUNTER_BLOCK;
    addr.page_addr = 0;
    addr.column_addr = 0;

    // 使用页缓冲区读取整页
    if (nand_read_page(addr, s_page_buffer) != 0) {
        printf("Error: read page failed at block %d page %d\n", addr.block_addr, addr.page_addr);
        return -1;
    }
    // 从缓冲区前4字节提取计数器值
    *counter = *(uint32_t*)s_page_buffer;
    return 0;
}

// 淇濆瓨鏃ユ湡鏃堕棿鎴筹紙绫讳技瀹炵幇锛�
int save_timestamp(uint64_t timestamp)
{
    nand_addr addr;
    addr.block_addr = NAND_DATE_BLOCK;
    addr.page_addr = 0;
    addr.column_addr = 0;

    if (nand_erase_block(addr.block_addr) != 0)
        return -1;
    if (nand_write_page(addr, (uint8_t *)&timestamp) != 0)
        return -1;
    return 0;
}

int load_timestamp(uint64_t *timestamp)
{
    nand_addr addr;
    addr.block_addr = NAND_DATE_BLOCK;
    addr.page_addr = 0;
    addr.column_addr = 0;
    // 使用页缓冲区读取整页
    if (nand_read_page(addr, s_page_buffer) != 0) {
        printf("Error: read page failed at block %d page %d\n", addr.block_addr, addr.page_addr);
        return -1;
    }
    // 从缓冲区前8字节提取时间值
    *timestamp = *(uint64_t*)s_page_buffer;
    return 0;
}

/**
 * @brief 浠�NAND 璇诲彇浠绘剰浣嶇疆銆佷换鎰忛暱搴︾殑鏁版嵁
 * @param addr        璧峰鐗╃悊鍦板潃锛堝瓧鑺傚亸绉伙級
 * @param size        瑕佽鍙栫殑瀛楄妭鏁�
 * @param read_buffer 杈撳嚭缂撳啿鍖猴紙蹇呴』鑳藉绾�size 瀛楄妭锛�
 * @return 0 鎴愬姛锛�1 澶辫触
 */
int nandflash_read(uint32_t addr, uint32_t size, uint8_t *read_buffer)
{
    if (read_buffer == NULL || size == 0)
    {
        return -1;
    }

    uint32_t page_size = nand_info.page_size;
    uint32_t pages_per_block = nand_info.page_count;
    uint32_t block_size = pages_per_block * page_size;

    // 璁＄畻璧峰鍧椼�椤靛唴鍋忕Щ
    uint32_t start_block = addr / block_size;
    uint32_t offset_in_block = addr % block_size;
    uint32_t start_page = offset_in_block / page_size;
    uint32_t start_col = offset_in_block % page_size;

    uint32_t rPos = 0;
    uint32_t block = start_block;
    uint32_t page = start_page;
    uint32_t col = start_col;

    while (rPos < size)
    {
        // 褰撳墠椤靛彲璇诲彇鐨勫瓧鑺傛暟锛堜粠 col 鍒伴〉灏撅級
        uint32_t bytes_this_page = page_size - col;
        if (bytes_this_page > size - rPos)
        {
            bytes_this_page = size - rPos;
        }

        // 鏋勯� NAND 鍦板潃缁撴瀯锛堝垪鍦板潃涓�0锛屽洜涓烘垜浠�鏄鍙栨暣椤碉級
        nand_addr address;
        address.column_addr = 0; // 搴曞眰鎬绘槸浠庨〉棣栬鍙�
        address.page_addr = page;
        address.block_addr = block;

        // 璇诲彇鏁撮〉鍒颁复鏃剁紦鍐插尯
        if (nand_read_page(address, s_page_buffer) != 0)
        {
            printf("nand_read_page failed at block %d page %d\n", block, page);
            return -1;
        }

        // 浠庝复鏃剁紦鍐插尯涓鍒舵墍闇�儴鍒嗭紙浠�col 寮�锛屽鍒�bytes_this_page 瀛楄妭锛�
        memcpy(read_buffer + rPos, s_page_buffer + col, bytes_this_page);

        rPos += bytes_this_page;
        col = 0; // 涓嬩竴椤典粠鍒�0 寮�
        page++;

        // 鑻ラ〉鍙疯秴鍑哄綋鍓嶅潡锛屽垯鍒囨崲鍒颁笅涓�潡
        if (page >= pages_per_block)
        {
            page = 0;
            block++;
        }
    }

    return 0;
}

/**
 * @brief 从结果区读取连续的有效数据（自动跳过每页末尾的填充字节）
 * @param start_frame  起始结果帧序号（0开始）
 * @param num_frames   要读取的帧数
 * @param buffer       输出缓冲区（至少 num_frames * sizeof(SharedResults_t) 字节）
 * @return 0成功，-1失败
 */
int nand_read_result_frames(uint32_t start_frame, uint32_t num_frames, uint8_t *buffer) {
    if (buffer == NULL || num_frames == 0) return -1;

    uint32_t bytes_per_result = sizeof(SharedResults_t);  // 24
    uint32_t valid_per_page = RESULTS_PER_PAGE * bytes_per_result;  // 2040
    uint32_t page_size = NAND_PAGE_SIZE;  // 2048
    uint32_t logical_offset = start_frame * bytes_per_result;
    uint32_t size = num_frames * bytes_per_result;
    uint32_t buf_pos = 0;
    uint8_t temp_page[NAND_PAGE_SIZE];   // 修正：页大小，而非 RESULTS_PER_PAGE

    while (size > 0) {
        uint32_t page_idx = logical_offset / valid_per_page;
        uint32_t page_offset = logical_offset % valid_per_page;
        uint32_t physical_addr = NAND_RESULT_START_ADDR + page_idx * page_size;

        if (nandflash_read(physical_addr, page_size, temp_page) != 0) {
            printf("Read failed at page %u\n", page_idx);
            return -1;
        }

        uint32_t bytes_in_page = valid_per_page - page_offset;
        uint32_t to_copy = (size < bytes_in_page) ? size : bytes_in_page;
        memcpy(buffer + buf_pos, temp_page + page_offset, to_copy);

        buf_pos += to_copy;
        logical_offset += to_copy;
        size -= to_copy;
    }
    return 0;
}

/**
 * @brief 鍚�NAND 鍐欏叆浠绘剰浣嶇疆銆佷换鎰忛暱搴︾殑鏁版嵁
 * @param addr         璧峰鐗╃悊鍦板潃锛堝瓧鑺傚亸绉伙級
 * @param size         瑕佸啓鍏ョ殑瀛楄妭鏁�
 * @param write_buffer 杈撳叆鏁版嵁缂撳啿鍖�
 * @return 0 鎴愬姛锛�1 澶辫触
 */
int nandflash_write(uint32_t addr, uint32_t size, uint8_t *write_buffer)
{
    if (write_buffer == NULL || size == 0)
    {
        return -1;
    }

    uint32_t page_size = nand_info.page_size;
    uint32_t pages_per_block = nand_info.page_count;
    uint32_t block_size = pages_per_block * page_size;

    // 璁＄畻璧峰鍧椼�椤靛唴鍋忕Щ
    uint32_t start_block = addr / block_size;
    uint32_t offset_in_block = addr % block_size;
    uint32_t start_page = offset_in_block / page_size;
    uint32_t start_col = offset_in_block % page_size;

    uint32_t wPos = 0;
    uint32_t block = start_block;
    uint32_t page = start_page;
    uint32_t col = start_col;

    while (wPos < size)
    {
        // 褰撳墠椤靛彲鍐欏叆鐨勫瓧鑺傛暟锛堜粠 col 鍒伴〉灏撅級
        uint32_t bytes_this_page = page_size - col;
        if (bytes_this_page > size - wPos)
        {
            bytes_this_page = size - wPos;
        }

        nand_addr address;
        address.column_addr = 0; // 搴曞眰鎬绘槸鎿嶄綔鏁撮〉
        address.page_addr = page;
        address.block_addr = block;

        // 濡傛灉涓嶆槸浠庨〉棣栧紑濮嬪啓锛屾垨鑰呬笉鏄啓婊℃暣椤碉紝鍒欓渶瑕佸厛璇诲彇鍘熼〉鍐呭
        if (col != 0 || bytes_this_page != page_size)
        {
            // 璇诲彇鍘熼〉鍐呭鍒颁复鏃剁紦鍐插尯
            if (nand_read_page(address, s_page_buffer) != 0)
            {
                printf("nand_read_page (for modify) failed at block %d page %d\n", block, page);
                return -1;
            }
            // 灏嗘柊鏁版嵁瑕嗙洊鍒颁复鏃剁紦鍐插尯鐨勭浉搴斾綅缃�
            memcpy(s_page_buffer + col, write_buffer + wPos, bytes_this_page);
            // 鍐欏洖鏁撮〉
            if (nand_write_page(address, s_page_buffer) != 0)
            {
                printf("nand_write_page (modified) failed at block %d page %d\n", block, page);
                return -1;
            }
        }
        else
        {
            // 鏁撮〉鍐欏叆锛岀洿鎺ヤ娇鐢ㄧ敤鎴锋暟鎹�
            if (nand_write_page(address, write_buffer + wPos) != 0)
            {
                printf("nand_write_page (full page) failed at block %d page %d\n", block, page);
                return -1;
            }
        }

        wPos += bytes_this_page;
        col = 0;
        page++;

        if (page >= pages_per_block)
        {
            page = 0;
            block++;
        }
    }

    return 0;
}
