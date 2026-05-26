#include <stdint.h>
#include <string.h>
#include "mnf_driver.h"
#include "battery.h"
#include "spi_flash.h"

// Manufacturer data buffer
static uint8_t manufacturer_data[MNF_DATA_LEN] __SECTION_ZERO("retention_mem_area0");


// Public functions

void mnf_dv_init_mnf_data(void){
    manufacturer_data[0] = 0xFF; // Company ID LSB
    manufacturer_data[1] = 0xFF; // Company ID MSB
    memcpy(&manufacturer_data[MNF_COMPANY_LEN], "UNSET000", MNF_DEVICE_ID_LEN);
    manufacturer_data[MNF_COMPANY_LEN + MNF_DEVICE_ID_LEN] = 0x00; // Battery Level

    uint8_t serial[MNF_DEVICE_ID_LEN];    
    uint32_t actual_size;
    spi_flash_read_data(serial, CUSTOM_DATA_FLASH_ADDRESS, MNF_DEVICE_ID_LEN, &actual_size);

    if (serial[0] != 0xFF)  // 빈 플래시는 0xFF
    {
        memcpy(&manufacturer_data[MNF_COMPANY_LEN], serial, MNF_DEVICE_ID_LEN);
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
    uint32_t actual_size;
    spi_flash_block_erase(CUSTOM_DATA_FLASH_ADDRESS, SPI_FLASH_OP_SE);
    spi_flash_write_data(device_id, CUSTOM_DATA_FLASH_ADDRESS, MNF_DEVICE_ID_LEN, &actual_size);
}


void mnf_dv_update_battery_level(void)
{
    uint8_t battery_level = battery_get_lvl(BATT_CR2032);
    // g_manufacturer_data의 battery level 부분 업데이트
    manufacturer_data[MNF_COMPANY_LEN + MNF_DEVICE_ID_LEN] = battery_level;
}       
