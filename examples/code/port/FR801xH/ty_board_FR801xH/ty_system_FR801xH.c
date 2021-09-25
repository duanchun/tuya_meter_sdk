#include "ty_system.h"
#include "ty_ble.h"
#include "ty_uart.h"
#include "ty_flash.h"
#include "ty_timer.h"
#include "ty_rtc.h"
#include "ty_pwm.h"
#include "ty_adc.h"
#include "ty_pin.h"
#include "ty_i2c.h"
#include "ty_spi.h"
#include "tuya_ble_api.h"
#include "tuya_ble_log.h"
//#include "tuya_ble_sdk_test.h"
#include "driver_wdt.h"
#include "sys_utils.h"




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
uint32_t ty_system_init(uint8_t location)
{
    switch(location)
    {
        case 0: {
            ty_uart_init();
            ty_uart2_init();
            ty_system_log_init();
            //peripheral init
            ty_flash_init();
            ty_rtc_init();
        } break;
        
        case 1: {
            ty_ble_set_tx_power(RF_TX_POWER_0dBm);
        } break;
        
        case 2: {
        } break;
        
        default: {
        } break;
    }
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_mainloop(void)
{
    tuya_ble_main_tasks_exec();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_reset(void)
{
    platform_reset_patch(0);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_wdt_init(void)
{
//    wdt_init(WDT_ACT_RST_CHIP, 4);
//    wdt_start();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_wdt_uninit(void)
{
//    wdt_stop();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_wdt_feed(void)
{
//    wdt_feed();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_log_init(void)
{
#if (TY_LOG_ENABLE||TUYA_BLE_LOG_ENABLE||TUYA_APP_LOG_ENABLE)
    elog_init();
//    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_LVL);
    elog_start();
#endif
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_log_hexdump(const char *name, uint8_t width, uint8_t *buf, uint16_t size)
{
#if (TY_LOG_ENABLE||TUYA_BLE_LOG_ENABLE||TUYA_APP_LOG_ENABLE)
    elog_hexdump(name, width, buf, size);
#endif
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_delay_ms(uint32_t ms)
{
    co_delay_100us(ms*10);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_delay_us(uint32_t us)
{
    if(us<10) {
        us = 1;
    } else {
        us = us/10;
    }
    co_delay_10us(us);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_enter_sleep(void)
{
    //ty_mainloop_timer_stop(); TODO
    system_sleep_enable();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_system_exit_sleep(void)
{
    return 0;
}

/*********************************************************
FN: 
*/
bool ty_system_is_sleep(void)
{
    return 0;
}






