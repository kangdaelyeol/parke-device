#include <stdint.h>
#include "rwip_config.h"
#include "gattc_task.h"
#include "gap.h"
#include "app_easy_timer.h"
#include "user_config.h"
#include "adv_service.h"
#include "mnf_service.h"
#include "user_peripheral.h"
#include "lis3dh_service.h"

uint8_t is_advertising __SECTION_ZERO("retention_mem_area0");


// state machine variables
adv_mode_t adv_mode __SECTION_ZERO("retention_mem_area0");


// test for accelometer
timer_hnd adv_timer __SECTION_ZERO("retention_mem_area0");


void update_mnf_dx() {
    int16_t dx = get_accelerometer_dx();
    uint8_t* mnf = (uint8_t*)mnf_dv_get_mnf_data();
    mnf[MNF_COMPANY_LEN + MNF_DEVICE_ID_LEN + MNF_BATTERY_LEVEL_LEN] = (uint8_t)(dx & 0xFF);
    mnf[MNF_COMPANY_LEN + MNF_DEVICE_ID_LEN + MNF_BATTERY_LEVEL_LEN + 1] = (uint8_t)((dx >> 8) & 0xFF);
    
    uint8_t adv_data[USER_ADVERTISE_DATA_LEN];
    memcpy(adv_data, USER_ADVERTISE_DATA, USER_ADVERTISE_DATA_LEN);
    memcpy(&adv_data[6], mnf, MNF_DATA_LEN);
    app_easy_gap_update_adv_data(adv_data, USER_ADVERTISE_DATA_LEN, NULL ,0);
}

void adv_timer_callback() {
    update_mnf_dx();
    adv_timer = app_easy_timer(50, adv_timer_callback);
}


// Public methods

void adv_svc_init(void) {
    is_advertising = 0;
    adv_mode = ADV_DEFAULT;
    adv_timer = EASY_TIMER_INVALID_TIMER;
}

void adv_svc_stop_adv(adv_mode_t mode) {
    if (!is_advertising) return;

    app_easy_gap_advertise_stop();
    is_advertising = 0;
    adv_mode = mode;
}

void adv_svc_start_undirected_adv(void) {
    if (is_advertising) return;
    
    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    
    if (cmd == NULL)
    {
        return;
    }

    mnf_svc_update_battery_level();

    const uint8_t* mnf = mnf_svc_get_mnf_data();

    memcpy(&cmd->info.host.adv_data[6], mnf, MNF_DATA_LEN);
    cmd->info.host.adv_data_len = USER_ADVERTISE_DATA_LEN;

    app_easy_gap_undirected_advertise_start();
    adv_timer_callback();
    is_advertising = 1;
}

void adv_svc_start_non_conn_adv(void) {
    if (is_advertising) return;

    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_non_connectable_advertise_get_active();

    if(cmd == NULL){
        return;
    }

    mnf_svc_update_battery_level();
    
    const uint8_t* mnf = mnf_svc_get_mnf_data();

    memcpy(&cmd->info.host.adv_data[6], mnf, MNF_DATA_LEN);
    cmd->info.host.adv_data_len = USER_ADVERTISE_DATA_LEN;

    app_easy_gap_non_connectable_advertise_start();
    adv_timer_callback();
    is_advertising = 1;
}


// SDK Callback methods

void user_app_adv_undirect_complete(uint8_t status)
{
    is_advertising = 0;

    if (status == GAP_ERR_CANCELED || status == GAP_ERR_TIMEOUT)
    {
        adv_svc_start_undirected_adv();
        return;
    }

    if(adv_mode == ADV_INIT_COMPLETED){
        adv_svc_start_non_conn_adv();
    }
}


void user_app_adv_nonconn_complete(uint8_t status)
{
    is_advertising = 0;

    if (status == GAP_ERR_CANCELED || status == GAP_ERR_TIMEOUT)
    {
        adv_svc_start_non_conn_adv();
        return;
    }

}