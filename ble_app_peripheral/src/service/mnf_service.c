#include <stdint.h>
#include "mnf_driver.h"
#include "mnf_service.h"
#include "battery.h"

const uint8_t* mnf_svc_get_mnf_data(void)
{
    return (uint8_t*)mnf_dv_get_mnf_data();
}

void mnf_svc_update_device_id(uint8_t *device_id)
{
    mnf_dv_update_device_id(device_id);
}

void mnf_svc_update_battery_level(void)
{
    uint8_t battery_level = battery_get_lvl(BATT_ALKALINE);
    mnf_dv_update_battery_level(battery_level);
}

void mnf_svc_init(void)
{ 
    mnf_dv_init_mnf_data();
}

int mnf_svc_is_mnf_initialized(void) {
    uint8_t* mnf = (uint8_t*)mnf_dv_get_mnf_data();
    
    if(memcmp(&mnf[MNF_COMPANY_LEN], "UNSET000", MNF_DEVICE_ID_LEN) == 0) {
        return 0; // Not initialized
    }
    return 1; // Initialized
}
