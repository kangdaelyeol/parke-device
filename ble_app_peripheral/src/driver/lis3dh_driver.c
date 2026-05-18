#include "i2c.h"
#include "lis3dh.h"
#include "lis3dh_driver.h"
static int16_t lis3dh_prev_x __SECTION_ZERO("retention_mem_area0");
static int16_t lis3dh_prev_y __SECTION_ZERO("retention_mem_area0");
static int16_t lis3dh_prev_z __SECTION_ZERO("retention_mem_area0");

/*
 * I2C Helper Functions
 ****************************************************************************************
 */
static void i2c_wait_until_ready(void)
{
    while (i2c_is_master_busy());
    while (i2c_controler_is_busy());
}

static void lis3dh_write_reg(uint8_t reg, uint8_t value)
{
    i2c_master_transmit_buffer_sync(&reg, 1, NULL, I2C_F_NONE);
    i2c_master_transmit_buffer_sync(&value, 1, NULL, I2C_F_ADD_STOP);
    i2c_wait_until_ready();
}

static uint8_t lis3dh_read_reg(uint8_t reg)
{
    uint8_t value = 0;
    i2c_master_transmit_buffer_sync(&reg, 1, NULL, I2C_F_NONE);
    i2c_master_receive_buffer_sync(&value, 1, NULL, I2C_F_ADD_STOP);
    i2c_wait_until_ready();
    return value;
}

static void lis3dh_read_multi(uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t reg_addr = reg | LIS3DH_AUTO_INCREMENT;
    i2c_master_transmit_buffer_sync(&reg_addr, 1, NULL, I2C_F_NONE);
    i2c_master_receive_buffer_sync(buf, len, NULL, I2C_F_ADD_STOP);
    i2c_wait_until_ready();
}

/*
 * Public Functions
 ****************************************************************************************
 */

// WHO_AM_I 확인 → 1=정상, 0=실패
int lis3dh_detect(void)
{
    return (lis3dh_read_reg(LIS3DH_WHO_AM_I) == 0x33) ? 1 : 0;
}

void lis3dh_init(void)
{
    // 100Hz, 모든 축 활성화, Normal mode
    lis3dh_write_reg(LIS3DH_CTRL_REG1, 0x57);
    // ±2g, Normal mode (10-bit)
    lis3dh_write_reg(LIS3DH_CTRL_REG4, 0x00);
}

const uint8_t* lis3dh_get_xyz_buffer(void) {
    static uint8_t buf[6];
    lis3dh_read_multi(LIS3DH_OUT_X_L, buf, 6);

    return buf;
}