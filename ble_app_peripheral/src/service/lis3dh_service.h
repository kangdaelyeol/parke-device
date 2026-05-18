// lis3dh_service.h
#ifndef _LIS3DH_SERVICE_H_
#define _LIS3DH_SERVICE_H_

#include <stdint.h>

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
} lis3dh_xyz;

int lis3dh_motion_detected(void);

#endif // _LIS3DH_SERVICE_H_