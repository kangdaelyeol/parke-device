// lis3dh_service.h
#ifndef _LIS3DH_SERVICE_H_
#define _LIS3DH_SERVICE_H_

// Defines
#ifndef ACCEL_MOTION_THRESHOLD
#define ACCEL_MOTION_THRESHOLD  40    // 움직임 감지 임계값

#define ACCEL_CHECK_INTERVAL 100   // 가속도 체크 간격 (단위: 10ms)
#define ADV_DURATION 500         // 광고 지속 시간 (단위: 10ms)

#endif

#include <stdint.h>

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
} lis3dh_xyz;


void lis3dh_svc_init(void);
void lis3dh_svc_start_scan(void);
void lis3dh_svc_stop_scan(void);
int16_t get_accelerometer_dx(void);

#endif // _LIS3DH_SERVICE_H_