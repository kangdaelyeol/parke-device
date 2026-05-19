#include <stdint.h>
#include "rwip_config.h"
#include "gattc_task.h"
#include "gap.h"
#include "app_easy_timer.h"
#include "user_config.h"
#include "adv_service.h"
#include "mnf_service.h"

uint8_t is_advertising __SECTION_ZERO("retention_mem_area0");
timer_hnd adv_service_timer __SECTION_ZERO("retention_mem_area0");

// Static methods



// Public methods

void adv_svc_timer_init(void) {
    is_advertising = 0;
    adv_service_timer = EASY_TIMER_INVALID_TIMER;
}

void adv_svc_start_undirected_adv(void) {
    if (is_advertising) return;
    
    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    
    if (cmd == NULL)
    {
        return;
    }

    mnf_svc_update_battery_level()
    const uint8_t *adv_data = mnf_svc_get_mnf_data();

    memcpy(&cmd->info.host.adv_data[13], adv_data, 11);
    cmd->info.host.adv_data_len = USER_ADVERTISE_DATA_LEN;

    app_easy_gap_undirected_advertise_start();
    is_advertising = 1;
}

void adv_svc_start_directed_adv(void) {
    if (is_advertising) return;

    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    if(cmd == NULL){
        return;
    }

    app_easy_gap_directed_advertise_start();
    is_advertising = 1;
}

void adv_svc_stop_adv(void) {
    if (!is_advertising) return;

    app_easy_gap_advertise_stop();
    is_advertising = 0;
}