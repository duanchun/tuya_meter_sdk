
/**
****************************************************************************
* @file      tuya_kel_timer.h
* @brief     tuya_kel_timer
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_KEL_TIMER_H__
#define __TUYA_KEL_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

typedef enum {
    TY_KEL_TIMER_SINGLE_SHOT,
    TY_KEL_TIMER_REPEATED
} TUYA_KEL_TIMER_MODE_E;

typedef void (*tuya_kel_timer_handler_cbk)(void*);
typedef void* tuya_kel_timer_handle;


TUYA_RET_E tuya_kel_timer_create(tuya_kel_timer_handle *handle, uint32_t ms, TUYA_KEL_TIMER_MODE_E mode, tuya_kel_timer_handler_cbk handler);
TUYA_RET_E tuya_kel_timer_delete(tuya_kel_timer_handle handle);
TUYA_RET_E tuya_kel_timer_start(tuya_kel_timer_handle handle);
TUYA_RET_E tuya_kel_timer_stop(tuya_kel_timer_handle handle);
TUYA_RET_E tuya_kel_timer_restart(tuya_kel_timer_handle handle, uint32_t ms);


#ifdef __cplusplus
}
#endif

#endif

