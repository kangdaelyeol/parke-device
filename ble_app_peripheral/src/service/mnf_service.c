#include <stdint.h>
#include "mnf_driver.h"
#include "mnf_service.h"
#include "battery.h"

const uint8_t* mnf_svc_get_mnf_data(void)
{
    return mnf_dv_get_mnf_data();
}

void mnf_svc_update_device_id(uint8_t *device_id)
{
    mnf_dv_update_device_id(device_id);
}

void mnf_svc_update_battery_level(void)
{
    uint8_t battery_level = battery_get_lvl(BATT_CR2032);
    mnf_dv_update_battery_level(battery_level);
}

void mnf_svc_init(void)
{
    mnf_dv_init_mnf_data();
}