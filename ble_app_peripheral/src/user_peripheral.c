// #include <stdio.h>
#include <stdint.h>
#include "rwip_config.h"
#include "gattc_task.h"
#include "gap.h"
#include "app_easy_timer.h"
#include "user_peripheral.h"
#include "user_custs1_impl.h"
#include "user_custs1_def.h"
#include "gpio.h"
#include "power_service.h"
#include "adv_service.h"
#include "mnf_service.h"

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
uint8_t app_connection_idx                      __SECTION_ZERO("retention_mem_area0");
timer_hnd app_adv_data_update_timer_used        __SECTION_ZERO("retention_mem_area0");
timer_hnd app_param_update_request_timer_used   __SECTION_ZERO("retention_mem_area0");


/*
 * Static functions
 ****************************************************************************************
 */

static void param_update_request_timer_cb(void)
{
    app_easy_gap_param_update_start(app_connection_idx);
    app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
}

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void user_app_init(void)
{
    app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
    power_svc_init();
    adv_svc_init();
    mnf_svc_init();
    default_app_on_init();
}

void user_app_adv_start(void) {
    if(mnf_svc_is_mnf_initialized()){
        power_svc_start_sleep_wakeup_cycle();
    } else {
        adv_svc_start_undirected_adv();
    }
}

void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    if (app_env[connection_idx].conidx != GAP_INVALID_CONIDX)
    {
        app_connection_idx = connection_idx;

        if ((param->con_interval < user_connection_param_conf.intv_min) ||
            (param->con_interval > user_connection_param_conf.intv_max) ||
            (param->con_latency != user_connection_param_conf.latency) ||
            (param->sup_to != user_connection_param_conf.time_out))
        {
            app_param_update_request_timer_used = app_easy_timer(APP_PARAM_UPDATE_REQUEST_TO, param_update_request_timer_cb);
        }
        default_app_on_connection(connection_idx, param);
    } else {
        adv_svc_start_undirected_adv();
    }

}

void user_app_disconnect(struct gapc_disconnect_ind const *param)
{
    if (app_param_update_request_timer_used != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(app_param_update_request_timer_used);
        app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
    }
    power_svc_init();
    adv_svc_init();
    mnf_svc_init();

    if(mnf_svc_is_mnf_initialized()){
        power_svc_start_sleep_wakeup_cycle();
    } else {
        adv_svc_start_undirected_adv();
    }
}

void user_catch_rest_hndl(ke_msg_id_t const msgid,
                          void const *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    switch(msgid)
    {
        case CUSTS1_VAL_WRITE_IND:
        {
            struct custs1_val_write_ind const *msg_param = (struct custs1_val_write_ind const *)(param);
            if (msg_param->handle == SVC1_IDX_SERIAL_VAL)
            {
                user_svc1_serial_wr_ind_handler(msgid, msg_param, dest_id, src_id);
            }
        } break;

        case CUSTS1_ATT_INFO_REQ:
        {
            struct custs1_att_info_req const *msg_param = (struct custs1_att_info_req const *)param;
            user_svc1_rest_att_info_req_handler(msgid, msg_param, dest_id, src_id);
        } break;

        case GATTC_EVENT_REQ_IND:
        {
            struct gattc_event_ind const *ind = (struct gattc_event_ind const *) param;
            struct gattc_event_cfm *cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
            cfm->handle = ind->handle;
            KE_MSG_SEND(cfm);
        } break;

        default:
            break;
    }
}
