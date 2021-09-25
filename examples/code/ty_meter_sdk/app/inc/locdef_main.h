/**
****************************************************************************
* @file      locdef_main.h
* @brief     locdef_main
* @author    xiaojian
* @version   V0.0.1
* @date      2021-08
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/


#ifndef __LOCDEF_MAIN_H__
#define __LOCDEF_MAIN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

typedef enum {
    TY_SWITCH_OFF = 0, // 关闭/上锁
    TY_SWITCH_ON = 1   // 打开/解锁
}ty_switch_e;

#ifdef __cplusplus
}
#endif

#endif //__TY_BLE_APP_H__

