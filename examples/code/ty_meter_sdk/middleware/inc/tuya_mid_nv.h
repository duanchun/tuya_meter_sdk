

/**
****************************************************************************
* @file      tuya_mid_nv.h
* @brief     tuya_mid_nv
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_MID_NV_H__
#define __TUYA_MID_NV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_log.h"
#include "tuya_type.h"

#define TUYA_NVRAM_FLASH_START_ADDR      (0x75000)


typedef struct {
	/* json配置参数 */
	uint32_t    limit_speed_alarm_interval;		  //超速报警声音间隔
	uint32_t    gps_report_interval;			  //gps上报间隔开机时
	uint32_t    wheel;							  //轮径
	float		voltage_lower_limit;			  //电压下限
	float		voltage_upper_limit;			  //电压上限
	uint16_t    battery_capacity;				  //电池容量
	int32_t 	limit_speed_alarm; 				  //超速报警开关
	/* 设备运行参数 */
	uint32_t	fsm_run_state;					  //状态机运行状态
	uint8_t 	self_recovery_flag; 			  //设备自恢复标志
	uint8_t 	network_reconn_cnt; 			  //网络重连次数
}tuya_dev_cfg_t;

typedef struct {
	uint8_t addr_flag;
	uint8_t addr[3];
}rcu_addr_t;

typedef enum {
	TUYA_NVRAM_DEV_CONFIG_LID = 0,
	TUYA_NVRAM_RCU433_LID,
	TUYA_NVRAM_MAX_LID
}TUYA_NVRAM_LID_E;


#define TUYA_NVRAM_DEV_CONFIG_LID_SIZE              (sizeof(tuya_dev_cfg_t))
#define TUYA_NVRAM_DEV_CONFIG_LID_TOTAL             1
#define TUYA_NVRAM_DEV_CONFIG_LID_VERNO             "000"
	
#define TUYA_NVRAM_RCU433_LID_SIZE                  (sizeof(rcu_addr_t))
#define TUYA_NVRAM_RCU433_LID_TOTAL                 2
#define TUYA_NVRAM_RCU433_LID_VERNO                 "000"

void tuya_mid_nv_init(void);
TUYA_RET_E tuya_mid_nv_write(TUYA_NVRAM_LID_E nLID, uint16_t nRecordId, void *pBuffer, uint16_t nBufferSize, int *pError);
TUYA_RET_E tuya_mid_nv_read(TUYA_NVRAM_LID_E nLID, uint16_t nRecordId, void *pBuffer, uint16_t nBufferSize, int *pError);

#ifdef __cplusplus
}
#endif

#endif



