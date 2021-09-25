/*
 Copyright (c) 2021 Tuya Inc

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "vfm_base.h"
#include "vfm_vbat.h"
#include "locdef_pin.h"
#include "locdef_service.h"
#include "locdef_config.h"
#include "driver_adc.h"//TODO
#include "tuya_log.h"


#define MAIN_POWER_SAMPLE_WIN_SIZE          9                   //主电源AD采样窗口长度
#define LOCDEF_VADC_CTR_EN_PIN          (18) /* 主电源电池采样使能引脚 */
#define LOCDEF_5V_EN_CTRL_PIN           (12) /* 5v电源使能控制引脚 */

static int device_vbat_read_rsoc(VBAT_TYPE_E type,uint8_t *rsoc);
static int device_vbt_read_voltage(VBAT_TYPE_E type,int *mV);

static uint16_t locdef_get_bat_vol(void);

static tyVFMVbatIntf_t m_intf = {
    .read_rsoc = device_vbat_read_rsoc,
    .read_voltage = device_vbt_read_voltage,
    .read_current = NULL,
    .read_temperature = NULL,
};




/**
 * @brief   adc获取电量
 *
 * @param   void
 *
 * @return  无
 * @note    ymax 要映射的目标区间最大值
 * @note    ymin 要映射的目标区间最小值
 * @note    xmax 目前数据最大值
 * @note    xmin 目前数据最小值
 * @note    x 为目前数据中的任一值
 * @note    y 为映射后的百分比值   转化公式：y=ymin+((ymax−ymin)/(xmax−xmin))*(x−xmin)
 */
static void locdef_get_main_power_battery(uint8_t *bat_per, uint16_t * voltage)
{
    static uint16_t log_cnt = 0;
    float voltage_tmp = 0;
    float offset = 0.0;
    float battery_per = 0;
    static float vbat_buf[MAIN_POWER_SAMPLE_WIN_SIZE] = {0};
    static uint16_t sample_index = 0;

    tuya_dev_cfg_t *cfg_param = locdef_get_dev_cfg_param();
    tuya_pin_write(EBIKE_BAT_48V_ADC_EN_GPIO, TUYA_PIN_HIGH);

    tuya_kel_system_delay_ms(50);
	voltage_tmp = locdef_get_bat_vol();
    offset = cfg_param->voltage_upper_limit - cfg_param->voltage_lower_limit;
    if (voltage_tmp < cfg_param->voltage_lower_limit) {
        battery_per = 0;
    } else {
        battery_per = (float)(100*((float)voltage_tmp - cfg_param->voltage_lower_limit)/offset);
    }
	TUYA_LOG_I(MOD_BATTERY, "upper_limit==%f,lower_limit==%f",cfg_param->voltage_upper_limit,cfg_param->voltage_lower_limit);

    /* 滑动平均滤波处理 */
    *bat_per = smoothing_filter_f(vbat_buf, MAIN_POWER_SAMPLE_WIN_SIZE,
                                  &sample_index, battery_per);
    TUYA_LOG_I(MOD_BATTERY, "get_main_power filter value: %d, bat_per: %.3f", *bat_per, battery_per);

    *voltage = voltage_tmp*10;//v
    tuya_pin_write(EBIKE_BAT_48V_ADC_EN_GPIO, TUYA_PIN_LOW);

    //if (log_cnt % 10 == 0) {
        TUYA_LOG_I(MOD_BATTERY, "get_main_power voltage:%.3f, voltage:%d, battery_per:%d",
                    voltage_tmp, *voltage,*bat_per);
    //}
    log_cnt++;
}



/**
 * @brief   主电源掉电回调处理函数
 *
 * @param
 *
 * @return  void
 */
static void locdef_power_off_irq_cb(void *args)
{
#if 0
    tuya_pin_name_t pin = LOCDEF_PWR_OFF_PIN;
    locdef_io_filter_notify(pin,0);
    tuya_pin_irq_disable(LOCDEF_PWR_OFF_PIN);
#endif
}


static int device_vbat_read_rsoc(VBAT_TYPE_E type,uint8_t *rsoc)
{
    uint16_t mV = 0;

    if (rsoc == NULL) {
        return -1;
    }
    if (type == VBAT_INTERNAL) {
		//TODO
        //return tuya_cellular_battery_get_rsoc(rsoc);
        return 0;
    }
    else {
        locdef_get_main_power_battery(rsoc,&mV);
    }
    return 0;
}

static int device_vbt_read_voltage(VBAT_TYPE_E type,int *mV)
{
    uint8_t rsoc = 0;
    uint16_t voltage = 0;

    if (mV == NULL) {
        return -1;
    }
    if (type == VBAT_INTERNAL) {
        return -2;//OPRT_NOT_SUPPORTED
    }
    else {
        locdef_get_main_power_battery(&rsoc,&voltage);
        *mV = voltage;
    }
    return 0;
}

uint8_t locdef_get_vat_percent(void)
{
    uint8_t rsoc;
	uint16_t mV = 0;

	locdef_get_main_power_battery(&rsoc,&mV);

    return rsoc;
}

static uint16_t locdef_get_bat_vol(void)
{
    uint16_t result;
	uint16_t ref_vol;
	float vbat_vol;
	float vbat_final;
	

    //adc_enable(NULL, NULL, 0);
	adc_get_result(ADC_TRANS_SOURCE_PAD, 0x04, &result);
	//adc_disable();

	TUYA_LOG_I(MOD_BATTERY, "bat value==%u\r\n",result);

	ref_vol = adc_get_ref_voltage(ADC_REFERENCE_AVDD);

	vbat_vol = ((float)result * (float)ref_vol) / 1024;

	TUYA_LOG_I(MOD_BATTERY, "bat ref_vol==%u vbat_vol==%f\r\n",ref_vol,vbat_vol);
    //公式中，接合实际硬件电路，48V输入电压 ADC采集到的 分压值为 855mV
	vbat_final = 48*((float)result * (float)ref_vol) / 1024/(float)855;
	TUYA_LOG_I(MOD_BATTERY, "bat final==%f\r\n",vbat_final);
	return vbat_final;
}


static void locdef_bat_init(void)
{
	struct adc_cfg_t cfg;
    uint16_t result;
    system_set_port_mux(EBIKE_BAT_48V_ADC_PORT, EBIKE_BAT_48V_ADC_GPIO_BIT, PORTD6_FUNC_ADC2);
    
    memset((void*)&cfg, 0, sizeof(cfg));
    cfg.src = ADC_TRANS_SOURCE_PAD;
    cfg.ref_sel = ADC_REFERENCE_AVDD;
    cfg.channels = 0x04;
    cfg.route.pad_to_sample = 1;
    cfg.clk_sel = ADC_SAMPLE_CLK_24M_DIV13;
    cfg.clk_div = 0x3f;
    adc_init(&cfg);
	adc_enable(NULL, NULL, 0);
}


int device_vbat_register(void)
{
    int op_ret = 0;
	#if 0
    /* 主电源掉电检测引脚配置，边沿触发中断 */
    op_ret = tuya_pin_irq_init(LOCDEF_PWR_OFF_PIN, TUYA_PIN_IN_IRQ | TUYA_PIN_IRQ_RISE_FALL,
                               locdef_power_off_irq_cb, NULL);
    if (op_ret == 0) {
        tuya_pin_irq_enable(LOCDEF_PWR_OFF_PIN);
    } else {
        return op_ret;
    }
	#endif
    /* 主电源电池采样使能，高有效 */
    //48V电池采样使能控制,输出,高有效
    TUYA_GPIO_INIT(EBIKE_BAT_48V_ADC_EN,GPIO_DIR_OUT,1);

    #if 0
    /* 5V电源使能，高有效 */
    op_ret = tuya_pin_init(LOCDEF_5V_EN_CTRL_PIN, TUYA_PIN_MODE_OUT_PP_HIGH);
    if (op_ret != 0) {
        return op_ret;
    } else {
        locdef_charge_enable(TRUE);
    }
	#endif

    /* 初始化主电源电池电压采样ADC */
	locdef_bat_init();
	
    op_ret =  ty_vfm_vbat_reg(&m_intf);
    return op_ret;
}
