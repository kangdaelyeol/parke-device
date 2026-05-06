#include <stdio.h>
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
#include "syscntl.h"
#include "co_bt.h"
#include "SEGGER_RTT.h"
#include "lis3dh.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define ACCEL_CHECK_INTERVAL    100   // 1초 (단위: 10ms)
#define ACCEL_MOTION_THRESHOLD  10    // 움직임 감지 임계값

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

    // 광고 데이터 초기화
    memcpy(stored_adv_data, USER_ADVERTISE_DATA, USER_ADVERTISE_DATA_LEN);
    stored_adv_data_len = USER_ADVERTISE_DATA_LEN;
    memcpy(stored_scan_rsp_data, USER_ADVERTISE_SCAN_RESPONSE_DATA, USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN);
    stored_scan_rsp_data_len = USER_ADVERTISE_SCAN_RESPONSE_DATA_LEN;

    default_app_on_init();

    SEGGER_RTT_printf(0, "GAP_ERR_NO_ERROR=%d, GAP_ERR_CANCELED=%d\r\n", 
                  GAP_ERR_NO_ERROR, GAP_ERR_CANCELED);

    // LIS3DH 초기화
    if (lis3dh_detect())
    {
        lis3dh_init();
        SEGGER_RTT_WriteString(0, "LIS3DH initialized!\r\n");
    }
    else
    {
        SEGGER_RTT_WriteString(0, "LIS3DH not detected!\r\n");
    }
}

void user_app_adv_start(void)
{
    SEGGER_RTT_WriteString(0, "adv_start called!\r\n");
    struct gapm_start_advertise_cmd* cmd;
    cmd = app_easy_gap_undirected_advertise_get_active();

    SEGGER_RTT_printf(0, "adv_data_len before: %d\r\n", cmd->info.host.adv_data_len);
    
    if (cmd == NULL)
    {
        SEGGER_RTT_WriteString(0, "ERROR: cmd is NULL!\r\n");
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


    SEGGER_RTT_printf(0, "adv_data_len after: %d\r\n", cmd->info.host.adv_data_len);
    

    app_easy_gap_undirected_advertise_start();
    is_advertising = 1;

    if (accel_check_timer == EASY_TIMER_INVALID_TIMER)
    {
        accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
    }
}



void user_app_connection(uint8_t connection_idx, struct gapc_connection_req_ind const *param)
{
    SEGGER_RTT_printf(0, "Connected! idx=%d\r\n", connection_idx);
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
    SEGGER_RTT_printf(0, "start default app_on_connection \r\n");
    default_app_on_connection(connection_idx, param);
    SEGGER_RTT_printf(0, "finished default app_on_connection \r\n");
}

void user_app_adv_undirect_complete(uint8_t status)
{
    is_advertising = 0;
    SEGGER_RTT_printf(0, "adv_complete status: %d\r\n", status);

    if (status == GAP_ERR_CANCELED)
    {
        SEGGER_RTT_WriteString(0, "Restarting adv...\r\n");
        user_app_adv_start();
    }
    else
    {
        SEGGER_RTT_printf(0, "Stopping adv, status=%d\r\n", status);
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

    SEGGER_RTT_WriteString(0, "Disconnected! Restarting advertising...\r\n");
    user_app_adv_start();
}

void user_update_manufacturer_data(uint8_t *serial)
{
    memcpy(&g_manufacturer_data[2], serial, 8);

    // 이름(7) + UUID AD(4) + 길이(1) + 타입(1) = 13
    memcpy(&stored_adv_data[13], g_manufacturer_data, 11);
    stored_adv_data_len = USER_ADVERTISE_DATA_LEN;

    SEGGER_RTT_printf(0, "Serial updated: %.8s\r\n", serial);
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
        SEGGER_RTT_WriteString(0, "Motion detected!\r\n");
        user_app_adv_start();
    }
    else
    {
        accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
    }
}

arch_main_loop_callback_ret_t user_on_ble_powered(void)
{
    SEGGER_RTT_WriteString(0, "BLE powered on!\r\n");
    if (!timer_started)
    {
        accel_check_timer = app_easy_timer(ACCEL_CHECK_INTERVAL, accel_check_timer_cb);
        timer_started = true;
    }
    return GOTO_SLEEP;
}

arch_main_loop_callback_ret_t user_on_system_powered(void)
{
    SEGGER_RTT_WriteString(0, "System powered on!\r\n");
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