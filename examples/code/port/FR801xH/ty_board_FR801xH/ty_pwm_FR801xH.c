#include "ty_pwm.h"
#include "driver_system.h"
#include "driver_pwm.h"




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
uint32_t ty_pwm_init(ty_pwm_t* p_pwm)
{
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_2, PORTD2_FUNC_PWM2);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pwm_start(ty_pwm_t* p_pwm)
{
    pwm_init(PWM_CHANNEL_2, p_pwm->freq, p_pwm->duty);
    pwm_start(PWM_CHANNEL_2);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pwm_stop(ty_pwm_t* p_pwm)
{
    pwm_stop(PWM_CHANNEL_2);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pwm_control(ty_pwm_t* p_pwm, uint8_t cmd, void* arg)
{
    pwm_update(PWM_CHANNEL_2, p_pwm->freq, p_pwm->duty);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_pwm_uninit(ty_pwm_t* p_pwm)
{
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_2, PORTD2_FUNC_D2);
    return 0;
}













