
/**
****************************************************************************
* @file      tuya_mid_dev.h
* @brief     tuya_mid_dev
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_MID_DEV_H__
#define __TUYA_MID_DEV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

TUYA_RET_E tuya_mid_wd_common_read(const uint8_t key_id, uint8_t **value, uint32_t *len);
TUYA_RET_E tuya_mid_wd_common_write(const uint8_t key_id, const uint8_t *value, const uint32_t len);


#ifdef __cplusplus
}
#endif

#endif

