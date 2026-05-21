// lis3dh_driver.h
#ifndef _LIS3DH_DRIVER_H_
#define _LIS3DH_DRIVER_H_
#include <stdint.h>

/*
 * LIS3DH DEFINES
 ****************************************************************************************
 */


 #define LIS3DH_WHO_AM_I         0x0F    // Should return 0x33
 #define LIS3DH_CTRL_REG1        0x20
    #define LIS3DH_CTRL_REG4        0x23
    #define LIS3DH_OUT_X_L          0x28
    #define LIS3DH_OUT_X_H          0x29
    #define LIS3DH_OUT_Y_L          0x2A
    #define LIS3DH_OUT_Y_H          0x2B
    #define LIS3DH_OUT_Z_L          0x2C
    #define LIS3DH_OUT_Z_H          0x2D

#define LIS3DH_AUTO_INCREMENT   0x80    // Set MSB to auto-increment register address


int lis3dh_detect(void);

void lis3dh_init(void);
void lis3dh_driver_read_multi(uint8_t reg, uint8_t *buf, uint16_t len);


#endif // _LIS3DH_DRIVER_H_