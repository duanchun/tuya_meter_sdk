/**********************************Copyright (c)**********************************
**                       版权所有 (C), 2015-2021, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    locdef_app_fsm.h
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.05.12
 * @brief   该文件用于实现定防器功能状态机
 *
******************************************************************************/

#ifndef __LOCDEF_APP_FSM_H_
#define __LOCDEF_APP_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_type.h"

#ifdef _LOCDEF_APP_FSM_GLOBAL
    #define _LOCDEF_APP_FSM_EXT
#else
    #define _LOCDEF_APP_FSM_EXT extern
#endif
typedef void (*TY_FSM_ACTFUNC_CALLBACK)(void *);

typedef struct fsm_table_s
{
    uint8_t event;                  /* 触发事件 */
    uint8_t cur_state;              /* 当前状态 */
    void (*event_actfunc)(void *);  /* 动作函数 */
    uint8_t next_state;             /* 跳转状态 */
}FSM_TABLE_T;

typedef struct fsm_s
{
    FSM_TABLE_T *fsm_table;         /* 状态迁移表 */
    uint8_t cur_state;              /* 状态机当前状态 */
    uint8_t table_nums;             /* 状态机状态迁移表数量 */
    uint8_t init_flag;              /* 初始化标志 */
}FSM_T;


/** @enum FSM_STATE_E
 *  @brief 状态机运行状态
 *
 */
//typedef enum
//{
//    FSM_GENERAL_LOCKING = 0x01, // 普通锁定状态
//    FSM_ALARM_LOCKING   = 0x02, // 防盗锁定状态
//    FSM_START_STATIC    = 0x04, // 启动静止状态
//    FSM_START_TRAVEL    = 0x08, // 启动行驶状态
//    FSM_HOLD_STATE      = 0x10, // 状态保持,处理状态不变的情况
//    FSM_CUSTOM_STATE    = 0x20, // 通过回调函数自行切换状态
//}FSM_STATE_E;

/** @enum FSM_TRIG_EVENT_E
 *  @brief 状态机触发事件
 *
 */
typedef enum
{
    FSM_VIB_DETECTION_EVENT = 0x01,             // 振动检测事件
    FSM_BLE_UNLOCK_EVENT,                       // 蓝牙开锁事件
    FSM_APP_OR_RCU433_LOCK_EVENT,               // APP或433遥控上锁事件
    FSM_APP_OR_RCU433_UNLOCK_EVENT,             // APP或433遥控解锁事件
    FSM_RCU433_ONEKEY_START_EVENT,              // 433遥控器闪电启动事件
    FSM_APP_OR_RCU433_SEARCH_EVENT,             // APP或433遥控寻车事件
    FSM_APP_OR_RCU433_CUSHION_UNLOCK_EVENT,     // APP或433遥控开坐垫锁事件
    FSM_MAIN_POWER_DOWN_EVENT,                  // 主电源掉电事件
    FSM_MAIN_POWER_ON_EVENT,                    // 主电源上电事件
    FSM_TRIGGER_ROTATION_EVENT,                 // 触发轮动事件
    FSM_RELEASE_ROTATION_EVENT,                 // 释放轮动事件
    FSM_TRIGGER_SPEEDING_EVENT,                 // 超速行驶事件
    FSM_RELEASE_SPEEDING_EVENT,                 // 超速解除事件
    FSM_MECH_KEY_UNLOCK_EVENT,                  // 机械钥匙开锁事件
    FSM_MECH_KEY_LOCK_EVENT,                    // 机械钥匙关锁事件
    FSM_MECH_ONEKEY_TRIG_EVENT,                 // 机械一键触发(START/STOP)事件
    FSM_APP_ONEKEY_TRIG_EVENT,                  // APP一键触发(START/STOP)事件
    FSM_EVENT_MAX,

}FSM_TRIG_EVENT_E, *PFSM_TRIG_EVENT_E;


/*********************************************************************************
使用方法：1.创建状态机 locdef_fsm_create()；
          2.程序轮询 locdef_dev_fsm_handle()运行状态机。
*********************************************************************************/
int locdef_fsm_create(uint8_t cur_state);

int locdef_dev_fsm_handle(uint8_t event, void *param);

int locdef_fsm_get_cur_state(void);



#ifdef __cplusplus
}
#endif
#endif
