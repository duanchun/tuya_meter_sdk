
#include "tuya_kel_flash.h"
#include "tuya_ble_port.h"

TUYA_RET_E tuya_kel_flash_init(void)
{
    return TUYA_OK;
}


TUYA_RET_E tuya_kel_flash_read(uint32_t addr, void* buf, uint32_t size)
{
    tuya_ble_device_enter_critical();
    flash_read(addr, size, (uint8_t*)buf);
    tuya_ble_device_exit_critical();
    return TUYA_OK;
}


TUYA_RET_E tuya_kel_flash_write(uint32_t addr, const void* buf, uint32_t size)
{
    tuya_ble_device_enter_critical();
    flash_write(addr, size, (uint8_t*)buf);
    tuya_ble_device_exit_critical();
    return TUYA_OK;
}


TUYA_RET_E ty_kel_flash_erase(uint32_t addr, uint32_t num)
{
    tuya_ble_device_enter_critical();
    flash_erase(addr, num*0x1000);
    tuya_ble_device_exit_critical();
    return TUYA_OK;
}


