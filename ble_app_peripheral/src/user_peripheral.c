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
#include "i2c.h"
#include "lis3dh.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define ACCEL_CHECK_INTERVAL    100   // 1초 (단위: 10ms)
#define ACCEL_MOTION_THRESHOLD  10    // 움직임 감지 임계값

// NVDS TAG 정의 (사용자 정의 TAG)
#define NVDS_TAG_DEVICE_SERIAL    ((uint8_t)0x80)
#define NVDS_SERIAL_LEN           8

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
uint8_t app_connection_idx                      __SECTION_ZERO("retention_mem_area0");
timer_hnd app_adv_data_update_timer_used        __SECTION_ZERO("retention_mem_area0");
timer_hnd app_param_update_request_timer_used   __SECTION_ZERO("retention_mem_area0");
timer_hnd accel_check_timer                     __SECTION_ZERO("retention_mem_area0");
uint8_t is_advertising                          __SECTION_ZERO("retention_mem_area0");
uint8_t stored_adv_data_len                     __SECTION_ZERO("retention_mem_area0");
uint8_t stored_scan_rsp_data_len                __SECTION_ZERO("retention_mem_area0");
uint8_t stored_adv_data[ADV_DATA_LEN]           __SECTION_ZERO("retention_mem_area0");
uint8_t stored_scan_rsp_data[SCAN_RSP_DATA_LEN] __SECTION_ZERO("retention_mem_area0");
static bool timer_started                       __SECTION_ZERO("retention_mem_area0");



// Manufacturer data buffer
static uint8_t g_manufacturer_data[11] = {
    0xFF, 0xFF,                           // Company ID
    'U','N','S','E','T','0','0','0',      // Device ID (초기값)
    0x00                                  // Battery Level
};

/*
 * FORWARD DECLARATIONS
 ****************************************************************************************
 */


static void accel_check_timer_cb(void);
static void param_update_request_timer_cb(void);

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void user_app_init(void)
{
    app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
    accel_check_timer = EASY_TIMER_INVALID_TIMER;
    is_advertising = 0;
    timer_started = false;  

    // NVDS에서 식별자 읽기
    uint8_t len = NVDS_SERIAL_LEN;
    uint8_t serial[NVDS_SERIAL_LEN];

    if (nvds_get(NVDS_TAG_DEVICE_SERIAL, &len, serial) == NVDS_OK)
    {
        // 저장된 식별자 있음 → 로드
        memcpy(&g_manufacturer_data[2], serial, NVDS_SERIAL_LEN);
    }
    else
    {
        // 저장된 식별자 없음 → UNSET000 유지
    }

    // 광고 데이터 초기화
    memcpy(stored_adv_data, USER_ADVERTISE_DATA, USER_ADVERTISE_DATA_LEN);
    stored_adv_data_len = USER_ADVERTISE_DATA_LEN;
    memcpy(stored_scan_rsp_data, USER_ADVERTISE_SCAN_RESPONSE_DATA, USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN);
    stored_scan_rsp_data_len = USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN;
    
    // 광고 데이터에 manufacturer data 반영
    memcpy(&stored_adv_data[13], g_manufacturer_data, 11);

    default_app_on_init();
}

void user_app_adv_start(void)
{
    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    
    if (cmd == NULL)
    {
        return;
    }
    // SDK가 자동으로 이름 등을 추가한 후
    // 우리 광고 데이터를 추가
    // 기존 데이터 무시하고 우리 데이터로 완전히 덮어쓰기
    memcpy(cmd->info.host.adv_data, stored_adv_data, stored_adv_data_len);
    cmd->info.host.adv_data_len = stored_adv_data_len;

    // scan response도 설정
    memcpy(cmd->info.host.scan_rsp_data, stored_scan_rsp_data, stored_scan_rsp_data_len);
    cmd->info.host.scan_rsp_data_len = stored_scan_rsp_data_len;


    

    app_easy_gap_undirected_advertise_start();
    is_advertising = 1;

    if (accel_check_timer == EASY_TIMER_INVALID_TIMER)
    {
        accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
    }
}



void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    if (app_env[connection_idx].conidx != GAP_INVALID_CONIDX)
    {
        app_connection_idx = connection_idx;
        is_advertising = 0;

        if ((param->con_interval < user_connection_param_conf.intv_min) ||
            (param->con_interval > user_connection_param_conf.intv_max) ||
            (param->con_latency != user_connection_param_conf.latency) ||
            (param->sup_to != user_connection_param_conf.time_out))
        {
            app_param_update_request_timer_used = app_easy_timer(APP_PARAM_UPDATE_REQUEST_TO, param_update_request_timer_cb);
        }
    }
    else
    {
        user_app_adv_start();
    }
    default_app_on_connection(connection_idx, param);
}

void user_app_adv_undirect_complete(uint8_t status)
{
    is_advertising = 0;

    if (status == GAP_ERR_CANCELED)
    {
        user_app_adv_start();
    }
    else
    {
        if (accel_check_timer == EASY_TIMER_INVALID_TIMER)
        {
            accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
        }
    }
}

void user_app_disconnect(struct gapc_disconnect_ind const *param)
{
    if (app_param_update_request_timer_used != EASY_TIMER_INVALID_TIMER)
    {
        app_easy_timer_cancel(app_param_update_request_timer_used);
        app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
    }

    user_app_adv_start();
}

void user_update_manufacturer_data(uint8_t *serial)
{
    // g_manufacturer_data 업데이트
    memcpy(&g_manufacturer_data[2], serial, 8);

    // NVDS에 영구 저장
    if (nvds_put(NVDS_TAG_DEVICE_SERIAL, NVDS_SERIAL_LEN, serial) == NVDS_OK)
    {
    }
    else
    {
        uint8_t result = nvds_put(NVDS_TAG_DEVICE_SERIAL, NVDS_SERIAL_LEN, serial);
    }

    // 광고 데이터 업데이트
    memcpy(&stored_adv_data[13], g_manufacturer_data, 11);
    stored_adv_data_len = USER_ADVERTISE_DATA_LEN;
}

static void accel_check_timer_cb(void)
{
    accel_check_timer = EASY_TIMER_INVALID_TIMER;

    if (is_advertising)
    {
        accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
        return;
    }

    if (lis3dh_motion_detected(ACCEL_MOTION_THRESHOLD))
    {
        user_app_adv_start();
    }
    else
    {
        accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
    }
}

arch_main_loop_callback_ret_t user_on_system_powered(void)
{
     if (!timer_started)
    {
        accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
        timer_started = true;
    }
    return GOTO_SLEEP;
}

sleep_mode_t user_app_validate_sleep(sleep_mode_t sleep_mode)
{
    return mode_active;
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

static void param_update_request_timer_cb(void)
{
    app_easy_gap_param_update_start(app_connection_idx);
    app_param_update_request_timer_used = EASY_TIMER_INVALID_TIMER;
}