
/**
****************************************************************************
* @file      tuya_mid_power_mgr.h
* @brief     tuya_mid_power_mgr
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_MID_POWER_MGR_H__
#define __TUYA_MID_POWER_MGR_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

#if 0
typedef enum {
	MOD_FSM_PROC                    = (0X01 << 0),
	MOD_VFM_UPLOAD                  = (0X01 << 1),
	MOD_RUC433                      = (0X01 << 2),
	MOD_IO_FILTER_TIMER             = (0X01 << 3),
	MOD_ROTATION_DET_TIMER          = (0X01 << 4),
	MOD_LOCDEF_AUDIO_TIMER          = (0X01 << 5),
	MOD_WARING_RECOVER_TIMER        = (0X01 << 6)
}TUYA_POWER_MGR_SUB_MOD_E;
#endif

typedef enum {
	SYS_ACTIVE = 0,  
	SYS_HALF_SLEEP,   
	SYS_DEEP_SLEEP 
}TUYA_SYS_STATUS_E;


typedef enum {
	SYS_EVT_SUSPEND = 0,
	SYS_EVT_RESUME
}TUYA_LOW_PWR_EVT_E;

typedef enum{
	WAKEUP_ONE_KEY_START      = (0x01 << 0),
	WAKEUP_EDOOR              = (0x01 << 1),
	WAKEUP_SPEEDING           = (0x01 << 2),
	WAKEUP_ROTATION           = (0x01 << 3),
	WAKEUP_VIBRATE            = (0x01 << 4),
	WAKEUP_RCU433             = (0x01 << 5)
}TUYA_SYS_WAKEUP_SOURCE_E;

typedef void* tuya_power_mgr_handle;
typedef void (* tuya_power_mgr_cbk)(TUYA_LOW_PWR_EVT_E evt);

tuya_power_mgr_handle tuya_mid_pwr_mgr_register(tuya_power_mgr_cbk cbk);

void tuya_mid_sleep_enable(void);

void tuya_mid_sleep_disable(void);

TUYA_RET_E tuya_mid_wakeup_source_set(uint32_t flag);

uint32_t tuya_mid_wakeup_source_get(void);

void tuya_mid_wakeup_source_clear(uint32_t type);

void tuya_mid_wakeup_source_reset(void);

void tuya_mid_mod_sleep_active(tuya_power_mgr_handle sub_mod_handle);

void tuya_mid_mod_sleep_inactive(tuya_power_mgr_handle sub_mod_handle);

TUYA_RET_E tuya_mid_power_mgr_init(void);


#ifdef __cplusplus
}
#endif

#endif



