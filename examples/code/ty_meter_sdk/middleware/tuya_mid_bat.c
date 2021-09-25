

#include "tuya_ble_log.h"
#include "ty_ble_bat.h"
#include "locdef_pin.h"
#include "driver_adc.h"


uint32_t ty_get_bat_vol(void)
{
    uint16_t result;
	uint16_t ref_vol;
	float vbat_vol;
	float vbat_final;
	

    //adc_enable(NULL, NULL, 0);
	adc_get_result(ADC_TRANS_SOURCE_PAD, 0x04, &result);
	//adc_disable();

	TUYA_APP_LOG_INFO("bat value==%u\r\n",result);

	ref_vol = adc_get_ref_voltage(ADC_REFERENCE_AVDD);

	vbat_vol = ((float)result * (float)ref_vol) / 1024;

	TUYA_APP_LOG_INFO("bat ref_vol==%u vbat_vol==%f\r\n",ref_vol,vbat_vol);
    //公式中，接合实际硬件电路，48V输入电压 ADC采集到的 分压值为 855mV
	vbat_final = 48*((float)result * (float)ref_vol) / 1024/(float)855;
	TUYA_APP_LOG_INFO("bat final==%f\r\n",vbat_final);
	return result;
}
