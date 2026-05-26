#ifndef _ADV_SERVICE_H_
#define _ADV_SERVICE_H_

#include <stdint.h>

typedef enum {
    ADV_DEFAULT,
    ADV_INIT_COMPLETED,
} adv_mode_t;

extern adv_mode_t adv_mode;

void adv_svc_init(void);
void adv_svc_stop_adv(adv_mode_t mode);
void adv_svc_start_undirected_adv(void);
void adv_svc_start_non_conn_adv(void);

#endif // _ADV_SERVICE_H_