#include <stdint.h>
#include "rwip_config.h"
#include "gattc_task.h"
#include "gap.h"
#include "app_easy_timer.h"
#include "user_config.h"
#include "adv_service.h"
#include "mnf_service.h"
#include "user_peripheral.h"

uint8_t is_advertising __SECTION_ZERO("retention_mem_area0");


// state machine variables
adv_mode_t adv_mode __SECTION_ZERO("retention_mem_area0");

// Public methods

void adv_svc_init(void) {
    is_advertising = 0;
    adv_mode = ADV_DEFAULT;
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

    const uint8_t* mnf = mnf_svc_get_mnf_data();

    mnf_svc_update_battery_level();

    memcpy(&cmd->info.host.adv_data[6], mnf, 11);
    cmd->info.host.adv_data_len = USER_ADVERTISE_DATA_LEN;

    app_easy_gap_undirected_advertise_start();
    is_advertising = 1;
}

void adv_svc_start_non_conn_adv(void) {
    if (is_advertising) return;

    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_non_connectable_advertise_get_active();

    if(cmd == NULL){
        return;
    }

    const uint8_t* mnf = mnf_svc_get_mnf_data();

    memcpy(&cmd->info.host.adv_data[6], mnf, 11);
    cmd->info.host.adv_data_len = USER_ADVERTISE_DATA_LEN;

    app_easy_gap_non_connectable_advertise_start();
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