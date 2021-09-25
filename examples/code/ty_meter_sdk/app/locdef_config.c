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
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS  THE SOFTWARE.
 */
/**
 * @file    locdef_config.c
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.12
 * @brief   该文件用于实现定防器参数配置管理
 *
******************************************************************************/

#include "locdef_config.h"
#include "vfm_base.h"
#include "vfm_fsm.h"
#include "string.h"
#include "cJSON.h"
#include "tuya_log.h"



#define DEV_CFG_INFO_BY_JSON "{\"limit_speed_alarm_interval\":10,\"gps_report_interval\":5,\
                               \"limit_speed_alarm\":1,\"voltage_lower_limit\":42,\
                               \"battery_capacity\":20,\"wheel\":14,\"voltage_upper_limit\":54.2}"

tuya_dev_cfg_t g_cfg_para = { 0 };


/**
 * @brief   获取设备运行参数
 *
 * @return  无
 */
tuya_dev_cfg_t * locdef_get_dev_cfg_param(void)
{
    return &g_cfg_para;
}


/**
 * @brief   保存设备配置参数
 *
 * @return  无
 */
TUYA_RET_E locdef_dev_save_dev_cfg_param(void)
{
    TUYA_RET_E opt_ret = 0;

    opt_ret = tuya_mid_wd_common_write(DEV_CFG_PARAM, (uint8_t*)&g_cfg_para, sizeof(tuya_dev_cfg_t));

    return opt_ret;
}

/**
 * @brief   恢复出厂设备配置参数
 *
 * @return  无
 */
static void locdef_dev_cfg_param_restore(void)
{
    g_cfg_para.limit_speed_alarm_interval = 10;        //超速报警声音间隔
    g_cfg_para.gps_report_interval = 5;                //gps上报间隔开机时
    g_cfg_para.wheel = 14;                             //轮径英寸
    g_cfg_para.voltage_lower_limit = 37.5;             //电压下限
    g_cfg_para.battery_capacity = 20;                  //电池容量
    g_cfg_para.voltage_upper_limit = 54.75; //电压上限
    g_cfg_para.limit_speed_alarm = 1;                  //超速报警开关
    g_cfg_para.fsm_run_state = FSM_ACCOFF_DEF;         //状态机运行状态
    g_cfg_para.self_recovery_flag = false;             //设备自恢复标志
    g_cfg_para.network_reconn_cnt = 0;                 //网络重连次数

    locdef_dev_save_dev_cfg_param();
}

/**
 * @brief   设置参数by json
 *
 * @param   void
 *
 * @return  无
 */
static TUYA_RET_E locdef_set_param_by_json( cJSON *root)
{
    TUYA_RET_E op_ret = TUYA_OK;

    cJSON *json = NULL;
    json = cJSON_GetObjectItem(root, LIMIT_SPEED_ALARM_INTERVAL);
    #if (!READ_DEFAULT_CONFIG_PARAM)
    if (json) {
        json = cJSON_GetObjectItem(json, "value");
    }
    #endif
    if (json == NULL) {
        goto TY_CJSON_DELETE_ROOT;
    }
    g_cfg_para.limit_speed_alarm_interval = json->valueint;
    json = cJSON_GetObjectItem(root, GPS_REPORT_INTERVAL);
    #if (!READ_DEFAULT_CONFIG_PARAM)
    if (json) {
        json = cJSON_GetObjectItem(json, "value");
    }
    #endif
    if (json == NULL) {
        goto TY_CJSON_DELETE_ROOT;
    }
    g_cfg_para.gps_report_interval = json->valueint;
    json = cJSON_GetObjectItem(root, LIMIT_SPEED_ALARM);
    #if (!READ_DEFAULT_CONFIG_PARAM)
    if (json) {
        json = cJSON_GetObjectItem(json, "value");
    }
    #endif
    if (json == NULL) {
        goto TY_CJSON_DELETE_ROOT;
    }
    g_cfg_para.limit_speed_alarm = json->valueint;
    json = cJSON_GetObjectItem(root, VOLTAGE_LOWER_LIMIT);
    #if (!READ_DEFAULT_CONFIG_PARAM)
    if (json) {
        json = cJSON_GetObjectItem(json, "value");
    }
    #endif
    if (json == NULL) {
        goto TY_CJSON_DELETE_ROOT;
    }
    g_cfg_para.voltage_lower_limit = json->valuedouble;
    json = cJSON_GetObjectItem(root, BATTERY_CAPACITY);
    #if (!READ_DEFAULT_CONFIG_PARAM)
    if (json) {
        json = cJSON_GetObjectItem(json, "value");
    }
    #endif
    if (json == NULL) {
        goto TY_CJSON_DELETE_ROOT;
    }
    g_cfg_para.battery_capacity = json->valueint;
    json = cJSON_GetObjectItem(root, WHEEL);
    #if (!READ_DEFAULT_CONFIG_PARAM)
    if (json) {
        json = cJSON_GetObjectItem(json, "value");
    }
    #endif
    if (json == NULL) {
        goto TY_CJSON_DELETE_ROOT;
    }
    g_cfg_para.wheel = json->valueint;
    json = cJSON_GetObjectItem(root, VOLTAGE_UPPER_LIMIT);
    #if (!READ_DEFAULT_CONFIG_PARAM)
    if (json) {
        json = cJSON_GetObjectItem(json, "value");
    }
    #endif
    if (json == NULL) {
        goto TY_CJSON_DELETE_ROOT;
    }
    g_cfg_para.voltage_upper_limit = json->valuedouble;
    TUYA_LOG_I(MOD_PUBLIC," json cfg_parm:%d-%d-%d-%d-%.3f-%.3f-%d",g_cfg_para.battery_capacity,g_cfg_para.gps_report_interval,\
                                    g_cfg_para.limit_speed_alarm,g_cfg_para.limit_speed_alarm_interval,\
                                    g_cfg_para.voltage_lower_limit,g_cfg_para.voltage_upper_limit,g_cfg_para.wheel);

    TUYA_LOG_I(MOD_PUBLIC," run_param: run_state(0x%x)-recovery(%d)-network_reconn%d", g_cfg_para.fsm_run_state, g_cfg_para.self_recovery_flag, g_cfg_para.network_reconn_cnt);

    return op_ret;
TY_CJSON_DELETE_ROOT:
    TUYA_LOG_I(MOD_PUBLIC,"param is no correct");

    return TUYA_COMMON_ERROR;
}

TUYA_RET_E locdef_wheel_diamete_update(uint8_t wheel_enum)
{
	switch(wheel_enum)
	{
		case 0:
			g_cfg_para.wheel = 10;
			break;
		case 1:
			g_cfg_para.wheel = 12;
			break;
		case 2:
			g_cfg_para.wheel = 14;
			break;
		case 3:
			g_cfg_para.wheel = 17;
			break;
		default:
			g_cfg_para.wheel = 14;
			break;
	}

	locdef_dev_save_dev_cfg_param();
	return TUYA_OK;
}

uint8_t locdef_wheel_diamete_get(void)
{
	uint8_t wheel_enum = 0;
	switch(g_cfg_para.wheel)
	{
		case 10:
			wheel_enum = 0;
			break;
		case 12:
			wheel_enum = 1;
			break;
		case 14:
			wheel_enum = 2;
			break;
		case 17:
			wheel_enum = 3;
			break;
		default:
			wheel_enum = 2;
			break;
	}

	return wheel_enum;
}



/**
 * @brief   配置参数初始化
 *
 * @param   void
 *
 * @return  无
 */
TUYA_RET_E locdef_param_init(void)
{
    cJSON *js_param = NULL;
    TUYA_RET_E op_ret = 0;
    uint32_t buff_len = 0;
    uint8_t *buff = NULL;

#if (READ_DEFAULT_CONFIG_PARAM)
    js_param = cJSON_Parse(DEV_CFG_INFO_BY_JSON);
#else
    char *pConfig = NULL;
    uint32_t len = 0;
    op_ret = ty_mid_wd_user_param_read((uint8_t**)&pConfig, &len);
    if (TUYA_OK != op_ret) {
        TUYA_LOG_E(MOD_PUBLIC," ws_db_get_user_param op_ret:%d", op_ret);
    }
    if (pConfig){
        TUYA_LOG_I(MOD_PUBLIC," pConfig: %s len:%d", pConfig, len);
    }
    js_param = cJSON_Parse(pConfig);
    tuya_ble_free(pConfig);
    pConfig = NULL;
#endif

    op_ret = tuya_mid_wd_common_read(DEV_CFG_PARAM, &buff, &buff_len);
    if (buff) {
		
        memcpy((uint8_t *)&g_cfg_para, buff, sizeof(tuya_dev_cfg_t));
        tuya_ble_free(buff);
        buff = NULL;
    } else {
        /* 保存默认出厂参数 */
        locdef_dev_cfg_param_restore();
		TUYA_LOG_I(MOD_PUBLIC," save default cfg param");
    }

    if (NULL != js_param) {
        #if (!READ_DEFAULT_CONFIG_PARAM)
        js_param = cJSON_GetObjectItem(js_param, "sw");
        if (js_param) {
           js_param = cJSON_GetObjectItem(js_param, "DEV_CFG_INFO_BY_JSON");
        }
        #endif
        if (js_param){
            op_ret = locdef_set_param_by_json(js_param);
            if (TUYA_OK != op_ret) {
                TUYA_LOG_E(MOD_PUBLIC," op_ret:%d", op_ret);
            }
        }
    } else {
        TUYA_LOG_E(MOD_PUBLIC," cJSON_Parse err %d!",js_param);
        op_ret = TUYA_COMMON_ERROR;
    }

    cJSON_Delete(js_param);
    js_param = NULL;

    return op_ret;
}

