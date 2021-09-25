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
 * @file    locdef_fsm.c
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.10
 * @brief   该文件用于实现定防器功能状态机
 *
******************************************************************************/
#include "tuya_log.h"
#include "locdef_audio.h"
#include "locdef_config.h"
#include "locdef_service.h"
#include "locdef_pin.h"
#include "vfm_sensor.h"
#include "vfm_fsm.h"
#include "vfm_base.h"
#include "locdef_dp.h"


static void fsm_warning_cb(void *param);
static void fsm_moving_cb(void *param);
static void fsm_acc_on_cb(void *param);
static void fsm_acc_off_def_cb(void *param);
static void fsm_acc_off_undef_cb(void *param);
static void fsm_hold_state_cb(void *param);



/* 状态迁移表 */
static VFM_FSM_TABLE_T sg_vfm_fsm_table[] = {
    /*  触发事件           初态           次态          用户回调函数        */
    //跳转到报警状态
    {SENSOR_EVENT_SHAKE, FSM_ACCOFF_DEF, FSM_WARNING, fsm_warning_cb},
    {SENSOR_EVENT_ROTATION, FSM_ACCOFF_DEF, FSM_WARNING, fsm_warning_cb},
    {SENSOR_EVENT_ACCON, FSM_ACCOFF_DEF, FSM_WARNING, fsm_warning_cb},

    //跳转到行驶状态
    {SENSOR_EVENT_ROTATION, FSM_ACCON, FSM_MOVING, fsm_moving_cb},

    //跳转到点火状态
    {SENSOR_EVENT_UNROTATION, FSM_MOVING, FSM_ACCON, fsm_acc_on_cb},
    {SENSOR_EVENT_REMOTE_ACCON, FSM_ACCOFF_UNDEF|FSM_ACCOFF_DEF, FSM_ACCON, fsm_acc_on_cb},
    {SENSOR_EVENT_ACCON, FSM_ACCOFF_UNDEF, FSM_ACCON, fsm_acc_on_cb},

    //跳转到熄火设防状态
    {SENSOR_EVENT_DEFENCE, FSM_ACCOFF_UNDEF, FSM_ACCOFF_DEF, fsm_acc_off_def_cb},
    {SENSOR_EVENT_ACCOFF, FSM_ACCON | FSM_WARNING , FSM_ACCOFF_DEF, fsm_acc_off_def_cb},

    //跳转到熄火解防状态
    {SENSOR_EVENT_UNDEFENCE, FSM_ACCOFF_DEF | FSM_WARNING, FSM_ACCOFF_UNDEF, fsm_acc_off_undef_cb},

    //状态保持
    {SENSOR_EVENT_UNROTATION, FSM_ACCOFF_DEF, FSM_HOLD_STATE, fsm_hold_state_cb},
    {SENSOR_EVENT_UNROTATION, FSM_WARNING, FSM_HOLD_STATE, fsm_hold_state_cb},
    {SENSOR_EVENT_SEARCH, FSM_ACCOFF_UNDEF|FSM_ACCOFF_DEF|FSM_ACCON, FSM_HOLD_STATE, fsm_hold_state_cb},
    {SENSOR_EVENT_CUSHION, FSM_ACCOFF_UNDEF|FSM_ACCOFF_DEF|FSM_ACCON, FSM_HOLD_STATE, fsm_hold_state_cb},
    {SENSOR_EVENT_OVERSPEED, FSM_MOVING, FSM_HOLD_STATE, fsm_hold_state_cb},
    {SENSOR_EVENT_SAFESPEED, FSM_MOVING, FSM_HOLD_STATE, fsm_hold_state_cb},

};

/* 计算迁移表数量 */
#define VFM_FSM_CALC_TABLE_NUMS (sizeof(sg_vfm_fsm_table)/sizeof(VFM_FSM_TABLE_T))


/* 状态机变量 */
static VFM_FSM_T sg_default_vfm_fsm =
{
    .fsm_table = sg_vfm_fsm_table,
    .table_nums = VFM_FSM_CALC_TABLE_NUMS,
};



/**
 * @brief   app状态机注册
 *
 * @param   cur_state程序运行时的定防器状态
 *
 * @return
 *
 * @note
 */
int locdef_fsm_register(uint32_t now_state)
{
    int op_ret = 0;

    op_ret = ty_vfm_fsm_register(&sg_default_vfm_fsm, now_state); 

    return op_ret;
}


/**
 * @brief   报警动作函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
static void fsm_warning_cb(void *param)
{
	uint32_t event;
	uint32_t alarm_type = DEVICE_ALARM;

	event = *((uint32_t *)param);
	
	locdef_dp_update(DPID_DEV_STATUS, DT_ENUM,1,&alarm_type);
	if (SENSOR_EVENT_SHAKE == event) {
		;
	}
	
    locdef_dev_audio_play(TONE_ALARM, PLAY_TYPE_LOOP, PLAYER_TIMEOUT_INTERVEL);

    TUYA_LOG_I(MOD_FSM, "user transfer to warning status callback");

}


/**
 * @brief   行驶动作函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
static void fsm_moving_cb(void *param)
{

    TUYA_LOG_I(MOD_FSM,"user transfer to moving status callback");

}


/**
 * @brief   ACC点火动作函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
static void fsm_acc_on_cb(void *param)
{
    uint32_t event;

    event = *((uint32_t *)param);

    if (SENSOR_EVENT_UNROTATION == event) {
		TUYA_LOG_I(MOD_FSM, "SENSOR_EVENT_SAFESPEED");
        
    } else if (SENSOR_EVENT_ACCON == event ||
               SENSOR_EVENT_REMOTE_ACCON == event) {
        locdef_set_onekey_func(KEY_ACC_OFF); 
        locdef_dev_audio_play(TONE_START_UP, PLAY_TYPE_ONCE, 0);
    }

    TUYA_LOG_I(MOD_FSM, "user transfer to acc on status callback");
}


/**
 * @brief   ACC熄火设防动作函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
static void fsm_acc_off_def_cb(void *param)
{
    locdef_set_onekey_func(KEY_ACC_ON); 
    locdef_dev_audio_play(TONE_STOP, PLAY_TYPE_ONCE, 0);

    TUYA_LOG_I(MOD_FSM, "user transfer to acc off status callback");

}


/**
 * @brief   ACC熄火解防动作函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
static void fsm_acc_off_undef_cb(void *param)
{
    locdef_dev_audio_stop();
    locdef_dev_audio_play(TONE_UNLOCK, PLAY_TYPE_ONCE, 0);
    TUYA_LOG_I(MOD_FSM, "user transfer to acc off undef status callback");
}


/**
 * @brief   状态保持动作函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
static void fsm_hold_state_cb(void *param)
{
    uint32_t event;
	static uint8_t flag = 0;
	tuya_dev_cfg_t *cfg_param = locdef_get_dev_cfg_param();
	uint32_t alarm_type = 0;

    event = *((uint32_t *)param);

    if (SENSOR_EVENT_SEARCH == event) {
		locdef_dev_audio_play(TONE_SEARCH, PLAY_TYPE_LOOP, PLAYER_TIMEOUT_INTERVEL);
    } else if (SENSOR_EVENT_CUSHION == event) {
        locdef_dev_audio_play(TONE_UNLOCK, PLAY_TYPE_ONCE, 0);
    } else if (SENSOR_EVENT_OVERSPEED == event) {
        TUYA_LOG_I(MOD_FSM, "SENSOR_EVENT_OVERSPEED");
		if( 0 == flag )
		{
		    flag = 1;
		    locdef_dev_audio_play(TONE_SPEEDING, PLAY_TYPE_ONCE,cfg_param->limit_speed_alarm_interval*1000);
		}
		

		alarm_type = DEVICE_ALARM;
		//locdef_dp_update(DPID_DEV_STATUS, DT_ENUM,1,&alarm_type);
    } else if (SENSOR_EVENT_SAFESPEED == event) {
		TUYA_LOG_I(MOD_FSM, "SENSOR_EVENT_SAFESPEED");
		flag = 0;
        locdef_dev_audio_stop();
    }

    TUYA_LOG_I(MOD_FSM, "fsm_hold_state_cb callback");
}


