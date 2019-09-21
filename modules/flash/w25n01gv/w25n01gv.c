#include "w25n01gv_internal.h"

/* static function declaration */
static int8_t w25n01gv_device_reset(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_read_jedec_id(flash_device_t* w25n01gc_flash,
                                     uint8_t* jedec_id, uint32_t len);
static int8_t w25n01gv_read_status_reg(flash_device_t* w25n01gc_flash,
                                       uint8_t reg_addr, uint8_t* reg_read);
static int8_t w25n01gv_write_status_reg(flash_device_t* w25n01gc_flash,
                                        uint8_t reg_addr, uint8_t* reg_write);
static int8_t w25n01gv_write_enable(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_write_disable(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_bad_block_mgmt(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_read_bbm_lut(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_last_ecc_failure_addr(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_block_erase(flash_device_t* w25n01gc_flash,
                                   uint16_t page_addr);
static int8_t w25n01gv_load_program_data(flash_device_t* w25n01gc_flash,
                                         bool random_load, uint16_t col_addr,
                                         uint8_t* buf, uint16_t buf_len);
static int8_t w25n01gv_quad_load_program_data(flash_device_t* w25n01gc_flash,
                                              bool random_load,
                                              uint16_t col_addr, uint8_t* buf,
                                              uint16_t buf_len);
static int8_t w25n01gv_program_execute(flash_device_t* w25n01gc_flash,
                                       uint16_t page_addr);
static int8_t w25n01gv_page_data_read(flash_device_t* w25n01gc_flash,
                                      uint16_t page_addr);
static int8_t w25n01gv_read_data(flash_device_t* w25n01gc_flash,
                                 uint16_t col_addr, uint8_t* buf,
                                 uint16_t buf_len);
static int8_t w25n01gv_fast_read(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_4byte_addr(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_dual_output(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_dual_output_4byte_addr(
    flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_quad_output(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_quad_output_4byte_addr(
    flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_dual_io(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_dual_io_4byte_addr(
    flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_quad_io(flash_device_t* w25n01gc_flash);
static int8_t w25n01gv_fast_read_quad_io_4byte_addr(
    flash_device_t* w25n01gc_flash);
static int8_t check_ecc(flash_device_t* w25n01gc_flash);
static int8_t check_fail(flash_device_t* w25n01gc_flash);
static int8_t check_busy(flash_device_t* w25n01gc_flash);
static int8_t set_block_protect(flash_device_t* w25n01gc_flash,
                                uint8_t block_protect_mode);
static int8_t wait_if_busy(flash_device_t* w25n01gc_flash,
                           uint32_t busy_timeout);

static int8_t check_ecc(flash_device_t* w25n01gc_flash)
{
    uint8_t reg_val;
    int8_t status = FLASH_SUCCESS;

    status =
        w25n01gv_read_status_reg(w25n01gc_flash, W25N01GV_STATUS_REG, &reg_val);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: read_status_reg fail!", __func__,
                  __LINE__);
        return status;
    }

    return ((reg_val & W25N01GV_ECC_MASK) >> W25N01GV_ECC_OFFSET);
}

static int8_t check_fail(flash_device_t* w25n01gc_flash)
{
    uint8_t reg_val;
    int8_t status = FLASH_SUCCESS;

    status =
        w25n01gv_read_status_reg(w25n01gc_flash, W25N01GV_STATUS_REG, &reg_val);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: read_status_reg fail!", __func__,
                  __LINE__);
        return status;
    }

    if (((reg_val & W25N01GV_EFAIL_MASK) >> W25N01GV_EFAIL_OFFSET) == 0x01)
    {
        LOG_FLASH(INFO, "W25N01GV: %s, %d: erase fail bit set!", __func__,
                  __LINE__);
        return ERASE_FAIL_CODE;
    }
    else if (((reg_val & W25N01GV_PFAIL_MASK) >> W25N01GV_PFAIL_OFFSET) == 0x01)
    {
        LOG_FLASH(INFO, "W25N01GV: %s, %d: program fail bit set!", __func__,
                  __LINE__);
        return PROGRAM_FAIL_CODE;
    }
    return ERASE_PROGRAM_SUCESS;
}

static int8_t check_busy(flash_device_t* w25n01gc_flash)
{
    uint8_t reg_val;
    int8_t status = FLASH_SUCCESS;

    status =
        w25n01gv_read_status_reg(w25n01gc_flash, W25N01GV_STATUS_REG, &reg_val);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: read_status_reg fail!", __func__,
                  __LINE__);
        return status;
    }

    return ((reg_val & W25N01GV_BUSY_MASK) >> W25N01GV_BUSY_OFFSET);
}

static int8_t set_block_protect(flash_device_t* w25n01gc_flash,
                                uint8_t block_protect_mode)
{
    uint8_t reg_val;
    uint8_t reg_write;
    int8_t status = FLASH_SUCCESS;

    status =
        w25n01gv_read_status_reg(w25n01gc_flash, W25N01GV_PROTECTION_REG, &reg_val);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: read_status_reg fail!", __func__,
                  __LINE__);
        return status;
    }

    reg_val &= ~W25N01GV_BP_MASK;
    reg_val &= ~W25N01GV_TB_MASK;

    reg_val |= ((block_protect_mode & 0x0F) << W25N01GV_BP_OFFSET);
    reg_val |= ((block_protect_mode & 0x10) << W25N01GV_TB_OFFSET);

    reg_write = reg_val;

    status = w25n01gv_write_status_reg(w25n01gc_flash, W25N01GV_PROTECTION_REG,
                                       &reg_val);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: write_status_reg fail!", __func__,
                  __LINE__);
        return status;
    }

    /* Recheck if the value is correct */
    status =
        w25n01gv_read_status_reg(w25n01gc_flash, W25N01GV_PROTECTION_REG, &reg_val);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: read_status_reg fail!", __func__,
                  __LINE__);
        return status;
    }

    if (reg_write != reg_val)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:written value mismatch!", __func__,
                  __LINE__);
        return status;
    }

    return FLASH_SUCCESS;
}

static int8_t wait_if_busy(flash_device_t* w25n01gc_flash, uint32_t busy_timeout)
{
    uint32_t iter = 0;
    int8_t status = FLASH_SUCCESS;
    /* TODO Implement custom timeout */
    while (status == 1)
    {
        if (iter == busy_timeout)
        {
            return FLASH_TIMEOUT;
        }
        w25n01gc_flash->sleep(W25N01GV_BUSY_SLEEP_TIME_MS);
        iter++;
        status = check_busy(w25n01gc_flash);
    }
    return status;
}

static int8_t w25n01gv_write_enable(flash_device_t* w25n01gc_flash)
{
    spi_transfer_t spi_xfer_data;
    int8_t status;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    spi_xfer_data.opcode = W25N01GV_WRITE_ENABLE;
    spi_xfer_data.dummy_cycles = W25N01GV_WRITE_ENABLE_DUMMY_CYCLES;
    spi_xfer_data.rx_len = 0;
    spi_xfer_data.tx_len = 0;

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_write_disable(flash_device_t* w25n01gc_flash)
{
    spi_transfer_t spi_xfer_data;
    int8_t status;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    spi_xfer_data.opcode = W25N01GV_WRITE_DISABLE;
    spi_xfer_data.dummy_cycles = W25N01GV_WRITE_DISABLE_DUMMY_CYCLES;
    spi_xfer_data.rx_len = 0;
    spi_xfer_data.tx_len = 0;

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_read_status_reg(flash_device_t* w25n01gc_flash,
                                       uint8_t reg_addr, uint8_t* reg_read)
{
    spi_transfer_t spi_xfer_data;
    int8_t status;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    spi_xfer_data.opcode = W25N01GV_READ_STATUS_REG;
    spi_xfer_data.dummy_cycles = W25N01GV_READ_STATUS_REG_DUMMY_CYCLES;
    spi_xfer_data.rx_buf = reg_read;
    spi_xfer_data.rx_len = 1;
    spi_xfer_data.tx_len = 0;
    spi_xfer_data.addr = &reg_addr;
    spi_xfer_data.addr_len = W25N01GV_SR_ADDR_SIZE;

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_write_status_reg(flash_device_t* w25n01gc_flash,
                                        uint8_t reg_addr, uint8_t* reg_write)
{
    spi_transfer_t spi_xfer_data;
    int8_t status;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    spi_xfer_data.opcode = W25N01GV_WRITE_STATUS_REG;
    spi_xfer_data.dummy_cycles = W25N01GV_WRITE_STATUS_REG_DUMMY_CYCLES;
    spi_xfer_data.rx_len = 0;
    spi_xfer_data.tx_buf = reg_write;
    spi_xfer_data.tx_len = 1;
    spi_xfer_data.addr = &reg_addr;
    spi_xfer_data.addr_len = W25N01GV_SR_ADDR_SIZE;

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_block_erase(flash_device_t* w25n01gc_flash,
                                   uint16_t page_addr)
{
    spi_transfer_t spi_xfer_data;
    int8_t status = FLASH_SUCCESS;
    uint8_t send_addr[2];

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    status = w25n01gv_write_enable(w25n01gc_flash);
    if (status != FLASH_SUCCESS)
    {
        return status;
    }

    send_addr[0] = (page_addr >> 8) & 0xFF;
    send_addr[1] = page_addr & 0xFF;

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    spi_xfer_data.opcode = W25N01GV_BLOCK_ERASE;
    spi_xfer_data.dummy_cycles = W25N01GV_BLOCK_ERASE_DUMMY_CYCLES;
    spi_xfer_data.dummy_cyles_pos = 0;
    spi_xfer_data.rx_len = 0;
    spi_xfer_data.tx_len = 0;
    spi_xfer_data.addr_len = W25N01GV_PAGE_ADDR_SIZE;
    spi_xfer_data.addr = send_addr;

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_load_program_data(flash_device_t* w25n01gc_flash,
                                         bool random_load, uint16_t col_addr,
                                         uint8_t* buf, uint16_t buf_len)
{
    spi_transfer_t spi_xfer_data;
    int8_t status = FLASH_SUCCESS;
    uint8_t send_addr[2];

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    status = w25n01gv_write_enable(w25n01gc_flash);
    if (status != FLASH_SUCCESS)
    {
        return status;
    }

    send_addr[0] = (col_addr >> 8) & 0xFF;
    send_addr[1] = col_addr & 0xFF;

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    if (random_load)
    {
        spi_xfer_data.opcode = W25N01GV_RANDOM_PROGRAM_DATA_LOAD;
    }
    else
    {
        spi_xfer_data.opcode = W25N01GV_PROGRAM_DATA_LOAD;
    }
    spi_xfer_data.dummy_cycles = W25N01GV_PROGRAM_DATA_LOAD_DUMMY_CYCLES;
    spi_xfer_data.rx_len = 0;
    spi_xfer_data.tx_len = buf_len;
    spi_xfer_data.tx_buf = buf;
    spi_xfer_data.addr_len = W25N01GV_COLUMN_ADDR_SIZE;
    spi_xfer_data.addr = send_addr;

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_program_execute(flash_device_t* w25n01gc_flash,
                                       uint16_t page_addr)
{
    spi_transfer_t spi_xfer_data;
    int8_t status = FLASH_SUCCESS;
    uint8_t send_addr[2];

    send_addr[0] = (page_addr >> 8) & 0xFF;
    send_addr[1] = page_addr & 0xFF;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    spi_xfer_data.opcode = W25N01GV_PROGRAM_EXECUTE;
    spi_xfer_data.dummy_cycles = W25N01GV_PROGRAM_EXECUTE_DUMMY_CYCLES;
    spi_xfer_data.dummy_cyles_pos = 0;
    spi_xfer_data.rx_len = 0;
    spi_xfer_data.tx_len = 0;
    spi_xfer_data.addr_len = W25N01GV_PAGE_ADDR_SIZE;
    spi_xfer_data.addr = send_addr;

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_page_data_read(flash_device_t* w25n01gc_flash,
                                      uint16_t page_addr)
{
    spi_transfer_t spi_xfer_data;
    int8_t status = FLASH_SUCCESS;
    uint8_t send_addr[2];

    send_addr[0] = (page_addr >> 8) & 0xFF;
    send_addr[1] = page_addr & 0xFF;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    spi_xfer_data.opcode = W25N01GV_PAGE_DATA_READ;
    spi_xfer_data.dummy_cycles = W25N01GV_PAGE_DATA_READ_DUMMY_CYCLES;
    spi_xfer_data.dummy_cyles_pos = 0;
    spi_xfer_data.rx_len = 0;
    spi_xfer_data.tx_len = 0;
    spi_xfer_data.addr_len = W25N01GV_COLUMN_ADDR_SIZE;
    spi_xfer_data.addr = send_addr;

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_read_data(flash_device_t* w25n01gc_flash,
                                 uint16_t col_addr, uint8_t* buf,
                                 uint16_t buf_len)
{
    spi_transfer_t spi_xfer_data;
    int8_t status = FLASH_SUCCESS;
    uint8_t send_addr[2];

    send_addr[0] = (col_addr >> 8) & 0xFF;
    send_addr[1] = col_addr & 0xFF;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    /* Wait for busy bit */
    status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
    if (status != FLASH_SUCCESS)
    {
        if (status == FLASH_TIMEOUT)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                      __func__, __LINE__);
        }
        else
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!", __func__,
                      __LINE__);
        }
        return status;
    }

    spi_xfer_data.opcode = W25N01GV_READ;
    spi_xfer_data.dummy_cycles = W25N01GV_READ_DUMMY_CYCLES;
    spi_xfer_data.dummy_cyles_pos = 1;
    spi_xfer_data.tx_len = 0;
    spi_xfer_data.rx_len = buf_len;
    spi_xfer_data.rx_buf = buf;
    spi_xfer_data.addr_len = W25N01GV_COLUMN_ADDR_SIZE;
    spi_xfer_data.addr = send_addr;

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t w25n01gv_read_jedec_id(flash_device_t* w25n01gc_flash,
                                     uint8_t* jedec_id, uint32_t len)
{
    spi_transfer_t spi_xfer_data;
    int8_t status;

    memset(&spi_xfer_data, 0, sizeof(spi_transfer_t));

    spi_xfer_data.opcode = W25N01GV_JEDEC_ID;
    spi_xfer_data.dummy_cycles = W25N01GV_JEDEC_ID_DUMMY_CYCLES;
    spi_xfer_data.dummy_cyles_pos = 0;
    spi_xfer_data.rx_buf = jedec_id;
    spi_xfer_data.rx_len = len;
    spi_xfer_data.tx_len = 0;

    status = flash_spi_transfer(w25n01gc_flash, &spi_xfer_data);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d:  spi transfer failed", __func__,
                  __LINE__);
        return status;
    }

    return status;
}

static int8_t check_jedec_id(uint8_t* jedec_id, const uint8_t* cmp_id, uint16_t len)
{
    for (int32_t i = 0; i < len; i++)
    {
        if (jedec_id[i] != cmp_id[i])
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: JEDEC ID mismatch", __func__,
                      __LINE__);
            return FLASH_DETECT_FAIL;
        }
    }
    return FLASH_SUCCESS;
}

int8_t detect_w25n01gv(flash_device_t* w25n01gc_flash)
{
    int8_t status = FLASH_SUCCESS;
    uint8_t jedec_id[W25N01GV_JEDEC_ID_SIZE];
    status = w25n01gv_read_jedec_id(w25n01gc_flash, jedec_id, W25N01GV_JEDEC_ID_SIZE);
    if (status != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: Read jedec id fail", __func__,
                  __LINE__);
        return status;
    }

    if (check_jedec_id(jedec_id, flash_list[0].flash_dev_id,
                       W25N01GV_JEDEC_ID_SIZE) != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: Check jedec_id fail", __func__,
                  __LINE__);
        return FLASH_DETECT_FAIL;
    }
    return FLASH_SUCCESS;
}

int8_t w25n01gv_init(flash_device_t* w25n01gc_flash)
{
    if (w25n01gc_flash->spi_xfer == NULL)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: spi_xfer NULL", __func__,
                  __LINE__);
        return FLASH_INVALID_PARAMS;
    }

    if (detect_w25n01gv(w25n01gc_flash) != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: Detect Flash fail", __func__,
                  __LINE__);
        return FLASH_DETECT_FAIL;
    }

    if (set_block_protect(w25n01gc_flash, BLOCK_PROTECT_NONE) != FLASH_SUCCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: Set block protect fail", __func__,
                  __LINE__);
        return FLASH_MISC_FAILURE;
    }
}

int8_t w25n01gc_flash_read(flash_device_t* w25n01gc_flash, uint32_t addr,
                  uint8_t* read_buf, uint32_t read_len)
{
    int8_t status = FLASH_SUCCESS;
    uint16_t page_addr;
    uint16_t col_addr;
    uint32_t rem_len = read_len;
    uint32_t read_len_page = 0;

    if ((addr + read_len) > w25n01gc_flash->flash_size)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: Invalid read addr", __func__,
                  __LINE__);
        return FLASH_INVALID_PARAMS;
    }

    page_addr = addr / w25n01gc_flash->page_size;
    col_addr = addr % w25n01gc_flash->page_size;

    while (rem_len > 0)
    {
        if (rem_len > (w25n01gc_flash->page_size - col_addr))
        {
            read_len_page = w25n01gc_flash->page_size - col_addr;
        }
        else
        {
            read_len_page = rem_len;
        }

        status = w25n01gv_page_data_read(w25n01gc_flash, page_addr);
        if (status != FLASH_SUCCESS)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d:  page data read fail", __func__,
                      __LINE__);
            return status;
        }

        status =
            w25n01gv_read_data(w25n01gc_flash, col_addr,
                               read_buf + (read_len - rem_len), read_len_page);
        if (status != FLASH_SUCCESS)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d:  read data read fail", __func__,
                      __LINE__);
            return status;
        }

        status = check_ecc(w25n01gc_flash);
        if ((status != ECC_SUCCESS_NO_CORRECTION) &&
            (status != ECC_SUCCESS_CORRECTION))
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d:  read data ECC error", __func__,
                      __LINE__);
            return status;
        }

        rem_len -= read_len_page;
        col_addr = 0;
        page_addr++;
    }

    return status;
}

int8_t w25n01gc_flash_write(flash_device_t* w25n01gc_flash, uint32_t addr,
                   uint8_t* write_buf, uint32_t write_len)
{
    int8_t status = FLASH_SUCCESS;
    uint16_t page_addr;
    uint16_t col_addr;
    uint32_t rem_len = write_len;
    uint32_t write_len_page = 0;

    if ((addr + write_len) > w25n01gc_flash->flash_size)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: Invalid read addr", __func__,
                  __LINE__);
        return FLASH_INVALID_PARAMS;
    }

    /* Checking fail incase there was a previous error. Not returning from this error. */
    status = check_fail(w25n01gc_flash);
    if (status != ERASE_PROGRAM_SUCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: previous operation error",
                  __func__, __LINE__);
    }

    page_addr = addr / w25n01gc_flash->page_size;
    col_addr = addr % w25n01gc_flash->page_size;

    while (rem_len > 0)
    {
        if (rem_len > (w25n01gc_flash->page_size - col_addr))
        {
            write_len_page = w25n01gc_flash->page_size - col_addr;
        }
        else
        {
            write_len_page = rem_len;
        }

        status = w25n01gv_load_program_data(w25n01gc_flash, true, col_addr,
                                            write_buf + (write_len - rem_len),
                                            write_len_page);
        if (status != FLASH_SUCCESS)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: load program data fail",
                      __func__, __LINE__);
            return status;
        }

        status = w25n01gv_program_execute(w25n01gc_flash, page_addr);
        if (status != FLASH_SUCCESS)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: program execute fail", __func__,
                      __LINE__);
            return status;
        }

        /* 
         * FIXME Double wait here and in program func, mostly it wont affect since we
         * wait here, the above wait is resolved immediately after single status reg read.
         */

        /* Wait for busy bit */
        status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_DEFAULT_TIMEOUT_MS);
        if (status != FLASH_SUCCESS)
        {
            if (status == FLASH_TIMEOUT)
            {
                LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                          __func__, __LINE__);
            }
            else
            {
                LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!",
                          __func__, __LINE__);
            }
            return status;
        }

        status = check_fail(w25n01gc_flash);
        if (status != ERASE_PROGRAM_SUCESS)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: program data operation error",
                      __func__, __LINE__);
            return status;
        }

        rem_len -= write_len_page;
        col_addr = 0;
        page_addr++;
    }
}

int8_t w25n01gc_flash_erase(flash_device_t* w25n01gc_flash, uint32_t addr,
                   uint32_t erase_len)
{
    int8_t status = FLASH_SUCCESS;
    uint16_t page_addr;
    uint32_t rem_len = erase_len;
    uint32_t erase_len_page = 0;

    if ((addr + erase_len) > w25n01gc_flash->flash_size)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: Invalid read addr", __func__,
                  __LINE__);
        return FLASH_INVALID_PARAMS;
    }

    /* Checking fail incase there was a previous error. Not returning from this error. */
    status = check_fail(w25n01gc_flash);
    if (status != ERASE_PROGRAM_SUCESS)
    {
        LOG_FLASH(ERROR, "W25N01GV: %s, %d: previous operation error",
                  __func__, __LINE__);
    }

    page_addr = addr / w25n01gc_flash->page_size;
    erase_len_page = w25n01gc_flash->page_size - (addr % w25n01gc_flash->page_size);

    while (rem_len > 0)
    {
        status = w25n01gv_block_erase(w25n01gc_flash, page_addr);
        if (status != FLASH_SUCCESS)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: block_erase fail",
                      __func__, __LINE__);
            return status;
        }

        /* Wait for busy bit */
        status = wait_if_busy(w25n01gc_flash, W25N01GV_BUSY_ERASE_TIMEOUT_MS);
        if (status != FLASH_SUCCESS)
        {
            if (status == FLASH_TIMEOUT)
            {
                LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy timeout!",
                          __func__, __LINE__);
            }
            else
            {
                LOG_FLASH(ERROR, "W25N01GV: %s, %d: wait if busy fail!",
                          __func__, __LINE__);
            }
            return status;
        }

        status = check_fail(w25n01gc_flash);
        if (status != ERASE_PROGRAM_SUCESS)
        {
            LOG_FLASH(ERROR, "W25N01GV: %s, %d: erase operation error",
                      __func__, __LINE__);
            return status;
        }

        if (rem_len < erase_len_page)
        {
            break;
        }
        rem_len -= erase_len_page;
        erase_len_page = w25n01gc_flash->page_size;
        page_addr++;
    }

    return status;
}
