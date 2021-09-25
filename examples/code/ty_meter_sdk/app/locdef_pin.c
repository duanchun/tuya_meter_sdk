
#include "locdef_pin.h"
//#include "driver_plf.h"
#include "driver_exti.h"
#include "driver_adc.h"
#include "tuya_ble_log.h"
#include "vfm_log.h"
#include "locdef_audio.h"
#include "locdef_service.h"
#include "locdef_main.h"
#include "vfm_vbat.h"
#include "vfm_sensor.h"
#include "tuya_kel_os.h"
#include "tuya_kel_timer.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "locdef_dp.h"
#include "tuya_log.h"


#define IO_FILTER_READ_CNT    (3)
#define IO_FILTER_VALID_HIGH  (0x0007)
#define IO_FILTER_VALID_LOW   (0x0000)
#define IO_INVALID_LEVEL      (2)


static tuya_kel_task_handle s_io_filter_task_handle = NULL;
static ONEKEY_FUNC_E s_onekey_func = KEY_ACC_ON;
static tuya_kel_timer_handle s_one_start_key_timer = NULL;

//extern uint8_t g_rotation_irq_en;
void tuya_pin_irq_enable(uint8_t gpio)
{
	//uint8_t port = (gpio>>4);
	//uint8_t gpio_bit = (gpio&0x0F);

	if(EBIKE_VIBRATE_DETECT_GPIO == gpio)//振动检测信号
	{
		ext_int_enable(EXTI_5);
	}else if(EBIKE_ROTATION_ALARM_GPIO == gpio){//轮动检测信号,输入，检测PWM脉冲
		ext_int_enable(EXTI_7);
	}else if(EBIKE_433_DATA_GPIO == gpio){//433数据通讯
    	ext_int_enable(EXTI_15);
    }else if(EBIKE_START_GPIO == gpio){//一键启动
    	ext_int_enable(EXTI_4);
    }else if(EBIKE_SPEED_ALARM_GPIO == gpio){//超速报警
		ext_int_enable(EXTI_6);
    }else if(EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO == gpio){//电门锁检测
        ext_int_enable(EXTI_11);
    }
}

void tuya_pin_irq_disable(uint8_t gpio)
{
	//uint8_t port = (gpio>>4);
	//uint8_t gpio_bit = (gpio&0x0F);

	if(EBIKE_VIBRATE_DETECT_GPIO == gpio)//振动检测信号
	{
		ext_int_disable(EXTI_5);
	}else if(EBIKE_ROTATION_ALARM_GPIO == gpio){//轮动检测信号,输入，检测PWM脉冲
		ext_int_disable(EXTI_7);
	}else if(EBIKE_433_DATA_GPIO == gpio){//433数据通讯
    	ext_int_disable(EXTI_15);
    }else if(EBIKE_START_GPIO == gpio){//一键启动
    	ext_int_disable(EXTI_4);
    }else if(EBIKE_SPEED_ALARM_GPIO == gpio){//超速报警
		ext_int_disable(EXTI_6);
    }else if(EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO == gpio){//电门锁检测
        ext_int_disable(EXTI_11);
    }
}


void tuya_pin_write_no_pmu(uint8_t gpio, uint8_t value)
{
    uint8_t port = (gpio>>4);
	uint8_t gpio_bit = (gpio&0x0F);

	gpio_set_pin_value(port,gpio_bit,value); 
}

uint8_t tuya_pin_read_no_pmu(uint8_t gpio)
{
	uint8_t port = (gpio>>4);
	uint8_t gpio_bit = (gpio&0x0F);

	return gpio_get_pin_value(port,gpio_bit);
}

void tuya_pin_write(uint8_t gpio, uint8_t value)
{
    uint8_t port = (gpio>>4);
	uint8_t gpio_bit = (gpio&0x0F);
	
	pmu_set_gpio_value(port,(1<<gpio_bit),value);
}

uint8_t tuya_pin_read(uint8_t gpio)
{
	uint8_t port = (gpio>>4);
	uint8_t gpio_bit = (gpio&0x0F);

	return pmu_get_gpio_value(port,gpio_bit);
}

void ty_app_exti_isr(uint32_t status)
{
    //TUYA_LOG_I(MOD_DEV, "exti status::%u",status);
    if((status>>EXTI_11) &0x01)//电门锁检测
    {
        //TUYA_LOG_I(MOD_DEV, "electric door lock check\r\n");
		locdef_elec_door_det_irq_cb(NULL);
    }else if((status>>EXTI_4) &0x01){//一键启动
        //TUYA_LOG_I(MOD_DEV, "one button start\r\n");
        locdef_onekey_start_irq_cb(NULL);
    }else if((status>>EXTI_5) &0x01){//振动检测信号
    	locdef_vib_sensor_irq_cb(NULL);
    }else if((status>>EXTI_6) &0x01){//超速报警
    	//TUYA_LOG_I(MOD_DEV, "speeding alarm\r\n");
    	locdef_speeding_irq_cb(NULL);
    }else if((status>>EXTI_15) &0x01){//433数据通讯
    	//TUYA_LOG_I(MOD_DEV, "433 data\r\n");
    	tuya_ble_433_irq_cb(NULL);
    }else if((status>>EXTI_7) &0x01){//轮动检测信号,输入，检测PWM脉冲
    	//TUYA_LOG_I(MOD_DEV, "rotation det\r\n");
		locdef_rotation_det_irq_cb(NULL);
    }
}

ONEKEY_FUNC_E locdef_get_onekey_func(void)
{
    return s_onekey_func;
}

void locdef_set_onekey_func(ONEKEY_FUNC_E func)
{
    s_onekey_func = func;
}


int locdef_io_filter_notify(uint16_t pin,uint32_t timeout)
{
	uint32_t temp = pin;

	//VFM_INFO(VFM_UPLOAD_LOG,"pin:%04x",pin);
    tuya_kel_send_msg(s_io_filter_task_handle,&temp,0);
    return 0;
}

/**
 * @brief   IO滤波处理
 *
 * @param
 *
 * @return  void
 */
static uint8_t io_filter_handle(uint8_t pin)
{
    static uint16_t read_cnt = 0;
    static uint16_t pin_status = 0;
    uint8_t valid_io_level = IO_INVALID_LEVEL;

    while (1)
    {
        if (tuya_pin_read_no_pmu(pin)) {
            pin_status = (pin_status << 1) | 0x01;
        } else {
            pin_status = (pin_status << 1);
        }

        read_cnt++;
        if (read_cnt >= IO_FILTER_READ_CNT) {
            if (pin_status == IO_FILTER_VALID_HIGH) {
                valid_io_level = TUYA_PIN_HIGH;
            } else if (pin_status == IO_FILTER_VALID_LOW){
                valid_io_level = TUYA_PIN_LOW;
            } else {
                valid_io_level = IO_INVALID_LEVEL;
            }
            read_cnt = 0;
            pin_status = 0;
            break;
        }
        ty_system_delay_ms(20);
    }

    return valid_io_level;
}

/**
 * @brief   IO中断滤波处理函数
 *
 * @param
 *
 * @return  void
 */
static void locdef_gpio_irq_filter_handle(uint8_t pin, uint8_t level)
{
    //uint8_t event = 0;
    DEV_RUNNING_PARAM_T *run_param = NULL;
    run_param = locdef_get_run_param();

    if (level == IO_INVALID_LEVEL) {
        TUYA_LOG_I(MOD_DEV, " valid gpio irq");
    }

    switch (pin)
    {
    #if 0
        case LOCDEF_PWR_OFF_PIN: /* 主电源掉电检测中断 */
            if (level == TUYA_PIN_LOW) {
                ty_vfmk_vbat_event_post(VBAT_EXTION,VBAT_CHG_EVENT_START);
                TUYA_LOG_I(MOD_DEV, " irq main power on");
            } else {
                ty_vfmk_vbat_event_post(VBAT_EXTION,VBAT_CHG_EVENT_DISCONNECT);
                TUYA_LOG_I(MOD_DEV, " irq main power down");
            }
            break;
		#endif
        case EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO:
            if (level == TUYA_PIN_HIGH) {
                run_param->single_mileage = 0;
                ty_vfm_sensor_event_post(SENSOR_EVENT_ACCON);
			    ext_int_set_type(EXTI_11, EXT_INT_TYPE_NEG);
			    
            } else {
				uint8_t temp = 0;
                ty_vfm_sensor_event_post(SENSOR_EVENT_ACCOFF);
				ext_int_set_type(EXTI_11, EXT_INT_TYPE_POS);
				locdef_dp_update(DPID_ACC_CTRL,DT_BOOL,1,&temp);
            }

            TUYA_LOG_I(MOD_DEV, "irq edoor status:%s", level?"open":"close");
            break;
        case EBIKE_SPEED_ALARM_GPIO:
            if (level == TUYA_PIN_HIGH) {
				ext_int_set_type(EXTI_6, EXT_INT_TYPE_NEG);
                ty_vfm_sensor_event_post(SENSOR_EVENT_SAFESPEED);
            } else {
				ext_int_set_type(EXTI_6, EXT_INT_TYPE_POS);
                ty_vfm_sensor_event_post(SENSOR_EVENT_OVERSPEED);
            }
            TUYA_LOG_I(MOD_DEV, " irq speeding status:%s", level?"false":"true");
            break;
        case EBIKE_START_GPIO:
            if (level == TUYA_PIN_LOW) {
                if (KEY_ACC_ON == locdef_get_onekey_func()) {
                    ty_vfm_sensor_event_post(SENSOR_EVENT_ACCON);
                } else {
                    ty_vfm_sensor_event_post(SENSOR_EVENT_ACCOFF);
                }
            }
            TUYA_LOG_I(MOD_DEV, " irq onekey start/stop trigger");
            break;
        default:
            break;
    }
}

static void locdef_one_start_key_check_cb(void* pTimerArg)
{
	uint8_t io_level = 0;
	
	io_level = io_filter_handle(EBIKE_START_GPIO);
	locdef_gpio_irq_filter_handle(EBIKE_START_GPIO, io_level);
	tuya_pin_irq_enable(EBIKE_START_GPIO);
}

static int locdef_one_start_key_check(void)
{
	if(NULL == s_one_start_key_timer)
	{
		tuya_kel_timer_create(&s_one_start_key_timer,500,TY_KEL_TIMER_SINGLE_SHOT,locdef_one_start_key_check_cb);
		if(NULL == s_one_start_key_timer)
		{
			TUYA_LOG_I(MOD_DEV, " create timer failed");
			return 1;
		}
		tuya_kel_timer_start(s_one_start_key_timer);
	}else{
		tuya_kel_timer_restart(s_one_start_key_timer,500);
	}
	
}

static int32_t locdef_io_filter_task(void* param)
{
	TUYA_RET_E ret = TUYA_OK;
    uint32_t pin = 0;
	uint8_t io_level = 0;
	TONE_TYPE_E cur_tone = TONE_MUTE; 

	ret = tuya_kel_wait_msg(s_io_filter_task_handle,(void*)&pin,0);
	if(TUYA_OK != ret)
	{
	    TUYA_LOG_W(MOD_SERVER, "tuya_kel_wait_msg failed");
		goto END;
	}

	TUYA_LOG_I(MOD_DEV, " locdef_io_filter_task pin: %02X", pin);
	cur_tone = locdef_get_cur_play_tone();
	/* 超速需要延时判断,避免突然加速还未检测到轮动,超速释放需要快速响应 */
	if (pin == EBIKE_SPEED_ALARM_GPIO && cur_tone != TONE_SPEEDING) { 
	    //ty_system_delay_ms(3000);
	    ty_system_delay_ms(500);
	} else if (pin == EBIKE_START_GPIO) {
	    if(0 != locdef_one_start_key_check())
	    {
			tuya_pin_irq_enable(pin);
	    }
		goto END;
	}

	io_level = io_filter_handle(pin);
	locdef_gpio_irq_filter_handle(pin, io_level);
	tuya_pin_irq_enable(pin);
END:
	TUYA_LOG_I(MOD_DEV, "locdef_io_filter_task:%u", os_get_free_heap_size());
    return 0;//must return 0
}

static void os_timer_over_speed_timer_handler(void *arg)
{
    tuya_pin_irq_enable(EBIKE_SPEED_ALARM_GPIO);
}

int16_t tuya_gpio_init(void)
{
    #define TY_485_SEND    0

	#if TY_485_SEND 
	// 485
	TUYA_GPIO_INIT(EBIKE_485_SWITCH_EN,GPIO_DIR_OUT,1);
	#else
	TUYA_GPIO_INIT(EBIKE_485_SWITCH_EN,GPIO_DIR_OUT,0);
	#endif

	s_io_filter_task_handle = (void*)tuya_kel_task_create(locdef_io_filter_task,NULL,4,10,0,0,NULL);
	if (s_io_filter_task_handle == NULL)
	{
		TUYA_LOG_E(MOD_PUBLIC,"create io filter task err");
		return 1;
	}
	
	NVIC_SetPriority(EXTI_IRQn, 2);
    NVIC_EnableIRQ(EXTI_IRQn);
	
	/************************************  中断配置初始化} ************* ******************/
    return 0;
}

