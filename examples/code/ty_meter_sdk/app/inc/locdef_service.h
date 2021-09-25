/**********************************Copyright (c)**********************************
**                       版权所有 (C), 2015-2021, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    locdef_service.h
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.12
 * @brief   该文件用于实现定防器业务
 *
******************************************************************************/

#ifndef __LOCDEF_SERVICE_H_
#define __LOCDEF_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "locdef_hid.h"


#define GPS_INFO_MAX_LEN                    24
#define LBS_INFO_MAX_LEN                    64
#define TY_GNSS_TIMEOUT                     2000

#define SELF_RECOVERY_POLLING_PERIOD        (1000*60*1)         //设备自恢复轮询周期,1min
#define SELF_RECOVERY_PERIOD                (1000*60*60*24*3)   //设备自恢复周期3Days,单位ms
#define SELF_RECOVERY_DELAY_PERIOD          (1000*60*60)        //设备自恢复超时延时周期1hour,单位ms
#define STATIC_PARK_TIMEOUT                 (1000*60*20)        //静止停车20min,单位ms
#define LOCDEF_ROTATION_REPORT_PULSE_CNT    10                  //轮动上报脉冲数
#define LOCDEF_ROTATION_ALARM_PULSE_CNT     5//40  TODO                //轮动报警脉冲数
#define ROUND_PULSE_NUMS                    25                  //1圈脉冲数
#define ROTATION_DET_PERIOD                 (500)               //轮动检测窗口500ms
#define ALARM_ROTATION_RELEASE_TIMES        (1000*5)            //报警轮动释放时间5s
#define TRAVEL_ROTATION_RELEASE_TIMES       (1000*1)            //行驶轮动释放时间1s
#define DATAPOINT_REPORT_INTERVEL           200                 //200ms
#define GPS_START_TIMEOUT_INTERVEL          (1000*60*5)         //GPS启动超时检测5min
#define PLAYER_TIMEOUT_INTERVEL             10*1000             //10秒
#define GPS_DEFAULT_REPORT_INTERVAL         60*60               //1小时
#define GPS_ALARM_MSG_REPORT_INTERVAL       (1000*30*1)         //GPS报警信息检测时间30s

#define FSM_TASK_STACK_SIZE                 (1024*4)            //状态机任务大小
#define FSM_TASK_PRIORITY                   TRD_PRIO_1          //状态机任务优先级
#define BASE_TASK_STACK                     (1024*3)            //基础任务栈大小
#define BASE_TASK_PRIO                      TRD_PRIO_2          //基础复任务优先级
#define DP_UPDATE_TASK_STACK                (1024*3)            //DP更新任务栈大小
#define DP_UPDATE_TASK_PRIO                 TRD_PRIO_3          //DP更新任务优先级
#define SELF_REC_TASK_STACK                 (1024*3)            //自恢复任务栈大小
#define SELF_REC_TASK_PRIO                  TRD_PRIO_4          //自恢复任务优先级

#define VIB_DET_TIME_WINDOW                 (200)               //振动检测时间窗口,单位ms
#if 1
#define VIB_SENSOR_SENSITIVITY              (100)            //振动传感器灵敏度,越小越灵敏
#else
#define VIB_SENSOR_SENSITIVITY              (30)             //振动传感器灵敏度,越小越灵敏
#endif

typedef enum {
    ADC_SCALE_1V250 = 0,
    ADC_SCALE_2V444,
    ADC_SCALE_3V233,
    ADC_SCALE_5V000,

}ADC_CFG_E;

typedef enum {
    RSSI_CUT,                                        //0-5
    RSSI_BAD,                                        //6-11
    RSSI_GENGRAL,                                    //12-17
    RSSI_GOOD,                                       //18-24
    RSSI_GREAT,                                      //25-31

}RSSI_LEVEL_E;

typedef enum {
    COMMON_SENSITIVITY = 5,
    RSSI_BAD_LOWER_LIMIT = 6,
    RSSI_GENGRAL_LOWER_LIMIT = 12,
    RSSI_GOOD_LOWER_LIMIT = 18,
    RSSI_GREAT_LOWER_LIMIT = 24,

}CELL_SNR_LEVEL_E;


typedef enum {
    DEVICE_NORMAL = 0x00,
    DEVICE_FAULT,
    DEVICE_CHARGING,
    DEVICE_CHARGE_DONE,
    DEVICE_ALARM
}STATUS_E;


typedef enum {
    ROTATION_TRI = 0x1,
    EDOOR_LOCK_TRI,
    EDOOR_LOCK_O_TRI,
}EDOOR_TRI_E;


typedef struct {
    uint32_t          speed;                          //速度：gps or 轮动
    float         single_mileage;                 //单次里程: 单位KM
    uint32_t         parking_time;                   //停车时间,单位ms
    int          gps_led_status;                 //gps指示灯状态
    uint8_t         alarm_enable;                   //防盗(振动)使能位: 1 开启; 0 关闭
    uint8_t         rotation_irq_en;                //轮动检测中断使能位：1 打开; 0 关闭
    uint8_t         rotation_det_cplt;              //轮动检测完成标志：1 完成; 0 未完成
    uint8_t         cushion_lock_pulse_en;          //坐垫锁脉冲使能：1 使能; 0 关闭
    uint8_t         dev_status;                     //设备状态:normal device_fault alarm
    uint16_t        rotation_alarm_cnt;             //轮动报警计数
    ty_dev_hid_t    dev_hid;
	uint8_t         hid_dev_bond;//1:unbond,0:bond(ty_hid_dev_bind_e)
	uint8_t         hid_svc_change_on;

}DEV_RUNNING_PARAM_T;

typedef enum
{
    APP_LIGHT_VERYSLOW_FLASH    = 3000,
    APP_LIGHT_SLOW_FLASH        = 2000,
    APP_LIGHT_NOMAL_FLASH       = 1000,
    APP_LIGHT_FAST_FLASH        = 300,
    APP_LIGHT_VERYFAST_FLASH    = 100,
    APP_LIGHT_ALAWYS_ON         = 1,
    APP_LIGHT_ALAWYS_OFF        = 0,
}APP_LIGHT_STATUS_e;




void locdef_vib_alarm_ctrl(int enable);
int locdef_vib_alarm_enable(void);


DEV_RUNNING_PARAM_T * locdef_get_run_param(void);

void locdef_charge_enable(int enable);

void locdef_cushion_lock(void);
uint32_t locdef_get_speed(void);
float locdef_get_single_mileage(void);

/* 滤波算法接口 */
void bubble_sort_f(float *data, uint32_t len);
float median_filter_f(float *data, uint32_t len);
uint16_t get_zero_value_f(float value_buf[], uint16_t n);
int variance_calc(const float value_buf[], uint16_t n, float *variance);
float smoothing_filter_f(float value_buf[], uint16_t n, uint16_t *filter_index, float now_value);


#ifdef __cplusplus
}
#endif
#endif

