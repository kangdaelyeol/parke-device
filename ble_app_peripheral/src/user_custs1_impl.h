#ifndef _USER_CUSTS1_IMPL_H_
#define _USER_CUSTS1_IMPL_H_

#include "gapc_task.h"
#include "gapm_task.h"
#include "custs1_task.h"

/**
 * @brief Serial value write indication handler.
 */
void user_svc1_serial_wr_ind_handler(ke_msg_id_t const msgid,
                                     struct custs1_val_write_ind const *param,
                                     ke_task_id_t const dest_id,
                                     ke_task_id_t const src_id);

/**
 * @brief Remaining attribute info request handler.
 */
void user_svc1_rest_att_info_req_handler(ke_msg_id_t const msgid,
                                         struct custs1_att_info_req const *param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id);

#endif // _USER_CUSTS1_IMPL_H_