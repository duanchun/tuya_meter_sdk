#include "ty_pin.h"
#include "ty_ble.h"
#include "ty_uart.h"
#include "ty_rtc.h"
#include "ty_pwm.h"
#include "ty_adc.h"
#include "ty_i2c.h"
#include "driver_system.h"
#include "driver_pmu.h"
#include "driver_gpio.h"
#include "button.h"




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
__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
    uint32_t gpio_value = ool_read32(PMU_REG_GPIOA_V);
    //button_toggle_detected(gpio_value);
    ool_write32(PMU_REG_PORTA_LAST, gpio_value);
}

/*********************************************************
FN: 
*/
uint32_t ty_pin_init(uint8_t pin, ty_pin_mode_t mode)
{
    //TODO
    //system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_D5);
    pmu_set_pin_to_PMU(GPIO_PORT_D, (1<<GPIO_BIT_5));
    pmu_set_pin_dir(GPIO_PORT_D, (1<<GPIO_BIT_5), GPIO_DIR_IN);
    pmu_set_pin_pull(GPIO_PORT_D, (1<<GPIO_BIT_5), true);
    pmu_port_wakeup_func_set(GPIO_PD5);
    //button_init(GPIO_PD5);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pin_set(uint8_t pin, ty_pin_level_t level)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pin_get(uint8_t pin, ty_pin_level_t* p_level)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pin_control(uint8_t pin, uint8_t cmd, void* arg)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pin_uninit(uint8_t pin, ty_pin_mode_t mode)
{
    return 0;
}

