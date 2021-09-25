/**********************************Copyright (c)**********************************
**                       版权所有 (C), 2015-2021, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    vfm_fsm.h
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.09
 * @brief   该文件用于实现VFM状态机
 *
******************************************************************************/

#ifndef __VFM_FSM_H_
#define __VFM_FSM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vfm_base.h"

#define VFM_DEFAULT_WARNING_RECOVERY_TIME        (10)   // 默认报警恢复时间,单位s
#define VFM_DEFAULT_STATE_TYPE_NUMS              (6)    // 默认的状态类型数量

typedef void (*TY_VFM_FSM_ACTFUNC_CB)(void *);


typedef struct
{
    uint32_t next_state;              /* 跳转状态 */
    void (*action_func_cb)(void *); /* 动作函数 */
}VFM_FSM_SESSION_TABLE_T;

typedef struct
{
    uint32_t event;                   /* 触发事件 */
    uint32_t now_state;               /* 当前状态 */
    uint32_t next_state;              /* 跳转状态 */
    void (*user_func_cb)(void *);   /* 用户回调函数 */
}VFM_FSM_TABLE_T;


typedef struct
{
    VFM_FSM_TABLE_T *fsm_table;     /* 状态迁移表 */
    uint32_t table_nums;              /* 状态机状态迁移表数量 */
}VFM_FSM_T;


typedef struct
{
    uint32_t now_state;               /* 状态机当前状态 */
    uint32_t init_flag;               /* 初始化标志 */
    VFM_FSM_T *fsm;                 /* 状态机 */
}VFM_FSM_INS_T;


/** @enum FSM_STATE_E
 *  @brief 状态机运行状态
 *
 */
typedef enum
{
    FSM_ACCOFF_UNDEF    = 0x0001, // ACC_OFF撤防状态
    FSM_ACCOFF_DEF      = 0x0002, // ACC_OFF设防状态
    FSM_ACCON           = 0x0004, // ACC_ON状态
    FSM_MOVING          = 0x0008, // 行驶状态
    FSM_WARNING         = 0x0010, // 报警状态
    /*
    可供开发者扩展自定义状态,必须按bit使用0x0020,0x0040,0x0080,依次类推
    */
    FSM_HOLD_STATE      = VFM_FSM_STATE_MAX, // 状态保持,处理状态不变的情况
}VFM_FSM_STATE_E;



/*********************************************************************************
使用方法：1.注册状态机 ty_vfm_fsm_register()；
          2.程序轮询 ty_vfm_fsm_event_proc()运行状态机。
*********************************************************************************/
/* 注册状态机      */
int ty_vfm_fsm_register(VFM_FSM_T *pfsm, uint32_t now_state);

/* 注销状态机 */
int ty_vfm_fsm_deregister(VFM_FSM_T *pfsm);

/* 状态机事件处理函数 */
int ty_vfm_fsm_event_proc(uint32_t event, void *param);

/* 获取状态机当前状态 */
int ty_vfm_fsm_get_now_state(uint32_t* now_state, uint32_t* index);

/* 设置报警恢复时间 */
int ty_vfm_set_warning_recovery_time(uint32_t time_s);



#ifdef __cplusplus
}
#endif
#endif

