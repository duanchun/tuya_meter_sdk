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

/**
 * @file    locdef_service.h
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.12
 * @brief   该文件用于实现定防器业务
 *
******************************************************************************/

#include "vfm_base.h"
#include "vfm_sensor.h"
#include "vfm_lock.h"
#include "vfm_upload.h"
#include "vfm_fsm.h"
#include "tuya_log.h"
#include "tuya_kel_os.h"
#include "locdef_service.h"
#include "tuya_kel_timer.h"
#include "tuya_mid_power_mgr.h"
#include "locdef_pin.h"
#include "locdef_hid.h"

static tuya_kel_timer_handle s_rotation_det_timer = NULL;
static tuya_kel_task_handle s_fsm_task_handle = NULL;
static tuya_power_mgr_handle s_power_mgr_handle = NULL;

DEV_RUNNING_PARAM_T        g_run_para = {
    .dev_status = DEVICE_NORMAL,
    .speed = 0,
    .rotation_irq_en = false,
    .rotation_det_cplt = true,
    .hid_dev_bond = TY_HID_DEV_UNBOND
};


/**
 * @brief   发送车辆状态机处理事件给线程
 *
 * @return  无
 */
DEV_RUNNING_PARAM_T * locdef_get_run_param(void)
{
    return &g_run_para;
}

void locdef_cushion_lock(void)
{
    TUYA_LOG_I(MOD_SERVER, "locdef_cushion_lock");
    /* 开启坐垫锁，定时器延时500ms */
    //g_run_para.cushion_lock_pulse_en = true;
    
    tuya_pin_write(EBIKE_BUCKET_LOCK_GPIO, TUYA_PIN_HIGH);
	tuya_kel_system_delay_ms(500);
	tuya_pin_write(EBIKE_BUCKET_LOCK_GPIO, TUYA_PIN_LOW);
}

uint32_t locdef_get_speed(void)
{
    return g_run_para.speed;
}

float locdef_get_single_mileage(void)
{
    return g_run_para.single_mileage;
}



/**
 * @brief   振动报警控制函数
 *
 * @param
 *
 * @return  无
 */
void locdef_vib_alarm_ctrl(int enable)
{
    DEV_RUNNING_PARAM_T * run_param = locdef_get_run_param();
    if (enable) {
        run_param->alarm_enable = true;
        tuya_pin_irq_enable(EBIKE_VIBRATE_DETECT_GPIO);
    } else {
        run_param->alarm_enable = false;
        tuya_pin_irq_disable(EBIKE_VIBRATE_DETECT_GPIO);
    }
}


int locdef_vib_alarm_enable(void)
{
    DEV_RUNNING_PARAM_T * run_param = locdef_get_run_param();
    return run_param->alarm_enable;
}

/**
 * @brief   设备上电处理
 *
 * @param   void
 *
 * @return
 */
void locdef_dev_power_on_handle(uint32_t *state)
{
    ACC_POWER_E acc_power;

    ty_vfm_acc_read(&acc_power);

    if (ACC_POWER_ON == acc_power) {
        *state = FSM_ACCON;
		locdef_set_onekey_func(KEY_ACC_OFF);
    } else {
		locdef_set_onekey_func(KEY_ACC_ON);
    }

    if (*state == FSM_ACCOFF_DEF) {
        ty_vfm_lock_ctrl(LOCK_TYPE_DEF_LOCK, LOCK);
    } else {
        ty_vfm_lock_ctrl(LOCK_TYPE_DEF_LOCK, LOCK);
        ty_vfm_lock_ctrl(LOCK_TYPE_MOTOR_LOCK, UNLOCK);
    }

}

/**
 * @brief   轮动检测中断控制函数
 *
 * @param
 *
 * @return  无
 */
static void locdef_dev_rotation_irq_ctrl(uint32_t period)
{
    uint32_t now_state = 0;
    static uint16_t base_tick_cnt = 0;
    static uint16_t release_cnt = 0;
    uint16_t rotation_rele_time = 0;
    DEV_RUNNING_PARAM_T *run_param = NULL;
    run_param = locdef_get_run_param();

    /* 1000ms周期,开启一次轮动检测中断 */
    if (base_tick_cnt % (1000/period) == 0) {

        ty_vfm_fsm_get_now_state(&now_state, NULL);

        /* 当报警态或行驶态时未检测到轮动信号,则触发释放轮动事件 */
        if (now_state == FSM_WARNING || now_state == FSM_ACCOFF_DEF || now_state == FSM_MOVING/* || now_state == FSM_ACCON*/) {
			//TUYA_LOG_I(MOD_SERVER, "now_state==0x%04X",now_state);
            if (!run_param->rotation_det_cplt)
            {
                release_cnt++;
				//TUYA_LOG_I(MOD_SERVER, "release_cnt==%d",release_cnt);
                /* 不同状态下轮动释放时间不同 */
                if (now_state == FSM_WARNING || now_state == FSM_ACCOFF_DEF) {
                    rotation_rele_time = ALARM_ROTATION_RELEASE_TIMES;
                } else if (now_state == FSM_MOVING) {
                    rotation_rele_time = TRAVEL_ROTATION_RELEASE_TIMES;
                }
                /* 提前清零速度 */
                if (release_cnt > TRAVEL_ROTATION_RELEASE_TIMES/period) {
                    run_param->speed = 0;
                }
				//TUYA_LOG_I(MOD_SERVER, "check==%d",rotation_rele_time/period);
                /* 检测多次轮动释放动作 */
                if (release_cnt > (rotation_rele_time/period)) {
                    release_cnt = 0;
                    run_param->rotation_det_cplt = true;
                    ty_vfm_sensor_event_post(SENSOR_EVENT_UNROTATION);
                }
            }
        }

        if (now_state != FSM_ACCOFF_UNDEF && !run_param->rotation_irq_en) {
            run_param->rotation_irq_en = true;
            run_param->rotation_det_cplt = false;
		    //TUYA_LOG_I(MOD_SERVER, " rotation_irq enable");
            tuya_pin_irq_enable(EBIKE_ROTATION_ALARM_GPIO); 
        }
    }

    base_tick_cnt++;
}


/**
 * @brief   轮动检测定时器回调
 *
 * @param   timerID   定时器id
 * @param   pTimerArg 定时器函数处理参数
 *
 * @return  无
 */
static void locdef_rotation_det_timer_cb(tuya_ble_timer_t param)
{
    uint32_t period = ROTATION_DET_PERIOD;

    /* 轮动检测中断控制 */
    locdef_dev_rotation_irq_ctrl(period);

}


static int tuya_fsm_proc_task(void *param)
{
	TUYA_RET_E ret = TUYA_OK;
    BASE_EVENT_t msg_info = {0};
	uint32_t now_state = 0;
	uint16_t msg_cnt = 0;

	tuya_mid_mod_sleep_inactive(s_power_mgr_handle);
	
    ret = tuya_kel_wait_msg(s_fsm_task_handle,(void*)&msg_info,0);
	if(TUYA_OK != ret)
	{
	    TUYA_LOG_W(MOD_SERVER, "tuya_kel_wait_msg failed");
		goto END;
	}
	
	ty_vfm_fsm_get_now_state(&now_state, NULL);
	TUYA_LOG_I(MOD_SERVER, "now_state:%04x,evt:0x%04x",now_state,msg_info.event);
	ty_vfm_fsm_event_proc(msg_info.event, &msg_info.event);

	msg_cnt = tuya_kel_msg_cnt_get(s_fsm_task_handle);
	TUYA_LOG_I(MOD_SERVER, "msg_cnt==%u",msg_cnt);
	if(0 == msg_cnt)
	{
		tuya_mid_mod_sleep_active(s_power_mgr_handle);
	}
END:
	return 0;//must return 0
}

static void tuya_core_power_mgr_cbk(TUYA_LOW_PWR_EVT_E evt)
{
	if(SYS_EVT_SUSPEND == evt)
	{
	}else{ //SYS_EVT_RESUME
	}
}

int tuya_fsm_init(void)
{
	tuya_kel_timer_create(&s_rotation_det_timer, ROTATION_DET_PERIOD, TY_KEL_TIMER_REPEATED, locdef_rotation_det_timer_cb);
	if (NULL == s_rotation_det_timer )
	{
		TUYA_LOG_E(MOD_SERVER,"create rotation det timer failed");
	}else{
		tuya_kel_timer_start(s_rotation_det_timer);
	}

	/* 状态机事件处理线程 */
    s_fsm_task_handle = (void*)tuya_kel_task_create(tuya_fsm_proc_task,NULL,sizeof(BASE_EVENT_t),10,0,0,NULL);
	if (NULL == s_fsm_task_handle )
	{
		TUYA_LOG_E(MOD_SERVER,"start thread tuya_fsm_proc_task err");
		return 1;
	}

	s_power_mgr_handle = tuya_mid_pwr_mgr_register(tuya_core_power_mgr_cbk);
	if (NULL == s_power_mgr_handle )
	{
		TUYA_LOG_E(MOD_SERVER,"power manager register failed");
		return 1;
	}

	tuya_vfm_base_init(s_fsm_task_handle);
	return 0;
}


