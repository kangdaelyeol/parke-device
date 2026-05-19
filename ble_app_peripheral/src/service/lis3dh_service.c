#include "lis3dh_driver.h"
#include "lis3dh_service.h"
#include <stdint.h>
#include "app_easy_timer.h"
#include "adv_service.h"
#include "power_service.h"

/**
 ****************************************************************************************
 * @file lis3dh.c
 * @brief LIS3DHTR Accelerometer Service
 ****************************************************************************************
 */



// Local variables
static int16_t lis3dh_prev_x __SECTION_ZERO("retention_mem_area0");
static int16_t lis3dh_prev_y __SECTION_ZERO("retention_mem_area0");
static int16_t lis3dh_prev_z __SECTION_ZERO("retention_mem_area0");

timer_hnd accelerometer_scan_hnd __SECTION_ZERO("retention_mem_area0");

// Helper functions

static void lis3dh_read_xyz(lis3dh_xyz *result)
{
    uint8_t buf[6];
    lis3dh_read_multi(LIS3DH_OUT_X_L, buf, 6);

    result->x = (int16_t)((buf[1] << 8) | buf[0]) >> 6;
    result->y = (int16_t)((buf[3] << 8) | buf[2]) >> 6;
    result->z = (int16_t)((buf[5] << 8) | buf[4]) >> 6;
}



// static Public functions
static int motion_detected(void)
{
    // static 변수 제거하고 전역 변수 사용
    int16_t x, y, z;
    int16_t dx, dy, dz;

    lis3dh_xyz xyz;
    lis3dh_read_xyz(&xyz);
    x = xyz.x;
    y = xyz.y;
    z = xyz.z;

    dx = x - lis3dh_prev_x;
    dy = y - lis3dh_prev_y;
    dz = z - lis3dh_prev_z;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    if (dz < 0) dz = -dz;

    lis3dh_prev_x = x;
    lis3dh_prev_y = y;
    lis3dh_prev_z = z;

    return (dx > ACCEL_MOTION_THRESHOLD || dy > ACCEL_MOTION_THRESHOLD || dz > ACCEL_MOTION_THRESHOLD) ? 1 : 0;
}

static void on_finish_advertising(void)
{
    adv_svc_stop_adv();
    adv_svc_go_to_sleep();
}

static void on_accelerometer_scan(void);

static void on_accelerometer_scan(void)
{
    accelerometer_scan_hnd = EASY_TIMER_INVALID_TIMER;

    if (motion_detected())
    {
        // 움직임 감지 → 광고 시작
        lis3dh_svc_stop_scan();
    
        power_svc_stop_cycle();
        adv_svc_start_adv();
        app_easy_timer(ADV_DURATION, on_finish_advertising); // 2초 후에 다시 체크
    }
    else {
        accelerometer_scan_hnd = app_easy_timer(ACCEL_CHECK_INTERVAL, on_accelerometer_scan);
    }
}

void lis3dh_svc_init(void) {
    if (lis3dh_detect())
    {
        lis3dh_init();
    }
}

void lis3dh_svc_start_scan(void) {
    if (accelerometer_scan_hnd != EASY_TIMER_INVALID_TIMER) {
        app_easy_timer_cancel(accelerometer_scan_hnd);
    }
    accelerometer_scan_hnd = app_easy_timer(ACCEL_CHECK_INTERVAL, on_accelerometer_scan);
}


void lis3dh_svc_stop_scan(void) {
    if (accelerometer_scan_hnd != EASY_TIMER_INVALID_TIMER) {
        app_easy_timer_cancel(accelerometer_scan_hnd);
        accelerometer_scan_hnd = EASY_TIMER_INVALID_TIMER;
    }
}