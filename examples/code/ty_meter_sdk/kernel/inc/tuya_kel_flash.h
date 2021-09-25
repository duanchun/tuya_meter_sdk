

/**
****************************************************************************
* @file      tuya_kel_flash.h
* @brief     tuya_kel_flash
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_KEL_FLASH_H__
#define __TUYA_KEL_FLASH_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

TUYA_RET_E tuya_kel_flash_init(void);

TUYA_RET_E tuya_kel_flash_read(uint32_t addr, void* buf, uint32_t size);

TUYA_RET_E tuya_kel_flash_write(uint32_t addr, const void* buf, uint32_t size);

TUYA_RET_E tuya_kel_flash_erase(uint32_t addr, uint32_t num);

#ifdef __cplusplus
}
#endif

#endif



