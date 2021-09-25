#include "ty_flash.h"
#include "ty_system.h"
#include "driver_flash.h"
#include "tuya_ble_port_FR801xH.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
            
/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
uint32_t ty_flash_init(void)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_flash_read(uint32_t addr, uint8_t* buf, uint32_t size)
{
    tuya_ble_device_enter_critical();
    flash_read(addr, size, buf);
    tuya_ble_device_exit_critical();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_flash_write(uint32_t addr, const uint8_t* buf, uint32_t size)
{
    tuya_ble_device_enter_critical();
    flash_write(addr, size, (void*)buf);
    tuya_ble_device_exit_critical();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_flash_erase(uint32_t addr, uint32_t num)
{
    tuya_ble_device_enter_critical();
    flash_erase(addr, num*0x1000);
    tuya_ble_device_exit_critical();
    return 0;
}






