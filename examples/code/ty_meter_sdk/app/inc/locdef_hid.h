

#ifndef __LOCDEF_HID_H__
#define __LOCDEF_HID_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "tuya_ble_type.h"

#define TUYA_BLE_MAX_PEER_BOND_NUM      (8)

#if 0
enum {
    RSSI_OPEN_CLOSE     = 0,
    RSSI_OPEN_LOW       = -72,  // need to be more closer 
    RSSI_OPEN_MIDDLE    = -78,
    RSSI_OPEN_HIGH      = -87, // can be more far away from device
    RSSI_CLOSE_NORMAL      = RSSI_OPEN_HIGH + (-12), // can be more far away from device
};
#else
enum {
    RSSI_OPEN_CLOSE     = 0,
    RSSI_OPEN_LOW       = -55,  // need to be more closer 
    RSSI_OPEN_MIDDLE    = -65,
    RSSI_OPEN_HIGH      = -75, // can be more far away from device
    RSSI_CLOSE_NORMAL      = RSSI_OPEN_HIGH + (-12), // can be more far away from device
};
#endif

typedef enum {
    TY_HID_DEV_BONDED = 0, // 
    TY_HID_DEV_UNBOND = 1  // 
}ty_hid_dev_bond_e;


typedef struct{
	uint8_t key[16];
	int8_t sensitivty;//靠近解锁灵敏度
	int8_t far_lock_sen;//远离上锁灵敏度
	uint8_t active;//是否已经被使用
	uint8_t near_unlock_switch;//靠近解锁锁开关[0:关闭,1:打开]
	uint8_t far_lock_switch;//远离上锁开关[0:关闭,1:打开]
}ty_dev_hid_t;



#ifdef __cplusplus
}
#endif

#endif

