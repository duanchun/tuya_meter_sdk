/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
 /*
 * INCLUDES (包含头文件)
 */
#include <stdbool.h>
#include "os_timer.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "driver_gpio.h"
#include "simple_gatt_service.h"
#include "peripheral.h"
#include "dev_info_service.h"
#include "batt_service.h"
#include "hid_service.h"
#include "jump_table.h"

#include "sys_utils.h"
#include "flash_usage_config.h"
#include "tuya_ble_api.h"
#include "ty_ble.h"

#include "ty_ble_config.h"
#include "tuya_ble_log.h"
#include "locdef_service.h"
#include "locdef_main.h"

#define TUYA_BLE_MAX_PEER_BOND_NUM      (8) //TODO


static os_timer_t os_timer_for_get_rssi = {0};
static os_timer_t os_timer_start_rssi_timer_delay = {0};
static os_timer_t os_timer_for_sv_change = {0};

static uint8_t slave_link_conidx;

uint8_t slave_irk[16] = {0};

static void ble_peripheral_bond_init(uint8_t enable);


#if TUYA_BLE_HID_HOST_SVC_CHANGE
svc_change_t hid_svc_change =
{
    .att_idx = HID_INFO_IDX,
    .type = PERM,
    .param.new_prop = GATT_PROP_READ , 
};
#endif

/*********************************************************************
 * @fn      app_gap_evt_cb
 *
 * @brief   Application layer GAP event callback function. Handles GAP evnets.
 *
 * @param   p_event - GAP events from BLE stack.
 *       
 *
 * @return  None.
 */
void app_gap_evt_cb(gap_event_t *p_event)
{
    DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param(); //TODO
    switch(p_event->type)
    {
        case GAP_EVT_ADV_END: {
            extern volatile bool g_adv_restart_glag;
            if(g_adv_restart_glag) {
                TUYA_APP_LOG_INFO("restart adv");
                g_adv_restart_glag = false;
                ty_ble_start_adv();
            }
        } break;
        
        case GAP_EVT_ALL_SVC_ADDED: {
#if TUYA_BLE_HID_HOST_SVC_CHANGE
            hid_svc_change.svc_id = hid_svc_id;
            hid_svc_change.param.new_prop = GATT_PROP_READ;

            gatt_change_svc(hid_svc_change);
#endif
            ty_ble_start_adv();
        } break;

        case GAP_EVT_SLAVE_CONNECT: {
            TUYA_APP_LOG_INFO("Connected");
            ty_ble_connect_handler();
            gap_set_mtu(247); //TODOs
            gatt_mtu_exchange_req(0);
            slave_link_conidx = p_event->param.slave_connect.conidx;
        } break;

        case GAP_EVT_DISCONNECT: {
            TUYA_APP_LOG_INFO("Link[%d] disconnect,reason:0x%02X", p_event->param.disconnect.conidx, p_event->param.disconnect.reason);
            ty_ble_disconnect_handler();
            ty_ble_start_adv();

            // disconnect the bond status and rssi query
            ty_hid_bond_info_disconnect();//TODO
            os_timer_stop(&os_timer_for_get_rssi); // stop rssi get
            ble_peripheral_bond_init(0); // close gap bonding
#if TUYA_BLE_HID_HOST_SVC_CHANGE
            hid_svc_change.svc_id = hid_svc_id;
            hid_svc_change.param.new_prop = GATT_PROP_READ;

            gatt_change_svc(hid_svc_change);
            run_param->hid_svc_change_on = TY_SWITCH_OFF;//TODO
#endif
        } break;

        case GAP_EVT_LINK_PARAM_REJECT: {
            TUYA_APP_LOG_INFO("Link[%d]param reject,status:0x%02x"
                      ,p_event->param.link_reject.conidx,p_event->param.link_reject.status);
        } break;

        case GAP_EVT_LINK_PARAM_UPDATE: {
            TUYA_APP_LOG_INFO("conn param update, interval:%dms, latency:%d, timeout:%dms",
                        (uint16_t)(p_event->param.link_update.con_interval*1.25),
                        (uint16_t)(p_event->param.link_update.con_latency),
                        (uint16_t)(p_event->param.link_update.sup_to*10));
        } break;

        case GAP_EVT_PEER_FEATURE: {
            TUYA_APP_LOG_INFO("peer[%d] feats ind",p_event->param.peer_feature.conidx);
        } break;

        case GAP_EVT_MTU: {
            TUYA_APP_LOG_INFO("mtu = %d", p_event->param.mtu.value);
        } break;
        
        case GAP_EVT_LINK_RSSI: {
            ty_hid_check_rssi(p_event->param.link_rssi); //TODO
        } break;
        case GAP_SEC_EVT_BOND_START:{
            TUYA_APP_LOG_INFO("Start bond procedure");
        }break;
        case GAP_SEC_EVT_BOND_FAIL:{
            ty_hid_bond_refresh(NULL);//TODO
            TUYA_APP_LOG_INFO("fail bond, report fail bond dp");
            if(p_event->param.bond_fail.reason == 0x45) { // For fixing apple issue
                tuya_ble_gap_disconnect();
            }
        }break;
        case GAP_SEC_EVT_SLAVE_ENCRYPT: {
			gap_bond_info_t bond_info;
            uint8_t index = 0;
            TUYA_APP_LOG_INFO("slave[%d]_encrypted,bond link index[%d]",p_event->param.slave_encrypt_conidx, gap_get_latest_bond_idx());

            if(gap_get_latest_bond_idx() < TUYA_BLE_MAX_PEER_BOND_NUM) {
                index = gap_get_latest_bond_idx();
                gap_bond_manager_get_info(index, &bond_info);
                memcpy(slave_irk, bond_info.peer_irk, 16); // must do, stack will release data
                if(ty_hid_bond_refresh(slave_irk) == 0) { //TODO
				//if(0){
                    TUYA_APP_LOG_INFO("Start RSSI Get Timer");
                    gap_set_link_rssi_report(true);
                    os_timer_stop(&os_timer_start_rssi_timer_delay);
                    os_timer_start(&os_timer_start_rssi_timer_delay, 2000, 0);
#if TUYA_BLE_HID_HOST_SVC_CHANGE
					#if 1 //TODO
                    if(TY_SWITCH_ON == run_param->hid_svc_change_on) {
                        os_timer_start(&os_timer_for_sv_change, 100, 0);
                    }
					#endif
#endif
                }
            }
			
        } break;

        case GAP_EVT_LINK_PARAM_REQ: {
            gap_evt_link_param_update_rsp_t rsp = {0};
            rsp.accept = true;
            rsp.src_id = p_event->param.link_update_req.dst_id;
            rsp.dst_id = p_event->param.link_update_req.src_id;
            rsp.conidx = 0;
            rsp.ce_len_min = 24;
            rsp.ce_len_max = 24;
            gap_param_update_rsp(&rsp);
			TUYA_APP_LOG_INFO("GAP_EVT_LINK_PARAM_REQ");
        } break;

        default: {
            TUYA_APP_LOG_INFO("unprocess type 0x%02x", p_event->type);
        } break;
    }
}

__attribute__((section("ram_code"))) void exti_isr_ram(void)
{
    uint32_t status,timer_value =0;
    status = ext_int_get_src();
	ext_int_clear(status);
	ty_app_exti_isr(status);
}

static void os_timer_for_get_rssi_handler(void *arg)
{
    gap_get_link_rssi(slave_link_conidx);
}

static void os_timer_for_start_rssi_delay_handler(void *arg)
{
    os_timer_stop(&os_timer_for_get_rssi);
    os_timer_start(&os_timer_for_get_rssi, 200, 1);
}


#if TUYA_BLE_HID_HOST_SVC_CHANGE
static void os_timer_for_svc_change_handler(void *arg)
{
//    uint16_t svc_uuid = 0xFD50;//0x1801;
//    uint16_t gap_svc_shdl = 0;
//    uint16_t gap_svc_ehdl = 0;

//    gatt_get_svc_hdl((uint8_t *)&svc_uuid,2,&gap_svc_shdl,&gap_svc_ehdl);

//    TUYA_APP_LOG_INFO("UUID(0x%02x), start handle = 0x%02x, end handle = 0x%02x", svc_uuid, gap_svc_shdl, gap_svc_ehdl);

    hid_svc_change.svc_id = hid_svc_id;
    hid_svc_change.param.new_prop = GATT_PROP_READ | GATT_PROP_AUTHEN;
    gatt_change_svc(hid_svc_change);
        						
    //gatt_svc_changed_req(slave_link_conidx,gap_svc_shdl,gap_svc_ehdl);
    gatt_svc_changed_req(slave_link_conidx,0x0001,0xffff);
}
#endif

static void ble_peripheral_bond_init(uint8_t enable)
{
    // Initialize security related settings.
    gap_security_param_t param = {
        .mitm = true, // for future use
        .ble_secure_conn = false,  //true, // [Android] some phone cannot support secure connect, but support bonding auth
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,  
        .pair_init_mode = GAP_PAIRING_MODE_INITIATE,
        .bond_auth = true,
        .password = 123456, //TODO
    };
    
    if(enable) {
        param.pair_init_mode = GAP_PAIRING_MODE_INITIATE;//GAP_PAIRING_MODE_INITIATE;
    }else {
        param.pair_init_mode = GAP_PAIRING_MODE_NO_PAIRING;
    }
    
    gap_security_param_init(&param);
}

void ble_peripheral_bond_request(void)
{
    ble_peripheral_bond_init(1);
   
#if TUYA_BLE_HID_HOST_REQ 
    TUYA_APP_LOG_INFO("start to rev bind");

    hid_svc_change.svc_id = hid_svc_id;
    hid_svc_change.param.new_prop = GATT_PROP_READ | GATT_PROP_AUTHEN;
    gatt_change_svc(hid_svc_change);

    gatt_svc_changed_req(slave_link_conidx,0x1,0xffff);
#else
    TUYA_APP_LOG_INFO("start to request bind");

    gap_security_req(slave_link_conidx);
#endif
}

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
void peripheral_init(void)
{
    // set local device name
    uint8_t local_name[] = TY_DEVICE_NAME;
    gap_set_dev_name(local_name, sizeof(local_name));
    
    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, TUYA_BLE_MAX_PEER_BOND_NUM, true);

    //gap_set_dev_appearance(GAP_APPEARE_HID_MOUSE);
    
    ble_peripheral_bond_init(0);
        
    gap_set_cb_func(app_gap_evt_cb);

    dis_gatt_add_service();
    batt_gatt_add_service();
    hid_gatt_add_service();
    
    sp_gatt_add_service();
    
    os_timer_init(&os_timer_for_get_rssi, os_timer_for_get_rssi_handler, NULL);
    os_timer_init(&os_timer_start_rssi_timer_delay,os_timer_for_start_rssi_delay_handler,NULL);
#if TUYA_BLE_HID_HOST_SVC_CHANGE
    os_timer_init(&os_timer_for_sv_change, os_timer_for_svc_change_handler, NULL);
#endif
    TUYA_APP_LOG_INFO("ty_ble_peripheral_init successed");
}

