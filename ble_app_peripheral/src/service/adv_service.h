#ifndef _ADV_SERVICE_H_
#define _ADV_SERVICE_H_

#include <stdint.h>

void adv_svc_timer_init(void);
void adv_svc_start_undirected_adv(void);
void adv_svc_start_directed_adv(void);
void adv_svc_stop_adv(void);

#endif // _ADV_SERVICE_H_