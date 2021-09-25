
/**
****************************************************************************
* @file      tuya_type.h
* @brief     tuya_type
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_TYPE_H__
#define __TUYA_TYPE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_ble_type.h"

typedef enum {
	TUYA_OK = 0,
	TUYA_INVALID_PARAMS = 1,
	TUYA_COMMON_ERROR
}TUYA_RET_E;

#ifndef __TUYA_WEAK__ 
#define __TUYA_WEAK__    __weak
#endif

#ifdef __cplusplus
}
#endif

#endif


