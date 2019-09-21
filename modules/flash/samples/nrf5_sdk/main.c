#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "w25n01gv_internal.h"

#define SPI_INSTANCE  0 /**< SPI instance index. */
#define VECTOR_LENGTH 2200

uint8_t recv_buf[VECTOR_LENGTH];  
uint8_t tx_buf[VECTOR_LENGTH];

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}

static ret_code_t nrf_drv_spi_transfer_own(uint8_t const * p_tx_buffer,
                                uint8_t         tx_buffer_length,
                                uint8_t       * p_rx_buffer,
                                uint8_t         rx_buffer_length)
{
  return nrf_drv_spi_transfer(&spi, p_tx_buffer, tx_buffer_length, p_rx_buffer, rx_buffer_length);
}

int main(void)
{
    int8_t status = FLASH_SUCCESS;
    nrfx_err_t ret_val;
    flash_device_t flash_dev;

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = 13;//SPI_SS_PIN
    spi_config.miso_pin = 21;//SPI_MISO_PIN
    spi_config.mosi_pin = 15;//SPI_MOSI_PIN
    spi_config.sck_pin  = 17;//SPI_SCK_PIN
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

    memset(recv_buf, 0, VECTOR_LENGTH);
    memset(tx_buf, 0, VECTOR_LENGTH);

    NRF_LOG_INFO("SPI flash sample started.");

    flash_dev.spi_xfer = nrf_drv_spi_transfer_own;
    flash_dev.sleep = nrf_delay_ms;
    flash_dev.flash_size = 128 * 1024 * 1024;
    uint16_t num_of_pages_per_block = 64;
    uint16_t num_of_blocks = 1024;
    uint16_t page_size = 2 * 1024;
    uint16_t num_ecc_bytes_per_page = 64;

    status = w25n01gv_init(&flash_dev);
    if (status != FLASH_SUCCESS)
    {
        NRF_LOG_INFO("winbond init fail");
    }
    
    NRF_LOG_INFO("winbond init done");

    for (int i = 0; i < 100; i++)
    {
        tx_buf[i] = i;
    }

    while (1)
    {

        memset(recv_buf, 0, 2200);

        NRF_LOG_INFO("winbond read start##############");
        status = w25n01gc_flash_read(&flash_dev, 10, recv_buf, 100);
        if (status != FLASH_SUCCESS)
        {
            NRF_LOG_INFO("winbond read fail");
        }
        NRF_LOG_INFO("winbond read end##############");


        NRF_LOG_INFO("winbond ERASE");

        status = w25n01gc_flash_erase(&flash_dev, 10, 10000);
        if (status != FLASH_SUCCESS)
        {
            NRF_LOG_INFO("winbond erase fail");
        }

        NRF_LOG_INFO("winbond read after erase##############");
        status = w25n01gc_flash_read(&flash_dev, 10, recv_buf, 100);
        if (status != FLASH_SUCCESS)
        {
            NRF_LOG_INFO("winbond read fail");
        }
        NRF_LOG_INFO("winbond read erase end##############");

        NRF_LOG_INFO("winbond WRITE");

        status = w25n01gc_flash_write(&flash_dev, 10, tx_buf, 100);
        if (status != FLASH_SUCCESS)
        {
            NRF_LOG_INFO("winbond write fail");
        }

        NRF_LOG_INFO("winbond WRITE end");


        memset(recv_buf, 0, 2200);

        NRF_LOG_INFO("winbond read after write##############");
        status = w25n01gc_flash_read(&flash_dev, 10, recv_buf, 100);
        if (status != FLASH_SUCCESS)
        {
            NRF_LOG_INFO("winbond read fail");
        }
        NRF_LOG_INFO("winbond read wr end##############");

        int status = memcmp (tx_buf, recv_buf, 100);
        if (status == 0)
        {
             NRF_LOG_INFO("********MATCH*********");
        }
        else
        {
             NRF_LOG_INFO("********:(:( nnnooooooo*********");
        }
        NRF_LOG_INFO("");
        for (int i = 0; i < 100; i++)
        {
            tx_buf[i]++;
        }
        nrf_delay_ms(2000);
    }
}

