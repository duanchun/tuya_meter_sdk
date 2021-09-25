

/**
****************************************************************************
* @file      tuya_kel_system.h
* @brief     tuya_kel_system
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_KEL_SYSTEM_H__
#define __TUYA_KEL_SYSTEM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

/**
* 接口名称: tuya_kel_sys_get_curr_time
* 描述:
*	获取系统运行的持续时间，单位ms，当时间达到858993456之后 会归零
* 参数:
*	无
* 返回值: 
*	成功：任务句柄 失败：NULL
*/
uint32_t tuya_kel_sys_get_curr_time(void);

__TUYA_WEAK__ void tuya_kel_system_delay_ms(uint32_t ms);

__TUYA_WEAK__ void tuya_kel_system_delay_us(uint32_t us);



#ifdef __cplusplus
}
#endif

#endif


