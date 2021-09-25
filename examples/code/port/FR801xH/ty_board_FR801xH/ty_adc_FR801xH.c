#include "ty_adc.h"
#include "driver_adc.h"
#include "driver_system.h"
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
uint32_t ty_adc_init(ty_adc_t* p_adc)
{
    if((p_adc->channel)&0x01) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_ADC0);
    }
    
    if((p_adc->channel)&0x02) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_ADC1);
    }
    
    if((p_adc->channel)&0x04) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_ADC2);
    }
    
    if((p_adc->channel)&0x08) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_ADC3);
    }
    
    struct adc_cfg_t cfg;
    memset((void*)&cfg, 0, sizeof(cfg));
    cfg.src = ADC_TRANS_SOURCE_PAD;
    cfg.ref_sel = ADC_REFERENCE_AVDD;
    cfg.channels = 0x01;
    cfg.route.pad_to_sample = 1;
    cfg.clk_sel = ADC_SAMPLE_CLK_24M_DIV13;
    cfg.clk_div = 0x3f;
    adc_init(&cfg);
    
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_adc_start(ty_adc_t* p_adc)
{
    struct adc_cfg_t cfg;
    uint16_t result;
    adc_enable(NULL, NULL, 0);
    co_delay_100us(10);
    adc_get_result(ADC_TRANS_SOURCE_PAD, 0x01, &result);
    adc_disable();
    p_adc->value = result;
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_adc_stop(ty_adc_t* p_adc)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_adc_control(ty_adc_t* p_adc, uint8_t cmd, void* arg)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_adc_uninit(ty_adc_t* p_adc)
{
    if((p_adc->channel)&0x01) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_D4);
    }
    
    if((p_adc->channel)&0x02) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_D5);
    }
    
    if((p_adc->channel)&0x04) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_D6);
    }
    
    if((p_adc->channel)&0x08) {
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_D7);
    }
    
    return 0;
}

