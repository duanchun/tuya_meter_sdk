

/**
****************************************************************************
* @file      tuya_log.h
* @brief     tuya_log
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TY_LOG_H__
#define __TY_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MOD_PUBLIC_LOG_ENABLE    0
#define MOD_AUDIO_LOG_ENABLE     0
#define MOD_NV_LOG_ENABLE        0
#define MOD_OS_LOG_ENABLE        1
#define MOD_AUDIOE_LOG_ENABLE    1
#define MOD_BATTERY_LOG_ENABLE   0
#define MOD_DEV_LOG_ENABLE       0
#define MOD_TIMER_LOG_ENABLE     0
#define MOD_FSM_LOG_ENABLE       1
#define MOD_APP_LOG_ENABLE       0
#define MOD_DP_LOG_ENABLE        0
#define MOD_HID_LOG_ENABLE       0
#define MOD_PERIPH_LOG_ENABLE    0
#define MOD_SERVER_LOG_ENABLE    1
#define MOD_SENSOR_LOG_ENABLE    0
#define MOD_RCU433_LOG_ENABLE    0
#define MOD_POWER_MGR_LOG_ENABLE 1


typedef enum {
	MOD_PUBLIC = 0,
	MOD_AUDIO = 1,
	MOD_NV = 2,
	MOD_OS = 3,
	MOD_AUDIOE = 4,
	MOD_BATTERY = 5,
	MOD_DEV = 6,
	MOD_TIMER = 7,
	MOD_FSM = 8,
	MOD_APP = 9,
	MOD_DP = 10,
	MOD_HID = 11,
	MOD_PERIPH = 12,
	MOD_SERVER = 13,
	MOD_SENSOR = 14,
	MOD_RCU433 = 15,
	MOD_POWER_MGR = 16,
	MOD_MAX
}TY_LOG_MODULE_E;

typedef enum {
	TY_LOG_LVL_NONE = 0,
    TY_LOG_LVL_INFO = (0X01<<0),
    TY_LOG_LVL_WORRING= (0X01<<1),
    TY_LOG_LVL_ERROR= (0X01<<2)
}TY_LOG_LVL_E;

/****************************ty log nv default value set { ************************************/

#if (0 == MOD_PUBLIC_LOG_ENABLE)
#define MOD_PUBLIC_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_PUBLIC_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_AUDIO_LOG_ENABLE)
#define MOD_AUDIO_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_AUDIO_LEVEL        (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if ( 0 == MOD_NV_LOG_ENABLE)
#define MOD_NV_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_NV_LEVEL       (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_OS_LOG_ENABLE)
#define MOD_OS_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_OS_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_AUDIOE_LOG_ENABLE)
#define MOD_AUDIOE_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_AUDIOE_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if ( 0 == MOD_BATTERY_LOG_ENABLE)
#define MOD_BATTERY_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_BATTERY_LEVEL      (TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR )
#endif

#if (0 == MOD_DEV_LOG_ENABLE)
#define MOD_DEV_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_DEV_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_TIMER_LOG_ENABLE)
#define MOD_TIMER_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_TIMER_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_FSM_LOG_ENABLE)
#define MOD_FSM_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_FSM_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_APP_LOG_ENABLE)
#define MOD_APP_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_APP_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_DP_LOG_ENABLE)
#define MOD_DP_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_DP_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_HID_LOG_ENABLE)
#define MOD_HID_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_HID_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_PERIPH_LOG_ENABLE)
#define MOD_PERIPH_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_PERIPH_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_SERVER_LOG_ENABLE)
#define MOD_SERVER_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_SERVER_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_SENSOR_LOG_ENABLE)
#define MOD_SENSOR_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_SENSOR_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_RCU433_LOG_ENABLE)
#define MOD_RCU433_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_RCU433_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif

#if (0 == MOD_POWER_MGR_LOG_ENABLE)
#define MOD_POWER_MGR_LEVEL    TY_LOG_LVL_NONE
#else
#define MOD_POWER_MGR_LEVEL      (TY_LOG_LVL_INFO | TY_LOG_LVL_WORRING | TY_LOG_LVL_ERROR)
#endif



/****************************ty log nv default value set } ************************************/


extern void ty_core_log(TY_LOG_MODULE_E mod_id,const char *func_name,unsigned int line, TY_LOG_LVL_E level, const char* format,...);

#define TY_DEBUG_E
#define TY_DEBUG_W
#define TY_DEBUG_I
	
#ifdef TY_DEBUG_E
    #define TUYA_LOG_E(mod_id,fmt, ...) ty_core_log( mod_id,__FUNCTION__,__LINE__,TY_LOG_LVL_ERROR,fmt, ##__VA_ARGS__)
#else
    #define TUYA_LOG_E(mod_id,fmt, ...)
#endif
	
#ifdef TY_DEBUG_W   
    #define TUYA_LOG_W(mod_id,fmt, ...) ty_core_log( mod_id,__FUNCTION__,__LINE__,TY_LOG_LVL_WORRING,fmt, ##__VA_ARGS__)
#else
    #define TUYA_LOG_W(mod_id,fmt, ...)
#endif
	
#ifdef TY_DEBUG_I   
    #define TUYA_LOG_I(mod_id,fmt, ...) ty_core_log( mod_id,__FUNCTION__,__LINE__,TY_LOG_LVL_INFO,fmt, ##__VA_ARGS__)
#else
    #define TUYA_LOG_I(mod_id,fmt, ...)
#endif


#ifdef __cplusplus
}
#endif

#endif


