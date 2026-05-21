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
#include "mnf_service.h"
#include "adv_service.h"
#include "lis3dh_driver.h"

#if defined (CFG_PRINTF_UART2)
#include "uart.h"
#include "uart_utils.h"
#endif

void user_svc1_serial_wr_ind_handler(ke_msg_id_t const msgid,
                                     struct custs1_val_write_ind const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id)
{
    if (param->length != 8) return;

    // 광고 데이터 업데이트
    mnf_svc_update_device_id((uint8_t*)param->value);
    app_easy_gap_disconnect(app_env[param->conidx].conidx);
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