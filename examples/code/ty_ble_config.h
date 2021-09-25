
/**
****************************************************************************
* @file      ty_ble_config.h
* @brief     ty_ble_config
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TY_BLE_APP_H__
#define __TY_BLE_APP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define TY_DEVICE_NAME        "TYHID"
#define TY_DEVICE_PID         "nh56iwzs" //
#define TY_DEVICE_MAC         "DC234D12ED2E"
#define TY_DEVICE_DID         "tuyaf3752699e2cd" //16Bytes
#define TY_DEVICE_AUTH_KEY    "4gBM3DK6SRmRn9LTLbyOUAz3bHMGCfMW" //32Bytes
	
#define TY_DEVICE_FIR_NAME    "ty_ble_hid_app_frq8018" //
#define TY_DEVICE_FVER_NUM    0x00000100 //
#define TY_DEVICE_FVER_STR    "1.0" //
#define TY_DEVICE_HVER_NUM    0x00000100
#define TY_DEVICE_HVER_STR    "1.0"


#define TY_ADV_INTERVAL       100   //range: 20~10240ms
#define TY_CONN_INTERVAL_MIN  180   //range: 7.5~4000ms
#define TY_CONN_INTERVAL_MAX  200   //range: 7.5~4000ms

#ifdef __cplusplus
}
#endif

#endif

