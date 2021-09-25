/**********************************Copyright (c)**********************************
**                       版权所有 (C), 2015-2021, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    vfm_fsm.c
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.09
 * @brief   该文件用于实现VFM状态机
 *
******************************************************************************/
#include "vfm_log.h"
#include "vfm_fsm.h"
#include "vfm_lock.h"
#include "vfm_sensor.h"
#include "vfm_upload.h"
#include "tuya_kel_timer.h"

static VFM_FSM_INS_T  g_vfm_fsm_instance;

static tuya_kel_timer_handle       g_warning_recovery_timer = {0};

static uint32_t         g_state_record = 0;
static uint32_t         g_recovery_time = VFM_DEFAULT_WARNING_RECOVERY_TIME;

static int vfm_start_warning_timeout(uint32_t now_state);


/* 动作函数声明 */
static void vfm_fsm_warning_cb(void *param);
static void vfm_fsm_moving_cb(void *param);
static void vfm_fsm_acc_on_cb(void *param);
static void vfm_fsm_acc_off_def_cb(void *param);
static void vfm_fsm_acc_off_undef_cb(void *param);
static void vfm_fsm_hold_state_cb(void *param);


/* 状态会话处理表 */
static VFM_FSM_SESSION_TABLE_T vfm_fsm_session_table[] = {
    /*状态类型              状态回话处理*/
    {FSM_ACCOFF_UNDEF,  vfm_fsm_acc_off_undef_cb},
    {FSM_ACCOFF_DEF,    vfm_fsm_acc_off_def_cb},
    {FSM_ACCON,         vfm_fsm_acc_on_cb},
    {FSM_MOVING,        vfm_fsm_moving_cb},
    {FSM_WARNING,       vfm_fsm_warning_cb},
    {FSM_HOLD_STATE,    vfm_fsm_hold_state_cb},
};


/**
 * @brief   状态转换
 *
 * @param   pfsm状态机对象, state跳转的状态
 *
 * @return  无
 *
 * @note
 */
int ty_vfm_fsm_state_transfer(uint32_t state)
{
    g_vfm_fsm_instance.now_state = state;

    return 0;
}

/**
 * @brief   获取状态对应的数组序号
 *
 * @param   state当前状态,index状态序号(从0开始计算)
 *
 * @return  无
 *
 * @note
 */
static uint32_t ty_vfm_get_state_index(uint32_t state)
{
    uint32_t index = 0;

    if (state) {
        while(!(state&0x01)) {
            index++;
            state = state >> 1;
        }
    }

    return index;
}


/**
 * @brief   获取状态机当前状态
 *
 * @param   now_state当前状态,index状态序号(从0开始计算)
 *
 * @return  无
 *
 * @note
 */
int ty_vfm_fsm_get_now_state(uint32_t* now_state, uint32_t* index)
{
    if (now_state) {
        *now_state = g_vfm_fsm_instance.now_state;
    }

    if (index) {
        *index = ty_vfm_get_state_index(g_vfm_fsm_instance.now_state);
    }

    return 0;
}


/**
 * @brief   状态机处理函数
 *
 * @param   pfsm状态机对象, event触发事件, param动作执行参数
 *
 * @return
 *
 * @note
 */
int ty_vfm_fsm_event_proc(uint32_t event, void *param)
{
    int op_ret = 0;
    uint32_t now_state = 0, next_state = 0;
    VFM_FSM_T *fsm = NULL;
    VFM_FSM_TABLE_T *pfsm_table = NULL;
    TY_VFM_FSM_ACTFUNC_CB user_func = NULL;
    int match = false;
    uint32_t i = 0;


    fsm = g_vfm_fsm_instance.fsm;
    now_state = g_vfm_fsm_instance.now_state;
    pfsm_table = g_vfm_fsm_instance.fsm->fsm_table;

    /* 遍历状态表      */
    for (i = 0; i < fsm->table_nums; i++) {
        /* 支持一个事件触发多个状态 */
        if (event == pfsm_table[i].event && (now_state & pfsm_table[i].now_state)) {
            match = true;
            next_state = pfsm_table[i].next_state;
            user_func = pfsm_table[i].user_func_cb;
            break;
        }
    }

    if (match) {

        VFM_INFO(VFM_FSM_LOG, "vfm_fsm now_state[0x%x]->next_state[0x%x]", now_state, next_state);

        if (FSM_HOLD_STATE != next_state) {
            if (FSM_WARNING == next_state) {
                vfm_start_warning_timeout(now_state);
            }
            ty_vfm_fsm_state_transfer(next_state);
        }

        /* 遍历会话表 */
        for (i = 0; i < VFM_DEFAULT_STATE_TYPE_NUMS; i++) {
            if (vfm_fsm_session_table[i].next_state == next_state) {
                if (vfm_fsm_session_table[i].action_func_cb) {
                    vfm_fsm_session_table[i].action_func_cb(param);
                    break;
                }
            }
        }

        if (user_func != NULL) {
            user_func(param);
        }

        if (FSM_HOLD_STATE != next_state) {
            ty_vfm_upload_event_send(next_state); 
        }

        op_ret = 0;
    }
    else {
        VFM_ERR(VFM_FSM_LOG, "vfm_fsm not found now_state[0x%x]->next_state[0x%x]", now_state, next_state);
        op_ret = -1;
    }

    return op_ret;
}


/**
 * @brief   报警状态恢复超时回调
 *
 * @param   timerID   定时器id
 * @param   pTimerArg 定时器函数处理参数
 *
 * @return  无
 */
static void vfm_warning_recovery_cb(void* pTimerArg)
{
    /* 恢复状态机报警前的状态 */
    ty_vfm_fsm_state_transfer(g_state_record);

    VFM_INFO(VFM_FSM_LOG, "warning recovery to [0x%x] status", g_state_record);
}

/**
 * @brief   关闭报警超时定时器
 *
 * @param
 * @param
 *
 * @return  无
 */
static int vfm_stop_warning_timeout(void)
{
    int op_ret;

	if(NULL != g_warning_recovery_timer)
	{
		tuya_kel_timer_stop(g_warning_recovery_timer);
		g_warning_recovery_timer = NULL;
	}

    g_state_record = 0;

    VFM_INFO(VFM_FSM_LOG, "stop warning timeout ret[%d]", op_ret);

    return op_ret;

}


/**
 * @brief   启动报警超时定时器
 *
 * @param
 * @param
 *
 * @return  无
 */
static int vfm_start_warning_timeout(uint32_t now_state)
{
    int op_ret;

    if(NULL != g_warning_recovery_timer)
	{
		tuya_kel_timer_stop(g_warning_recovery_timer);
		tuya_kel_timer_delete(g_warning_recovery_timer);
		g_warning_recovery_timer =  NULL;
	}
	tuya_kel_timer_create(&g_warning_recovery_timer,g_recovery_time*1000,TY_KEL_TIMER_SINGLE_SHOT,vfm_warning_recovery_cb);
	tuya_kel_timer_start(g_warning_recovery_timer);
	
    g_state_record = now_state;

    VFM_INFO(VFM_FSM_LOG, "start warning timeout ret[%d]", op_ret);

    return op_ret;

}


/**
 * @brief   启动报警超时定时器
 *
 * @param
 * @param
 *
 * @return  无
 */
int ty_vfm_set_warning_recovery_time(uint32_t time_s)
{
    g_recovery_time = time_s;
    return 0;
}


/**
 * @brief   注册vfm状态机
 *
 * @param   fsm_handle状态机句柄指针
 *
 * @return
 *
 * @note
 */
int ty_vfm_fsm_register(VFM_FSM_T *pfsm, uint32_t now_state)
{
    int op_ret = 0;

    g_vfm_fsm_instance.fsm = pfsm;
    g_vfm_fsm_instance.now_state = now_state;
    g_vfm_fsm_instance.init_flag = true;

    return 0;
}


/**
 * @brief   注销vfm状态机
 *
 * @param   fsm_handle状态机句柄指针
 *
 * @return
 *
 * @note
 */
int ty_vfm_fsm_deregister(VFM_FSM_T *pfsm)
{
    int op_ret = 0;

	if(NULL != g_warning_recovery_timer)
	{
		tuya_kel_timer_delete(g_warning_recovery_timer);
		g_warning_recovery_timer = NULL;
	}

    g_vfm_fsm_instance.fsm = NULL;
    g_vfm_fsm_instance.init_flag = false;

    return 0;
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
static void vfm_fsm_warning_cb(void *param)
{
    uint32_t event;

    event = *((uint32_t *)param);

    if (SENSOR_EVENT_SHAKE == event) {

    } else if (SENSOR_EVENT_ROTATION == event) {
        ty_vfm_acc_ctrl(ACC_POWER_ON);
        ty_vfm_lock_ctrl(LOCK_TYPE_MOTOR_LOCK, LOCK);
    } else if (SENSOR_EVENT_ACCON == event) {

    }

    /* 上报报警DP */

    /* 启动超时报警恢复 */

    VFM_INFO(VFM_FSM_LOG, "transfer to warning status callback");

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
static void vfm_fsm_moving_cb(void *param)
{
    /* 上报行驶DP */

    VFM_INFO(VFM_FSM_LOG, "transfer to moving status callback");

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
static void vfm_fsm_acc_on_cb(void *param)
{
    uint32_t event;

    event = *((uint32_t *)param);

    if (SENSOR_EVENT_UNROTATION == event) {

    } else if (SENSOR_EVENT_ACCON == event ||
               SENSOR_EVENT_REMOTE_ACCON == event) {
        /* 关闭防盗 */
        ty_vfm_lock_ctrl(LOCK_TYPE_DEF_LOCK, UNLOCK);
        ty_vfm_lock_ctrl(LOCK_TYPE_MOTOR_LOCK, UNLOCK);
        ty_vfm_acc_ctrl(ACC_POWER_ON);

        /* 上报ACC点火DP */
        //ty_vfm_upload_event_send(DP_FUN_ACC);
    }

    VFM_INFO(VFM_FSM_LOG, "transfer to acc on status callback");
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
static void vfm_fsm_acc_off_def_cb(void *param)
{
    /* 开启防盗 */
    ty_vfm_lock_ctrl(LOCK_TYPE_DEF_LOCK, LOCK);
    ty_vfm_lock_ctrl(LOCK_TYPE_MOTOR_LOCK, UNLOCK);
    ty_vfm_acc_ctrl(ACC_POWER_OFF);

    /* 上报设防DP */
    //ty_vfm_upload_event_send(DP_FUN_ACC);
    //ty_vfm_upload_event_send(DP_FUN_DEF_LOCK);

    VFM_INFO(VFM_FSM_LOG, "transfer to acc off status callback");

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
static void vfm_fsm_acc_off_undef_cb(void *param)
{
    ACC_POWER_E action = 0;
    /* 关闭防盗 */
    ty_vfm_lock_ctrl(LOCK_TYPE_DEF_LOCK, UNLOCK);
	/* 关闭电门 */
	ty_vfm_acc_ctrl(ACC_POWER_OFF);

	if(FSM_ACCOFF_DEF == g_state_record)
	{
	    vfm_stop_warning_timeout();
	}

	tuya_kel_system_delay_ms(150);
    ty_vfm_acc_read(&action);
	if(ACC_POWER_ON == action)
	{
	    ty_vfm_sensor_event_post(SENSOR_EVENT_ACCON);
	}

    /* 上报解防DP */
    //ty_vfm_upload_event_send(DP_FUN_DEF_LOCK);

    VFM_INFO(VFM_FSM_LOG, "transfer to acc off undef status callback");

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
static void vfm_fsm_hold_state_cb(void *param)
{
    uint32_t event;

    event = *((uint32_t *)param);

    if (SENSOR_EVENT_UNROTATION == event) {
        ty_vfm_acc_ctrl(ACC_POWER_OFF);
        ty_vfm_lock_ctrl(LOCK_TYPE_MOTOR_LOCK, UNLOCK);
    } else if (SENSOR_EVENT_CUSHION == event) {
        ty_vfm_lock_ctrl(LOCK_TYPE_CUSHION_LOCK, UNLOCK);
    } else if (SENSOR_EVENT_OVERSPEED == event) {
        /* 上报超速DP */
    } else if (SENSOR_EVENT_SAFESPEED == event) {
        /* 上报超速解除DP */
    }

    VFM_INFO(VFM_FSM_LOG, "vfm_fsm_hold_state_cb callback");
}

