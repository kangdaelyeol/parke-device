#include "gpio.h"
#include "app_api.h"
#include "app.h"
#include "prf_utils.h"
#include "custs1.h"
#include "custs1_task.h"
#include "user_custs1_def.h"
#include "user_custs1_impl.h"
#include "user_peripheral.h"
#include "user_periph_setup.h"
#include "SEGGER_RTT.h"

void user_svc1_serial_wr_ind_handler(ke_msg_id_t const msgid,
                                     struct custs1_val_write_ind const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    if (param->length != 8) return;

    // 받은 8바이트 serial 저장
    char serial[9] = {0};
    memcpy(serial, param->value, 8);

    SEGGER_RTT_printf(0, "Serial received: %s\r\n", serial);

    // 광고 데이터 업데이트
    user_update_manufacturer_data((uint8_t*)serial);
}

void user_svc1_rest_att_info_req_handler(ke_msg_id_t const msgid,
                                         struct custs1_att_info_req const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    struct custs1_att_info_rsp *rsp = KE_MSG_ALLOC(CUSTS1_ATT_INFO_RSP,
                                                   src_id,
                                                   dest_id,
                                                   custs1_att_info_rsp);
    rsp->conidx  = app_env[param->conidx].conidx;
    rsp->att_idx = param->att_idx;
    rsp->length  = 0;
    rsp->status  = ATT_ERR_WRITE_NOT_PERMITTED;

    KE_MSG_SEND(rsp);
}