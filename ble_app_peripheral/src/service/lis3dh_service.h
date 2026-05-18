// lis3dh_service.h
#ifndef _LIS3DH_SERVICE_H_
#define _LIS3DH_SERVICE_H_

// Defines
#ifndef ACCEL_MOTION_THRESHOLD
#define ACCEL_MOTION_THRESHOLD  20    // 움직임 감지 임계값

#endif

#include <stdint.h>

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
} lis3dh_xyz;

int lis3dh_motion_detected(void);

void lis3dh_service_init(void);

#endif // _LIS3DH_SERVICE_H_