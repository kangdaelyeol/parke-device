#include "rwip_config.h"
#include "gattc_task.h"
#include "gap.h"
#include "app_easy_timer.h"
#include "power_service.h"
#include "user_peripheral.h"

timer_hnd sleep_mode_timer __SECTION_ZERO("retention_mem_area0");
timer_hnd wakeup_mode_timer __SECTION_ZERO("retention_mem_area0");

// static functions

static void on_sleep(void);
static void on_wakeup(void);

static void on_sleep(void)
{
    wakeup_mode_timer = EASY_TIMER_INVALID_TIMER;
    arch_set_sleep_mode(ARCH_EXT_SLEEP_ON);
    sleep_mode_timer = app_easy_timer(SLEEP_DURATION, on_wakeup);
}


static void on_wakeup(void)
{   
    sleep_mode_timer = EASY_TIMER_INVALID_TIMER;
    arch_set_sleep_mode(ARCH_SLEEP_OFF);
    wakeup_mode_timer = app_easy_timer(WAKEUP_DURATION, on_sleep);
}

// public functions

void power_svc_init(void)
{
    // 타이머 초기화
    sleep_mode_timer = EASY_TIMER_INVALID_TIMER;
    wakeup_mode_timer = EASY_TIMER_INVALID_TIMER;
}

void power_svc_start_sleep_wakeup_cycle(void)
{
    // 슬립 모드 타이머 시작
    if (sleep_mode_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(sleep_mode_timer);
        sleep_mode_timer = EASY_TIMER_INVALID_TIMER;
    }
    if(wakeup_mode_timer != EASY_TIMER_INVALID_TIMER){
        app_easy_timer_cancel(wakeup_mode_timer);
        wakeup_mode_timer = EASY_TIMER_INVALID_TIMER;
    }
    on_wakeup();
}

void power_svc_go_to_sleep(void)
{
    if (sleep_mode_timer != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(sleep_mode_timer);
        sleep_mode_timer = EASY_TIMER_INVALID_TIMER;
    }
    if(wakeup_mode_timer != EASY_TIMER_INVALID_TIMER){
        app_easy_timer_cancel(wakeup_mode_timer);
        wakeup_mode_timer = EASY_TIMER_INVALID_TIMER;
    }
    on_sleep();
}


sleep_mode_t user_app_validate_sleep(sleep_mode_t sleep_mode) {
    return mode_sleeping;
}