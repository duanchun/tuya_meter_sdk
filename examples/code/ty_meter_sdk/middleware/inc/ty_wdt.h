/**
****************************************************************************
* @file      ty_ble_wdt.h
* @brief     ty_ble_wdt
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/


#ifndef __TY_BLE_WDT_H__
#define __TY_BLE_WDT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_ble_type.h"

void ty_ble_wdt_init(uint8_t delay_s);

void ty_ble_wdt_feed(void);

void ty_ble_wdt_start(void);

void ty_ble_wdt_stop(void);



#ifdef __cplusplus
}
#endif
#endif


