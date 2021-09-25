/**
****************************************************************************
* @file      tuya_ble_port_fr801xH.h
* @brief     tuya_ble_port_fr801xH
* @author    suding
* @version   V1.0.0
* @date      2021-05
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/


#ifndef __TUYA_BLE_PORT_FR801XH_H__
#define __TUYA_BLE_PORT_FR801XH_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDE
 */
#include "tuya_ble_config.h"
#include "ty_system.h"
#include "elog.h"

/*********************************************************************
 * CONSTANT
 */
#if (TUYA_BLE_LOG_ENABLE||TUYA_APP_LOG_ENABLE)
    #define TUYA_BLE_PRINTF(...)        log_d(__VA_ARGS__)
    #define TUYA_BLE_HEXDUMP(...)       ty_system_log_hexdump("", 8, __VA_ARGS__)
#else
    #define TUYA_BLE_PRINTF(...)
    #define TUYA_BLE_HEXDUMP(...)
#endif

#define tuya_ble_device_enter_critical()        \
{                                               \
    uint8_t __CR_NESTED = 0;                    \
    GLOBAL_INT_DISABLE();

#define tuya_ble_device_exit_critical()         \
    GLOBAL_INT_RESTORE();\
}

/*********************************************************************
 * STRUCT
 */

/*********************************************************************
 * EXTERNAL VARIABLE
 */

/*********************************************************************
 * EXTERNAL FUNCTION
 */


#ifdef __cplusplus
}
#endif

#endif //__TUYA_BLE_PORT_FR801XH_H__



