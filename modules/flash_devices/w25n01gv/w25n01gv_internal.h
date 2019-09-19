#ifndef _W25N01GV_INTERNAL_H_
#define _W25N01GV_INTERNAL_H_

#include "ext_flash.h"

typedef struct w25n01gv_instr
{
    uint8_t opcode;
    uint8_t dummy_cycles;
} w25n01gv_instr_t;

/* Utility macros */
#define W25N01GV_OPCODE_SIZE                          1U
#define W25N01GV_SR_ADDR_SIZE                         1U
#define W25N01GV_PAGE_ADDR_SIZE                       2U
#define W25N01GV_BLOCK_ADDR_SIZE                      2U
#define W25N01GV_COLUMN_ADDR_SIZE                     2U
#define W25N01GV_JEDEC_ID_SIZE                        3U

#define W25N01GV_BUSY_SLEEP_TIME_MS                   (15U)
#define W25N01GV_BUSY_DEFAULT_TIMEOUT_MS              (10U)
#define W25N01GV_BUSY_ERASE_TIMEOUT_MS                (10U)

/* Commands */
#define W25N01GV_DEVICE_RESET                         (0xFF)
#define W25N01GV_JEDEC_ID                             (0x9F)
#define W25N01GV_READ_STATUS_REG                      (0x0F)
#define W25N01GV_WRITE_STATUS_REG                     (0x1F)
#define W25N01GV_WRITE_ENABLE                         (0x06)
#define W25N01GV_WRITE_DISABLE                        (0x04)
#define W25N01GV_PROGRAM_EXECUTE                      (0x10)
#define W25N01GV_PAGE_DATA_READ                       (0x13)
#define W25N01GV_READ                                 (0x03)  
#define W25N01GV_FAST_READ                            (0x0B)
#define W25N01GV_FAST_READ_4BYTE_ADDR                 (0x0C)
#define W25N01GV_FAST_READ_DUAL_OUTPUT                (0x3B)
#define W25N01GV_FAST_READ_DUAL_OUTPUT_4BYTE_ADDR     (0x3B)
#define W25N01GV_FAST_READ_QUAD_OUTPUT                (0x6B)
#define W25N01GV_FAST_READ_QUAD_OUTPUT_4BYTE_ADDR     (0x6C)
#define W25N01GV_FAST_READ_DUAL_IO                    (0xBB)
#define W25N01GV_FAST_READ_DUAL_IO_4BYTE_ADDR         (0xBC)
#define W25N01GV_FAST_READ_QUAD_IO                    (0xEB)
#define W25N01GV_FAST_READ_QUAD_IO_4BYTE_ADDR         (0xEC)
#define W25N01GV_BB_MANAGEMENT                        (0xA1)
#define W25N01GV_READ_BBM_LUT                         (0xA5)
#define W25N01GV_LAST_ECC_FAILURE_PAGE_ADDR           (0xA9)
#define W25N01GV_BLOCK_ERASE                          (0xD8)
#define W25N01GV_PROGRAM_DATA_LOAD                    (0x02)
#define W25N01GV_RANDOM_PROGRAM_DATA_LOAD             (0x84)
#define W25N01GV_QUAD_PROGRAM_DATA_LOAD               (0x32)
#define W25N01GV_RANDOM_QUAD_PROGRAM_DATA_LOAD        (0x34)

/* Dummy cycles for commands */
#define W25N01GV_JEDEC_ID_DUMMY_CYCLES                             (1)
#define W25N01GV_READ_STATUS_REG_DUMMY_CYCLES                      (0)
#define W25N01GV_WRITE_STATUS_REG_DUMMY_CYCLES                     (0)
#define W25N01GV_WRITE_ENABLE_DUMMY_CYCLES                         (0)
#define W25N01GV_WRITE_DISABLE_DUMMY_CYCLES                        (0)
#define W25N01GV_PROGRAM_EXECUTE_DUMMY_CYCLES                      (1)
#define W25N01GV_PAGE_DATA_READ_DUMMY_CYCLES                       (1)
#define W25N01GV_READ_DUMMY_CYCLES                                 (1)
/* #define W25N01GV_FAST_READ_DUMMY_CYCLES                            (1) */
/* #define W25N01GV_FAST_READ_4BYTE_ADDR_DUMMY_CYCLES                 (3) */
/* #define W25N01GV_FAST_READ_DUAL_OUTPUT_DUMMY_CYCLES                (1) */
/* #define W25N01GV_FAST_READ_DUAL_OUTPUT_4BYTE_ADDR_DUMMY_CYCLES     (3) */
/* #define W25N01GV_FAST_READ_QUAD_OUTPUT_DUMMY_CYCLES                (1) */
/* #define W25N01GV_FAST_READ_QUAD_OUTPUT_4BYTE_ADDR_DUMMY_CYCLES     (3) */
/* #define W25N01GV_FAST_READ_DUAL_IO_DUMMY_CYCLES                    (4) */
/* #define W25N01GV_FAST_READ_DUAL_IO_4BYTE_ADDR_DUMMY_CYCLES         (5) */
/* #define W25N01GV_FAST_READ_QUAD_IO_DUMMY_CYCLES                    (6) */
/* #define W25N01GV_FAST_READ_QUAD_IO_4BYTE_ADDR_DUMMY_CYCLES         (7) */
#define W25N01GV_BB_MANAGEMENT_DUMMY_CYCLES                        (0)
#define W25N01GV_READ_BBM_LUT_DUMMY_CYCLES                         (1)
#define W25N01GV_LAST_ECC_FAILURE_PAGE_ADDR_DUMMY_CYCLES           (1)
#define W25N01GV_BLOCK_ERASE_DUMMY_CYCLES                          (1)
#define W25N01GV_PROGRAM_DATA_LOAD_DUMMY_CYCLES                    (0)
#define W25N01GV_RANDOM_PROGRAM_DATA_LOAD_DUMMY_CYCLES             (0)
#define W25N01GV_QUAD_PROGRAM_DATA_LOAD_DUMMY_CYCLES               (0)
#define W25N01GV_RANDOM_QUAD_PROGRAM_DATA_LOAD_DUMMY_CYCLES        (0)


/* Status Registers */
#define W25N01GV_PROTECTION_REG                       (0xA0)
#define W25N01GV_BP_OFFSET                            (3)
#define W25N01GV_BP_MASK                              (0x78)
#define W25N01GV_SRP1_OFFSET                          (0)
#define W25N01GV_SRP1_MASK                            (0x01)
#define W25N01GV_SRP0_OFFSET                          (7)
#define W25N01GV_SRP0_MASK                            (0x80)
#define W25N01GV_WPE_OFFSET                           (1)
#define W25N01GV_WPE_MASK                             (0x02)
#define W25N01GV_TB_OFFSET                            (2)
#define W25N01GV_TB_MASK                              (0x04)

#define W25N01GV_CONFIGURATION_REG                    (0xB0)
#define W25N01GV_OTPL_OFFSET                          (7)
#define W25N01GV_OTPL_MASK                            (0x80)
#define W25N01GV_OTPE_OFFSET                          (6)
#define W25N01GV_OTPE_MASK                            (0x40)
#define W25N01GV_SR1L_OFFSET                          (5)
#define W25N01GV_SR1L_MASK                            (0x20)
#define W25N01GV_ECCE_OFFSET                          (4)
#define W25N01GV_ECCE_MASK                            (0x10)
#define W25N01GV_BUF_OFFSET                           (3)
#define W25N01GV_BUF_MASK                             (0x08)

#define W25N01GV_STATUS_REG                           (0xC0)
#define W25N01GV_LUTF_OFFSET                          (6)
#define W25N01GV_LUTF_MASK                            (0x40)
#define W25N01GV_ECC_OFFSET                           (4)
#define W25N01GV_ECC_MASK                             (0x30)
#define W25N01GV_ECC1_OFFSET                          (5)
#define W25N01GV_ECC1_MASK                            (0x20)
#define W25N01GV_ECC0_OFFSET                          (4)
#define W25N01GV_ECC0_MASK                            (0x10)
#define W25N01GV_PFAIL_OFFSET                         (3)
#define W25N01GV_PFAIL_MASK                           (0x08)
#define W25N01GV_EFAIL_OFFSET                         (2)
#define W25N01GV_EFAIL_MASK                           (0x04)
#define W25N01GV_WEL_OFFSET                           (1)
#define W25N01GV_WEL_MASK                             (0x02)
#define W25N01GV_BUSY_OFFSET                          (0)
#define W25N01GV_BUSY_MASK                            (0x01)

enum ecc_code
{
    ECC_SUCCESS_NO_CORRECTION = 0,
    ECC_SUCCESS_CORRECTION,
    ECC_FAIL_SINGLE_PAGE,
    ECC_FAIL_MULTIPLE
};

enum fail_code
{
    ERASE_PROGRAM_SUCESS = 0,
    ERASE_FAIL_CODE,
    PROGRAM_FAIL_CODE
};

enum block_protection_mode
{
    BLOCK_PROTECT_NONE = 0x00000,
    BLOCK_PROTECT_U_256K = 0x00001,
    BLOCK_PROTECT_U_512K = 0x00010,
    BLOCK_PROTECT_U_1M = 0x00011,
    BLOCK_PROTECT_U_2M = 0x00100,
    BLOCK_PROTECT_U_4M = 0x00101,
    BLOCK_PROTECT_U_8M = 0x00110,
    BLOCK_PROTECT_U_16M = 0x00111,
    BLOCK_PROTECT_U_32M = 0x01000,
    BLOCK_PROTECT_U_64M = 0x01001,
    BLOCK_PROTECT_L_256K = 0x10001,
    BLOCK_PROTECT_L_512K = 0x10010,
    BLOCK_PROTECT_L_1M = 0x10011,
    BLOCK_PROTECT_L_2M = 0x10100,
    BLOCK_PROTECT_L_4M = 0x10101,
    BLOCK_PROTECT_L_8M = 0x10110,
    BLOCK_PROTECT_L_16M = 0x10111,
    BLOCK_PROTECT_L_32M = 0x11000,
    BLOCK_PROTECT_L_64M = 0x11001,
    BLOCK_PROTECT_ALL = 0x01100,
};

/* Function declarations */
int8_t detect_w25n01gv(flash_device_t* w25n01gc_flash);
int8_t w25n01gv_init(flash_device_t* w25n01gc_flash);
int8_t w25n01gc_flash_read(flash_device_t* w25n01gc_flash, uint32_t addr,
                  uint8_t* read_buf, uint32_t read_len);
int8_t w25n01gc_flash_write(flash_device_t* w25n01gc_flash, uint32_t addr,
                   uint8_t* write_buf, uint32_t write_len);
int8_t w25n01gc_flash_erase(flash_device_t* w25n01gc_flash, uint32_t addr,
                   uint32_t erase_len);

#endif
