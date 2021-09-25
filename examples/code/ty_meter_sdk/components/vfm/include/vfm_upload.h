/**********************************Copyright (c)**********************************
**                       版权所有 (C), 2015-2021, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    vfm_upload.h
 * @author  shanming
 * @version v0.0.1
 * @date    2021.08.09
 * @brief
 *
******************************************************************************/

#ifndef __VFM_UPLOAD_H_
#define __VFM_UPLOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vfm_base.h"
#include "vfm_log.h"

/************************************ DP { **********************/

#define PROP_BOOL 0
#define PROP_VALUE 1
#define PROP_STR 2
#define PROP_ENUM 3
#define PROP_BITMAP 4

typedef union {
    int dp_value;             // valid when dp type is value
    uint32_t dp_enum;             // valid when dp type is enum
    char *dp_str;             // valid when dp type is str
    int dp_bool;             // valid when dp type is bool
    uint32_t dp_bitmap;           // valid when dp type is bitmap
}TY_OBJ_DP_VALUE_U;
typedef struct {
    uint8_t dpid;                // dp id
    unsigned char type;          // dp type
    TY_OBJ_DP_VALUE_U value;    // dp value
    uint32_t time_stamp;          // dp happen time. if 0, mean now
}TY_OBJ_DP_S;
/************************************ DP } **********************/

typedef struct {
    // DP ID
    uint16_t dpid;
    // DP功能
    uint16_t fun;
    // 周期时间 单位ms 最小间隔100ms(值小于100 无效) 0:关闭上传当前事件DP
    uint32_t timeval;
} VFM_DC_STAT_T;

typedef struct {
    // DP ID
    uint16_t dpid;
    // DP功能
    uint16_t fun;
    // TRUE：上传当前事件DP  FALSE: 关闭上传当前事件DP
    int en;
} VFM_DC_EVT_T;

typedef struct
{
    // 统计上传表
    VFM_DC_STAT_T *st_table;
    // 事件上传表
    VFM_DC_EVT_T  *evt_table;
    // 批量事件上传表 状态切换
    VFM_DC_EVT_T  *multi_evt_table;
    // 周期统计上传表事件组数
    uint8_t st_nums;
    // 事件上传表事件组数
    uint8_t evt_nums;
    // 单次批量事件表事件组数
    uint8_t multi_nums;
    // 状态机状态数量 对应上传表的列数
    uint8_t state_nums;
}VFM_UPLOAD_CFG_T;

typedef enum{
    DP_FUN_STEERING_LOCK = VFM_FSM_STATE_MAX+1,   /*龙头锁*/
    DP_FUN_CUSHION_LOCK,	  /*坐筒锁*/
    DP_FUN_MOTOR_LOCK,	    /*电机锁*/
    DP_FUN_DEF_LOCK,        /*布防锁*/
    DP_FUN_USER_LOCK,	      /*用户自定义*/
    DP_FUN_SPEED,           /*速度*/
    DP_FUN_MILE,            /*单次里程*/
    DP_FUN_TOTALMILE,       /*总里程*/
    DP_FUN_SENSOR_ERROR,	  /*sensor故障码*/
    DP_FUN_ACC,			        /*acc状态*/
    DP_FUN_GNSS_DEV_STAUTS, /*gnss设备状态*/
    DP_FUN_GNSS_STATUS,     /*定位状态*/
    DP_FUN_GNSS_SNR,	    	/*载噪比*/
    DP_FUN_GNSS_POSTION,	  /*定位数据*/
    DP_FUN_GNSS_SPEED,	    /*定位速度*/
    DP_FUN_GNSS_COURSE,	    /*航向*/
    DP_FUN_GNSS_HDOP,	    	/*水平精度*/
    DP_FUN_CELL_SNR,	    	/*cell 载噪比*/
    DP_FUN_LBS,			        /*lbs信息*/
    DP_FUN_WIFI,			      /*wifi ap信息*/
    DP_FUN_INTER_RSOC,	    /*内部剩余电量百分比*/
    DP_FUN_INTER_VOLTAGE,   /*内部电池电压*/
    DP_FUN_INTER_CURRENT,   /*内部电池电流*/
    DP_FUN_INTER_TEMP,	    /*内部电池温度*/
    DP_FUN_EXT_RSOC,	 	    /*外部剩余电量百分比*/
    DP_FUN_EXT_VOLTAGE,	    /*外部电池电压*/
    DP_FUN_EXT_CURRENT,	    /*外部电池电流*/
    DP_FUN_EXT_TEMP,    	 	/*外部电池温度*/
    DP_FUN_MAX,	 	          /*MAX*/
}VFM_DP_FUN;

/* 上传表定义示例
// 上传表由事件组组成 相邻事件组在表用空行隔开 事件组的INDEX行 代表该事件在INDEX状态下的处理配置
// 用户配置 可供开发者扩展自定义状态 若增加状态机状态 须按FSM INDEX顺序在每个事件组INDEX处理增加处配置

// 周期上传配置表 对应状态下事件DP会被循环上传
STATIC VFM_DC_STAT_T s_vfm_dc_st_table[] = {
  // FSM_ACCOFF_UNDEF  (状态INDEX)
  {DPID_GPS_POSITION,  DP_FUN_GNSS_POSTION,     0},
  // FSM_MOVING
  {DPID_GPS_POSITION,  DP_FUN_GNSS_POSTION,     30000},
  // FSM_ACCOFF_DEF
  {DPID_GPS_POSITION,  DP_FUN_GNSS_POSTION,     30000},
  // FSM_ACCON
  {DPID_GPS_POSITION,  DP_FUN_GNSS_POSTION,     30000},
  // FSM_WARNING
  {DPID_GPS_POSITION,  DP_FUN_GNSS_POSTION,     3600},
  // 若VFM FSM增加一个状态
  // {DPID_GPS_POSITION,  DP_FUN_GNSS_POSTION,     3600},

  {DPID_GPS_POSITION,  DP_FUN_LBS,              0},
  {DPID_GPS_POSITION,  DP_FUN_LBS,              30000},
  {DPID_GPS_POSITION,  DP_FUN_LBS,              30000},
  {DPID_GPS_POSITION,  DP_FUN_LBS,              30000},
  {DPID_GPS_POSITION,  DP_FUN_LBS,              3600},
  // 若VFM FSM增加一个状态
  // {DPID_GPS_POSITION,  DP_FUN_LBS,              3600},

  {DPID_GPS_POSITION,  DP_FUN_WIFI,             0},
  {DPID_GPS_POSITION,  DP_FUN_WIFI,             30000},
  {DPID_GPS_POSITION,  DP_FUN_WIFI,             30000},
  {DPID_GPS_POSITION,  DP_FUN_WIFI,             30000},
  {DPID_GPS_POSITION,  DP_FUN_WIFI,             3600},
  // 若VFM FSM增加一个状态
  // {DPID_GPS_POSITION,  DP_FUN_WIFI,             3600},
};

// 状态事件配置表 对应状态下收到对应事件 会上传对应事件DP
STATIC VFM_DC_EVT_T s_vfm_dc_evt_table[] = {
  {7,   DP_FUN_ACC,     TRUE},
  {7,   DP_FUN_ACC,     TRUE},
  {7,   DP_FUN_ACC,     TRUE},
  {7,   DP_FUN_ACC,     TRUE},
  {7,   DP_FUN_ACC,     TRUE},
  // 若VFM FSM增加一个状态
  // {7,   DP_FUN_ACC,     TRUE},

  {8,   DP_FUN_MILE,    TRUE},
  {8,   DP_FUN_MILE,    TRUE},
  {8,   DP_FUN_MILE,    TRUE},
  {8,   DP_FUN_MILE,    TRUE},
  {8,   DP_FUN_MILE,    TRUE},
  // 若VFM FSM增加一个状态
  // {8,   DP_FUN_MILE,    TRUE},

  {9,   DP_FUN_SPEED,   TRUE},
  {9,   DP_FUN_SPEED,   TRUE},
  {9,   DP_FUN_SPEED,   TRUE},
  {9,   DP_FUN_SPEED,   TRUE},
  {9,   DP_FUN_SPEED,   TRUE},
  // 若VFM FSM增加一个状态
  // {9,   DP_FUN_SPEED,   TRUE},
};

// 状态切换单次批量上传配置表 当前状态下批量上传配置事件DP
STATIC VFM_DC_EVT_T s_vfm_dc_multi_evt_table[] = {
  {4,   DP_FUN_GNSS_POSTION,    TRUE},
  {4,   DP_FUN_GNSS_POSTION,    TRUE},
  {4,   DP_FUN_GNSS_POSTION,    TRUE},
  {4,   DP_FUN_GNSS_POSTION,    TRUE},
  {4,   DP_FUN_GNSS_POSTION,    TRUE},
  // 若VFM FSM增加一个状态
  // {4,   DP_FUN_GNSS_POSTION,    TRUE},

  {7,   DP_FUN_ACC,             TRUE},
  {7,   DP_FUN_ACC,             TRUE},
  {7,   DP_FUN_ACC,             FALSE},
  {7,   DP_FUN_ACC,             TRUE},
  {7,   DP_FUN_ACC,             TRUE},
  // 若VFM FSM增加一个状态
  // {7,   DP_FUN_ACC,             TRUE},

  {8,   DP_FUN_MILE,            FALSE},
  {8,   DP_FUN_MILE,            FALSE},
  {8,   DP_FUN_MILE,            TRUE},
  {8,   DP_FUN_MILE,            TRUE},
  {8,   DP_FUN_MILE,            TRUE},
  // 若VFM FSM增加一个状态
  // {8,   DP_FUN_MILE,            TRUE},

  {9,   DP_FUN_SPEED,           TRUE},
  {9,   DP_FUN_SPEED,           TRUE},
  {9,   DP_FUN_SPEED,           TRUE},
  {9,   DP_FUN_SPEED,           TRUE},
  {9,   DP_FUN_SPEED,           FALSE},
  // 若VFM FSM增加一个状态
  // {9,   DP_FUN_SPEED,           FALSE},
};
*/

// VFM上传注册接口
int ty_vfm_upload_register(VFM_UPLOAD_CFG_T *pcfg);
// 上报事件接口
int ty_vfm_upload_event_send(VFM_DP_FUN dpFun);

#ifdef __cplusplus
}
#endif
#endif
