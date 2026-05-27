#include "i2c.h"
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



/*
 * Public Functions
 ****************************************************************************************
 */

// WHO_AM_I 확인 → 1=정상, 0=실패
int lis3dh_detect(void)
{
    return (lis3dh_read_reg(LIS3DH_WHO_AM_I) == 0x33) ? 1 : 0;
}

void lis3dh_dv_init(void)
{
    i2c_wait_until_ready();
        // CTRL_REG1: 0x57 = 100Hz, all axes enabled, normal mode
    // 0101 0111 = ODR=0101 (100Hz), LPen=0, Zen=1, Yen=1, Xen=1
    lis3dh_write_reg(LIS3DH_CTRL_REG1, 0x57);
    
    // CTRL_REG4: 0x00 = ±2g, BDU=0, Normal mode (10-bit)
    lis3dh_write_reg(LIS3DH_CTRL_REG4, 0x00);
}

void lis3dh_driver_read_multi(uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t reg_addr = reg | LIS3DH_AUTO_INCREMENT;
    i2c_master_transmit_buffer_sync(&reg_addr, 1, NULL, I2C_F_NONE);
    i2c_master_receive_buffer_sync(buf, len, NULL, I2C_F_ADD_STOP);
    i2c_wait_until_ready();
}