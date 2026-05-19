#include <stdint.h>
#include "rwip_config.h"
#include "gattc_task.h"
#include "gap.h"
#include "app_easy_timer.h"
#include "user_config.h"
#include "adv_service.h"
#include "mnf_service.h"

uint8_t is_advertising __SECTION_ZERO("retention_mem_area0");

// Static methods

static int is_initialized(uint8_t *mnf) {
    if(memcmp(&mnf[MNF_COMPANY_LEN], "UNSET000", MNF_DEVICE_ID_LEN) == 0) {
        return 0; // Not initialized
    }
    return 1; // Initialized
}

static void adv_svc_start_undirected_adv(uint8_t* mnf) {
    if (is_advertising) return;
    
    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    
    if (cmd == NULL)
    {
        return;
    }

    mnf_svc_update_battery_level();

    memcpy(&cmd->info.host.adv_data[13], mnf, 11);
    cmd->info.host.adv_data_len = USER_ADVERTISE_DATA_LEN;

    app_easy_gap_undirected_advertise_start();
    is_advertising = 1;
}

static void adv_svc_start_non_conn_adv(uint8_t* mnf) {
    if (is_advertising) return;

    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_non_connectable_advertise_get_active();

    if(cmd == NULL){
        return;
    }

    memcpy(&cmd->info.host.adv_data[13], mnf, 11);
    cmd->info.host.adv_data_len = USER_ADVERTISE_DATA_LEN;

    app_easy_gap_non_connectable_advertise_start();
    is_advertising = 1;
}



// Public methods

void adv_svc_timer_init(void) {
    is_advertising = 0;
}

void adv_svc_stop_adv(void) {
    if (!is_advertising) return;

    app_easy_gap_advertise_stop();
    is_advertising = 0;
}

void adv_svc_start_adv(void){
    uint8_t* mnf = mnf_svc_get_mnf_data();

    if(is_initialized(mnf)) {
        adv_svc_start_non_conn_adv(mnf);
    } else {
        adv_svc_start_undirected_adv(mnf);
    }
}