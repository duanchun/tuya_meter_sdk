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
#include "locdef_pin.h"//TODO

#include "vfm_sensor.h"
#include "vfm_fsm.h"
#include "vfm_upload.h"
#include "locdef_service.h"
#include "locdef_audio.h"
#include "locdef_config.h"
#include "locdef_dp.h"
#include "driver_exti.h"//TODO
#include "tuya_kel_timer.h"
#include "driver_timer.h"//TODO
#include "tuya_log.h"


#define ROTATION_HARD_TIMER_ID  TIMER0

#define PI                          3.141592653f
#define ROTATION_PULSE_VALID_THR    (4)
#define SPEED_SAMPLE_WIN_SIZE               7                   //速度采样窗口长度

//static os_timer_t g_pulse_cnt_timer = {0};
static uint8_t          g_hard_timer_en = false;
static uint32_t           g_pulse_cnt = 0;


static int device_sensor_acc_ctrl(ACC_POWER_E action);
static int device_sensor_acc_read(ACC_POWER_E *type);
static int device_sensor_read_speed(uint32_t *speed);
static int device_sensor_read_mileage(float *mileage);
static tyVFMSensorIntf_t m_intf = {
    .ctrl_acc =                                 device_sensor_acc_ctrl,
    .read_acc =                                 device_sensor_acc_read,
    .read_speed =                               device_sensor_read_speed,
    .read_mileage =                             device_sensor_read_mileage,
    .read_total_mileage =                       NULL,
    .read_error =                               NULL,
};


static int device_sensor_acc_ctrl(ACC_POWER_E action)
{
    int ret = 0;
    if (action == ACC_POWER_ON) {
        ret = tuya_pin_write(EBIKE_ELECTRIC_DOOR_LOCK_CTRL_GPIO, TUYA_PIN_HIGH);
    }
    else {
        ret = tuya_pin_write(EBIKE_ELECTRIC_DOOR_LOCK_CTRL_GPIO, TUYA_PIN_LOW);
    }

    return ret;
}

static int device_sensor_acc_read(ACC_POWER_E *type)
{
    int ret = 0;
	if (TUYA_PIN_HIGH == tuya_pin_read_no_pmu(EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO)) {
        *type = ACC_POWER_ON;
    } else {
        *type = ACC_POWER_OFF;
    }

    return ret;
}

static int device_sensor_read_mileage(float *mileage)
{
    *mileage = locdef_get_single_mileage();
    return 0;
}


static int device_sensor_read_speed(uint32_t *speed)
{
    *speed = locdef_get_speed();
    return 0;
}


/**
 * @brief   超速检测回调处理函数
 *
 * @param
 *
 * @return  void
 */
void locdef_speeding_irq_cb(void *args)
{
    //TUYA_LOG_I(MOD_DEV, " locdef_speeding_irq_cb");
	tuya_pin_irq_disable(EBIKE_SPEED_ALARM_GPIO);
    locdef_io_filter_notify(EBIKE_SPEED_ALARM_GPIO, 0);
}


/**
 * @brief   一键启动回调处理函数
 *
 * @param
 *
 * @return  void
 */
void locdef_onekey_start_irq_cb(void *args)
{
	tuya_pin_irq_disable(EBIKE_START_GPIO);
    locdef_io_filter_notify(EBIKE_START_GPIO, 0);

    //TUYA_LOG_I(MOD_DEV, " locdef_onekey_start_irq_cb");
}

/**
 * @brief   振动检测回调处理函数
 *
 * @param
 *
 * @return  void
 */
void locdef_vib_sensor_irq_cb(void *args)
{
    static uint32_t first_trig_tick = 0;
    static uint32_t count = 0;
    TONE_TYPE_E cur_tone = locdef_get_cur_play_tone(); 

    uint32_t cur_tick = tuya_kel_sys_get_curr_time();

    if(TUYA_PIN_HIGH == tuya_pin_read_no_pmu(EBIKE_VIBRATE_DETECT_GPIO))
	{
		ext_int_set_type(EXTI_5, EXT_INT_TYPE_NEG);
	}else{
		ext_int_set_type(EXTI_5, EXT_INT_TYPE_POS);
	}
   
    /* 记录首次触发系统时基,或超过3倍时间窗口 */
    if (first_trig_tick == 0 || ((cur_tick - first_trig_tick) >= (VIB_DET_TIME_WINDOW*3))) {
        first_trig_tick = cur_tick;
        count = 0;
    }

    /* 判断500ms内脉冲次数 */
    if ((cur_tick - first_trig_tick) >= VIB_DET_TIME_WINDOW) {
        if (count >= VIB_SENSOR_SENSITIVITY) {
            if (cur_tone != TONE_ALARM) {
				TUYA_LOG_I(MOD_DEV, " irq vib trigger count==%d", count);
                TUYA_LOG_I(MOD_DEV, " irq vib trigger event time:%lld", cur_tick - first_trig_tick);
                ty_vfm_sensor_event_post(SENSOR_EVENT_SHAKE);
            } else {
                TUYA_LOG_I(MOD_DEV, " irq vib trigger event audio busy");
            }
        } else {
            TUYA_LOG_I(MOD_DEV, " irq vib trigger count %d", count);
        }

        first_trig_tick = 0;
        count = 0;
    }

    count +=1;

    if (count % 100 == 0) {
        //TUYA_APP_LOG_INFO(" irq vib sensor cnt: %d time:%lld",
                         //count, cur_tick - first_trig_tick);
    }
}

/**
 * @brief   轮动检测回调处理函数
 *
 * @param
 *
 * @return  void
 * @note 轮动测速思路
 *  1.开启500ms的周期定时器,每个周期T开启轮动中断检测使能;
 *  2.每个周期T同时开启一个us级硬件定时器，采集t时间窗口;
 *  3.t时间窗口结束后，计算触发的轮动中断个数，从而算出转速。
 *
 */
void locdef_rotation_det_irq_cb(void *args)
{

    if (!g_hard_timer_en) {
        g_pulse_cnt = 0;
        g_hard_timer_en = true;
        /* 启动500ms的脉冲计数时间窗口 */
        //os_timer_start(&g_pulse_cnt_timer, ROTATION_DET_PERIOD,false);
        timer_run(ROTATION_HARD_TIMER_ID);
		//TUYA_LOG_I(MOD_DEV, "timer run:timer_0");
    }

	#if 0
	if(TUYA_PIN_HIGH == tuya_pin_read_no_pmu(EBIKE_ROTATION_ALARM_GPIO))
	{
		ext_int_set_type(EXTI_7, EXT_INT_TYPE_NEG);
	}else{
		ext_int_set_type(EXTI_7, EXT_INT_TYPE_POS);
	}
	#endif
    g_pulse_cnt++;

    //TUYA_LOG_I(MOD_DEV, " rotation irq pulse_cnt:%d", g_pulse_cnt);

}


/**
 * @brief   脉冲计数硬件定时器
 *
 * @param
 *
 * @return  void
 */
static void pulse_cnt_timer_cb(void *param)
{
    static float speed_buf[SPEED_SAMPLE_WIN_SIZE] = {0};
    static uint16_t filter_index = 0;
    float variance = 0;
    float speed_m_s = 0;
    float speed_km_h = 0;
    float filter_speed = 0;
    float rotat_nums = 0;
    float wheel_diameters = 0;
    DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();
    tuya_dev_cfg_t * dev_cfg_parm = locdef_get_dev_cfg_param();

    g_hard_timer_en = false;
    run_param->rotation_irq_en = false;
    tuya_pin_irq_disable(EBIKE_ROTATION_ALARM_GPIO);
    timer_stop(ROTATION_HARD_TIMER_ID);
    //TUYA_LOG_I(MOD_DEV,  " rotation_irq disable g_pulse_cnt==%u",g_pulse_cnt);

    uint32_t now_state;
    ty_vfm_fsm_get_now_state(&now_state, NULL);

    /* 需要对脉冲进行一个过滤，避免打开电门时出现少量脉冲干扰 */
    if (g_pulse_cnt > ROTATION_PULSE_VALID_THR) {
        rotat_nums = (float)g_pulse_cnt / ROUND_PULSE_NUMS;
        /* 轮径英寸转米 */
        wheel_diameters = (float)dev_cfg_parm->wheel * 0.0254; 

        /* v = s/t, s=旋转圈数*圆周长, t = 500ms */
        speed_m_s = (rotat_nums*PI*wheel_diameters) / (ROTATION_DET_PERIOD/1000.0f);
        /* 换算成KM/H */
        speed_km_h = speed_m_s * 3.6;

        /* 滑动平均滤波处理,保留一位小数 */
        filter_speed = smoothing_filter_f(speed_buf, SPEED_SAMPLE_WIN_SIZE,
                                          &filter_index, speed_km_h);

        /* 当方差较小时,速度恒定赋值平滑滤波后的值 */
        variance_calc(speed_buf, SPEED_SAMPLE_WIN_SIZE, &variance);
        if (variance <= 1) {
            run_param->speed = filter_speed*10;
            /* 单次里程计算,单位KM */
            run_param->single_mileage += (filter_speed * 1.0f/3600);
        } else {
            run_param->speed = speed_km_h*10;
            /* 单次里程计算,单位KM */
            run_param->single_mileage += (speed_km_h * 1.0f/3600);
        }

        /* 轮动达到2KM/H的速度时,才触发轮动报警 */
        if (speed_km_h > 2.0) {
            run_param->rotation_alarm_cnt = 0;
			TUYA_LOG_I(MOD_DEV, "speed_km_h==%f",speed_km_h);
            ty_vfm_sensor_event_post(SENSOR_EVENT_ROTATION);
        }

        run_param->rotation_det_cplt = true;

    }
    /* 启动防盗后,统计轮动脉冲数量 */
    else if (FSM_ACCOFF_DEF == now_state) {
        run_param->rotation_alarm_cnt += g_pulse_cnt;
        if (run_param->rotation_alarm_cnt > LOCDEF_ROTATION_ALARM_PULSE_CNT) {
            run_param->rotation_alarm_cnt = 0;
			TUYA_LOG_I(MOD_DEV, "g_pulse_cnt==%u",g_pulse_cnt);
            ty_vfm_sensor_event_post(SENSOR_EVENT_ROTATION);
        }
        run_param->rotation_det_cplt = true;
    }

	#if 0
    TUYA_LOG_I(MOD_DEV, "ratation pulse_cnt[%d], rotat_nums[%0.2f], alarm_cnt:[%d]",
                     g_pulse_cnt, rotat_nums, run_param->rotation_alarm_cnt);

    TUYA_LOG_I(MOD_DEV, "now_speed:[%0.2f], filter_speed:[%0.2f], report_speed:[%d], variance:[%0.5f]",
                     speed_km_h, filter_speed, run_param->speed, variance);

    TUYA_LOG_I(MOD_DEV, "single mileage [%0.3f]KM", run_param->single_mileage);
	#endif
}

__attribute__((section("ram_code"))) void timer0_isr_ram(void)
{
    timer_clear_interrupt(ROTATION_HARD_TIMER_ID);
	//VFM_INFO(VFM_SENSOR_LOG,"timer0_isr_ram\r\n");
	pulse_cnt_timer_cb(NULL);
}


/**
 * @brief   冒泡排序
 *
 * @param
 *
 * @return  无
 */
void bubble_sort_f(float *data, uint32_t len)
{
    uint32_t i,j;
    float temp = 0;

    if (data == NULL || len == 0) {
        return;
    }
    for (i = 0; i < len-1; i++) {
        for (j = 0 ; j < len-i-1; j++) {
            if (data[j] > data[j+1]) {
                temp = data[j];
                data[j] = data[j+1];
                data[j+1] = temp;
            }
        }
    }
}


/**
 * @brief   滑动平均滤波
 *
 * @param   n至少为3以上
 *
 * @return  无
 */
float smoothing_filter_f(float value_buf[], uint16_t n, uint16_t *filter_index, float now_value)
{
    float sum = 0;
    uint16_t i = 0;

    if (*filter_index >= n) {
        return 0;
    }

    value_buf[*filter_index] = now_value;
    (*filter_index)++;

    if (*filter_index == n) {
        *filter_index = 0;
    }

    bubble_sort_f(value_buf, n);

    for (i = 1; i < n-1; i++) {
        sum += value_buf[i];
    }

    return (sum/(n-2));
}

/**
 * @brief   方差计算
 *
 * @param
 *
 * @return  无
 */
int variance_calc(const float value_buf[], uint16_t n, float *variance)
{
    uint16_t i = 0;
    double sum = 0, avg = 0;

    int op_ret = 0;

    if (value_buf == NULL || n == 0 || variance == NULL) {
        return -1;//OPRT_INVALID_PARM;
    }

    for (i = 0; i < n; i++) {
        sum += value_buf[i];
    }

    avg = sum / n;

    sum = 0;
    for (i = 0; i < n; i++) {
        sum += (value_buf[i]-avg)*(value_buf[i]-avg);
    }

    /* 标准差 */
    //*variance = sqrt(sum/n);

    /* 方差 */
    *variance = (sum/n);

    return op_ret;
}


/**
 * @brief   轮动、振动、超速、一键启动，GPIO引脚初始化
 *
 * @return
 *
 * @note
 */
static int device_sensor_gpio_extern_det_init(void)
{
    int op_ret = 0;

    /* 电门锁（ACC）控制引脚配置，高有效 */
    TUYA_GPIO_INIT(EBIKE_ELECTRIC_DOOR_LOCK_CTRL,GPIO_DIR_OUT,0);

    /* 轮动检测引脚配置，边沿触发中断*/
    pmu_set_pin_to_CPU(EBIKE_ROTATION_ALARM_PORT, (1<<EBIKE_ROTATION_ALARM_GPIO_BIT));
    system_set_port_mux(EBIKE_ROTATION_ALARM_PORT,EBIKE_ROTATION_ALARM_GPIO_BIT,PORTC0_FUNC_C0);
	gpio_set_dir(EBIKE_ROTATION_ALARM_PORT,EBIKE_ROTATION_ALARM_GPIO_BIT,GPIO_DIR_IN);
	ext_int_set_port_mux(EXTI_7,EXTI_7_PC0);
	ext_int_set_type(EXTI_7, EXT_INT_TYPE_POS);
    ext_int_enable(EXTI_7);
	//os_timer_init(&g_pulse_cnt_timer, pulse_cnt_timer_cb, NULL);
	op_ret = timer_init(ROTATION_HARD_TIMER_ID,ROTATION_DET_PERIOD*1000,TIMER_PERIODIC);
	NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 2);
	TUYA_LOG_I(MOD_DEV, "timer_init timer_0 ret:%u\r\n",op_ret);
#if 1
    /* 振动检测引脚配置，边沿触发中断*/
    //振动检测信号,输入，边沿触发
    pmu_set_pin_to_CPU(EBIKE_VIBRATE_DETECT_PORT, (1<<EBIKE_VIBRATE_DETECT_GPIO_BIT));
    system_set_port_mux(EBIKE_VIBRATE_DETECT_PORT,EBIKE_VIBRATE_DETECT_GPIO_BIT,PORTC2_FUNC_C2);
	gpio_set_dir(EBIKE_VIBRATE_DETECT_PORT,EBIKE_VIBRATE_DETECT_GPIO_BIT,GPIO_DIR_IN);
	ext_int_set_port_mux(EXTI_5,EXTI_5_PC2);
    ext_int_set_type(EXTI_5, EXT_INT_TYPE_POS);
    ext_int_enable(EXTI_5);
#endif

	//超速报警，输入,边沿触发
    pmu_set_pin_to_CPU(EBIKE_SPEED_ALARM_PORT, (1<<EBIKE_SPEED_ALARM_GPIO_BIT));
    system_set_port_mux(EBIKE_SPEED_ALARM_PORT,EBIKE_SPEED_ALARM_GPIO_BIT,PORTC1_FUNC_C1);
	gpio_set_dir(EBIKE_SPEED_ALARM_PORT,EBIKE_SPEED_ALARM_GPIO_BIT,GPIO_DIR_IN);
	ext_int_set_port_mux(EXTI_6,EXTI_6_PC1);
    ext_int_set_type(EXTI_6, EXT_INT_TYPE_NEG);
    ext_int_enable(EXTI_6);

    /* 一键启动按键，下降沿触发中断 */
	//一键启动,输入,低有效
	pmu_set_pin_to_CPU(EBIKE_START_PORT, (1<<EBIKE_START_GPIO_BIT));
    system_set_port_mux(EBIKE_START_PORT,EBIKE_START_GPIO_BIT,PORTA4_FUNC_A4);
	gpio_set_dir(EBIKE_START_PORT,EBIKE_START_GPIO_BIT,GPIO_DIR_IN);
	ext_int_set_port_mux(EXTI_4,EXTI_4_PA4);
    ext_int_set_type(EXTI_4, EXT_INT_TYPE_NEG);
    ext_int_enable(EXTI_4);

    return op_ret;

}
int device_sensor_register(void)
{
    int op_ret = 0;
    op_ret = device_sensor_gpio_extern_det_init();
    ty_vfm_sensor_reg(&m_intf);
    return op_ret;
}
