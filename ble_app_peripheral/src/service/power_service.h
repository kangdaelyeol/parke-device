#ifndef _POWER_SERVICE_H_
#define _POWER_SERVICE_H_

// defines

#include <stdint.h>
#define SLEEP_DURATION 500 
#define WAKEUP_DURATION 500

void power_svc_init(void);
void power_svc_start_sleep_wakeup_cycle(void);
void power_svc_go_to_sleep(void);
void power_svc_stop_cycle(void);

#endif // _POWER_SERVICE_H_