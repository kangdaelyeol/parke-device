#ifndef _MNF_DRIVER_H_
#define _MNF_DRIVER_H_

#include <stdint.h>

#define MNF_COMPANY_LEN 2
#define MNF_DEVICE_ID_LEN 8
#define MNF_BATTERY_LEVEL_LEN 1
#define MNF_DATA_LEN (MNF_COMPANY_LEN + MNF_DEVICE_ID_LEN + MNF_BATTERY_LEVEL_LEN)
#define NVDS_TAG_DEVICE_SERIAL    ((uint8_t)0x80)

void mnf_dv_init_mnf_data(void);
const uint8_t* mnf_dv_get_mnf_data(void)
void mnf_dv_update_device_id(uint8_t *device_id);
void mnf_dv_update_battery_level(uint8_t battery_level);


#endif // _MNF_DRIVER_H_