/**
 ****************************************************************************************
 * @file lis3dh.c
 * @brief LIS3DHTR Accelerometer Driver
 ****************************************************************************************
 */

#include "lis3dh.h"
#include "i2c.h"


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

void lis3dh_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t buf[6];
    lis3dh_read_multi(LIS3DH_OUT_X_L, buf, 6);

    *x = (int16_t)((buf[1] << 8) | buf[0]) >> 6;
    *y = (int16_t)((buf[3] << 8) | buf[2]) >> 6;
    *z = (int16_t)((buf[5] << 8) | buf[4]) >> 6;
}

// 이전 값과 비교해서 움직임 감지
// threshold: 감지 민감도 (예: 20 = 약한 움직임, 50 = 강한 움직임)
// 반환값: 1=움직임 감지, 0=정지
int lis3dh_motion_detected(int16_t threshold)
{
    // static 변수 제거하고 전역 변수 사용
    int16_t x, y, z;
    int16_t dx, dy, dz;

    lis3dh_read_xyz(&x, &y, &z);

    dx = x - lis3dh_prev_x;
    dy = y - lis3dh_prev_y;
    dz = z - lis3dh_prev_z;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    if (dz < 0) dz = -dz;

    lis3dh_prev_x = x;
    lis3dh_prev_y = y;
    lis3dh_prev_z = z;

    return (dx > threshold || dy > threshold || dz > threshold) ? 1 : 0;
}