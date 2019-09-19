#include "ext_flash.h"

#define MAX_SPI_BUFFER_SIZE 2100

extern const flash_list_t flash_list[] = {
    {"winbond w25n01gv", {0xEF, 0xAA, 0x21}}};

/* FIXME Find a better way to handle this without global buf */
uint8_t spi_xfer_buf[MAX_SPI_BUFFER_SIZE];
uint8_t spi_rcv_buf[MAX_SPI_BUFFER_SIZE];

int8_t flash_spi_transfer(flash_device_t* flash_dev,
                          spi_transfer_t* spi_xfer_data)
{
    uint16_t len = 0;
    int32_t status = FLASH_SUCCESS;

    memset(spi_xfer_buf, 0, MAX_SPI_BUFFER_SIZE);
    memset(spi_rcv_buf, 0, MAX_SPI_BUFFER_SIZE);

    spi_xfer_buf[0] = spi_xfer_data->opcode;
    len++;

    
    if (spi_xfer_data->dummy_cyles_pos == 0)
    {
        len += spi_xfer_data->dummy_cycles;
    }

    if (spi_xfer_data->addr_len != 0)
    {
        memcpy(spi_xfer_buf + len, spi_xfer_data->addr, spi_xfer_data->addr_len);
        len += spi_xfer_data->addr_len;
    }

    if (spi_xfer_data->dummy_cyles_pos == 1)
    {
        len += spi_xfer_data->dummy_cycles;
    }

    if (spi_xfer_data->tx_len != 0)
    {
        memcpy(spi_xfer_buf + len, spi_xfer_data->tx_buf, spi_xfer_data->tx_len);
        len += spi_xfer_data->tx_len;
    }

    NRF_LOG_DEBUG("tx_len: %d, rx_len: %d", len, len + spi_xfer_data->rx_len);
    NRF_LOG_HEXDUMP_DEBUG(spi_xfer_buf, len);

    /* Transfer SPI data and receive */
    status = flash_dev->spi_xfer(spi_xfer_buf, len, spi_rcv_buf,
                                 (len + spi_xfer_data->rx_len));
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "Flash: %s, %d: SPI Xfer failed!", __func__, __LINE__);
        return status;
    }

    while (spi_xfer_done == false);

    spi_xfer_done = false;

    if (spi_xfer_data->rx_len != 0)
    {
        NRF_LOG_DEBUG("rx_l: %d", spi_xfer_data->rx_len);
        memcpy(spi_xfer_data->rx_buf, spi_rcv_buf + len, spi_xfer_data->rx_len);
        NRF_LOG_HEXDUMP_DEBUG(spi_rcv_buf, len + spi_xfer_data->rx_len);
    }

    return FLASH_SUCCESS;
}
