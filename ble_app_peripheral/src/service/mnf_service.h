#ifndef _MNF_SERVICE_H_
#define _MNF_SERVICE_H_

#include <stdint.h>
#include "mnf_driver.h"

const uint8_t* mnf_svc_get_mnf_data(void);
void mnf_svc_update_device_id(uint8_t *device_id);
void mnf_svc_update_battery_level(void);
void mnf_svc_init(void);

#endif // _MNF_SERVICE_H_