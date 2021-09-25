/**
****************************************************************************
* @file      ty_ble_dp.h
* @brief     ty_ble_dp
* @author    xiaojian
* @version   V0.0.1
* @date      2021-08-14
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/


#ifndef __TY_BLE_DP_H__
#define __TY_BLE_DP_H__

#include "tuya_ble_stdlib.h"
#include "tuya_ble_mutli_tsf_protocol.h"

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
                        1:dp数据点序列号重新定义
            此为自动生成代码，如在开发平台有相关修改请重新下载SDK
****************************************************************************/
#define STANDARD_DP
//布防开关,可下发可上报
//Notes:
#define DPID_DEF_CTRL                           1 //协议定义，0-->上锁状态 1-->解锁状态
//启动,可下发可上报
//Notes:
#define DPID_ACC_CTRL                           2
//电池电量,只上报
//Notes:
#define DPID_POWER_LEVEL                        3
//速度,只上报
//Notes:
#define DPID_SPEED                              17//6
//单次里程,只上报
//Notes:
#define DPID_SIG_MILEAGE                        5
//寻车,可下发可上报
//Notes:
#define DPID_SEARCH                             0x1E
//坐桶锁,可下发可上报
//Notes:
#define DPID_CUSHION_CTRL                       32

//中控电池电量,只上报
//Notes:小电池
#define DPID_VBAT_LEVEL                         16

#define DPID_RSSI_SENSITIVITY                   33

//HID远离上锁,只下发
//Notes:
#define DPID_FAR_LOCK_CLOSE_SWITCH              34 //协议定义，1-->打开 0-->关闭

//设备状态,只上报
//Notes:
#define DPID_DEV_STATUS                         35 


//轮径,只可下发可上报
//Notes:
#define DPID_WHEEL_DIAMETE                       40 


//HID绑定状态,只上报
//Notes:
#define DPID_QUERY_BOND_STATUS                   52 //协议定义，1-->未绑定 0-->已绑定

//HID靠近解锁,只上报
//Notes:
#define DPID_NEAR_UNLOCK_OPEN_SWITCH             53 //协议定义，1-->打开 0-->关闭



typedef enum
{
    DP_HOUR = 1000*60*60,
    DP_MINUTE = 1000*60,
    DP_SECOND = 1000,
    DP_MILLIS = 1,
}DP_INTER_UNIT_E;



typedef enum {
    TY_PRO_TYPE_DP_PARSE = 0,
	TY_PRO_TYPE_DP_REPORT
}ty_dp_pro_type_e;

typedef struct {
	uint8_t dpid;//
	uint8_t type;//dpid 命令类型
	uint16_t len;//数据长度
}ty_dp_header_t;

typedef struct {
	ty_dp_header_t header;
	void *value;
}ty_dp_data_t;


/**
 * @brief  处理接收到的dp数据
 *
 * @param
 *         [in]dp_data  : dp数据
 *         [in]dp_len   : dp数据长度
 *
 * @note   
 *
 * */
int ty_dp_data_received_handle(uint8_t *dp_data, uint16_t dp_len);

void ty_dp_query_req(void);


#ifdef __cplusplus
}
#endif

#endif//__TY_BLE_DP_H__

