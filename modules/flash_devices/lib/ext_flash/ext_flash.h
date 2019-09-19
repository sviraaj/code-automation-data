#ifndef __EXT_FLASH_H__
#define __EXT_FLASH_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

//#define NRFX_LOG_MODULE EXT_FLASH
#include <nrfx_log.h>

#define MAX_FLASH_ID_SZ 3U

#define ERROR   0
#define INFO    1

#define LOG_FLASH(log_level, ...) \
do {                                     \
        if (log_level == ERROR)          \
        {                                \
            NRF_LOG_ERROR( __VA_ARGS__); \
        }                                \
        else if (log_level == INFO)      \
        {                                \
            NRF_LOG_INFO( __VA_ARGS__);  \
        }                                \
    } while (0);                         \

typedef struct spi_transfer
{
    uint8_t opcode;
    uint8_t dummy_cycles;
    uint8_t dummy_cyles_pos;
    uint8_t* addr;
    uint8_t addr_len;
    uint8_t* tx_buf;
    uint8_t* rx_buf;
    uint32_t tx_len;
    uint32_t rx_len;
} spi_transfer_t;

typedef struct flash_device_list
{
    const char* flash_dev_name;
    uint8_t flash_dev_id[MAX_FLASH_ID_SZ];
} flash_list_t;

enum flash_error_codes
{
    FLASH_SUCCESS = 0,
    FLASH_DETECT_FAIL = -1,
    FLASH_TRANSFER_ERROR = -2,
    FLASH_TIMEOUT = -3,
    FLASH_INVALID_PARAMS = -4,
    FLASH_MISC_FAILURE = -5,
};

typedef struct flash_device
{
    int32_t (*spi_xfer)(uint8_t* tx_buf, uint32_t tx_len, uint8_t* rx_buf,
                        uint32_t rx_len);
    void (*sleep)(uint32_t ms);
    uint32_t flash_size;
    uint16_t num_of_pages_per_block;
    uint16_t num_of_blocks;
    uint16_t page_size;
    uint16_t num_ecc_bytes_per_page;
} flash_device_t;

int8_t flash_spi_transfer(flash_device_t* flash_dev,
                          spi_transfer_t* spi_xfer_data);
int8_t flash_read(flash_device_t* flash_dev, uint32_t addr,
                  uint8_t* read_buf, uint32_t read_len);
int8_t flash_write(flash_device_t* flash_dev, uint32_t addr,
                   uint8_t* write_buf, uint32_t write_len);
int8_t flash_erase(flash_device_t* flash_dev, uint32_t addr,
                   uint32_t erase_len);

extern const flash_list_t flash_list[];

/**< Flag used to indicate that SPI instance completed the transfer. */
extern volatile bool spi_xfer_done;

#endif
