
#include <stdint.h>
#include <string.h>
#include "nvds.h"
#include "mnf_driver.h"

// Manufacturer data buffer
static uint8_t manufacturer_data[MNF_DATA_LEN] __SECTION_ZERO("retention_mem_area0");


// Public functions

void mnf_dv_init_mnf_data(void){
    manufacturer_data[0] = 0xFF; // Company ID LSB
    manufacturer_data[1] = 0xFF; // Company ID MSB
    memcpy(&manufacturer_data[MNF_COMPANY_LEN], "UNSET000", MNF_DEVICE_ID_LEN);
    manufacturer_data[MNF_COMPANY_LEN + MNF_DEVICE_ID_LEN] = 0x00; // Battery Level

    // NVDS에서 식별자 읽기
    uint8_t len = NVDS_SERIAL_LEN;
    uint8_t serial[NVDS_SERIAL_LEN];

    if (nvds_get(NVDS_TAG_DEVICE_SERIAL, &len, serial) == NVDS_OK)
    {
        // 저장된 식별자 있음 → 로드
        memcpy(&manufacturer_data[MNF_COMPANY_LEN], serial, NVDS_SERIAL_LEN);
    }
}

const uint8_t* mnf_dv_get_mnf_data(void)
{
    return manufacturer_data;
}

void mnf_dv_update_device_id(uint8_t *device_id)
{
    // g_manufacturer_data의 device ID 부분 업데이트
    memcpy(&manufacturer_data[MNF_COMPANY_LEN], device_id, MNF_DEVICE_ID_LEN);
}


void mnf_dv_update_battery_level(uint8_t battery_level)
{
    // g_manufacturer_data의 battery level 부분 업데이트
    manufacturer_data[MNF_COMPANY_LEN + MNF_DEVICE_ID_LEN] = battery_level;
}       
