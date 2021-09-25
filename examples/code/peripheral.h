/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
#ifndef BLE_SIMPLE_PERIPHERAL_H
#define BLE_SIMPLE_PERIPHERAL_H
 
#define SP_TASK_EVT_NOTI    0x0001
#define TUYA_BLE_HID_HOST_SVC_CHANGE    1


enum
{
    GATT_SVC_IDX_SP,
    GATT_SVC_NUM    
};
// Gap advertising parameters
struct ty_gap_adv_param_t
{
    uint8_t         adv_mode;               //!< Advertising mode, connectable/none-connectable, see @ GAP_ADV_MODE_DEFINES
    uint8_t         adv_addr_type;          //!< see @ GAP_ADDR_TYPE_DEFINES
    gap_mac_addr_t  peer_mac_addr;          //!< peer mac addr,used for direction adv
    uint8_t         phy_mode;               //!< see @GAP_PHY_VALUES
    uint16_t        adv_intv_min;           //!< Minimum advertising interval, (in unit of 625us). Must >=  32
    uint16_t        adv_intv_max;           //!< Maximum advertising interval, (in unit of 625us). Must >=  32
    uint8_t         adv_chnl_map;           //!< Advertising channal map, 37, 38, 39, see @ GAP_ADVCHAN_DEFINES
    uint8_t         adv_filt_policy;        //!< Advertising filter policy, see @ GAP_ADV_FILTER_MODE_DEFINES
    uint8_t         adv_sid;                //!< Advertising set idx, for EXTENDED adv and PERIODIC ADV only, range:0~0xF
    uint16_t        per_adv_intv_min;       //!< Minimum periodic advertising interval (in unit of 1.25ms).. Must be greater than 20ms,for PERIODIC ADV only
    uint16_t        per_adv_intv_max;       //!< Maximum periodic advertising interval (in unit of 1.25ms). Must be greater than 20ms,for PERIODIC ADV only
    uint8_t         disc_mode;              //!< Advertising discovery mode, see @defgroup GAP_DISC_MODE
} ;


/** @function group ble peripheral device APIs (ble外设相关的API)
 * @{
 */
 
void app_gap_evt_cb(gap_event_t *p_event);

/*********************************************************************
 * @fn      simple_peripheral_init
 *
 * @brief   Initialize simple peripheral profile, BLE related parameters.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void peripheral_init(void);

void ble_peripheral_bond_request(void);

#endif
