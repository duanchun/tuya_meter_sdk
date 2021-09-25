/**                       版权所有 (C), 2015-2021, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    locdef_config.h
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.12
 * @brief   该文件用于实现定防器参数配置管理
 *
******************************************************************************/

#ifndef __LOCDEF_CONFIG_H_
#define __LOCDEF_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_type.h"
#include "tuya_mid_nv.h"

#define READ_DEFAULT_CONFIG_PARAM     TRUE

#define DEV_CFG_PARAM                       3 //"dev_cfg_param"

/******************************************************************************
 *                      json配置字段
 * ***************************************************************************/
#define LIMIT_SPEED_ALARM_INTERVAL          "limit_speed_alarm_interval"
#define GPS_REPORT_INTERVAL                 "gps_report_interval"
#define LIMIT_SPEED_ALARM                   "limit_speed_alarm"
#define VOLTAGE_LOWER_LIMIT                 "voltage_lower_limit"
#define BATTERY_CAPACITY                    "battery_capacity"
#define WHEEL                               "wheel"
#define VOLTAGE_UPPER_LIMIT                 "voltage_upper_limit"


tuya_dev_cfg_t * locdef_get_dev_cfg_param(void);

TUYA_RET_E locdef_dev_save_dev_cfg_param(void);

TUYA_RET_E locdef_param_init(void);





#ifdef __cplusplus
}
#endif
#endif


