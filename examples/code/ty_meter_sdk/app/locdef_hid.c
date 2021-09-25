
#include "locdef_hid.h"
#include "locdef_service.h"
#include "board.h"
#include "vfm_sensor.h"
#include "locdef_main.h"
#include "locdef_pin.h"
#include "tuya_ble_log.h"
#include "locdef_fsm.h"
#include "locdef_dp.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_log.h"


#define DEV_HID_RSSI_FILTER_DEBOUNCE  (5) // Add the filter data for opening the lock
#define RSSI_SAMPLE_MAX                 3
static uint8_t  s_hid_key_verify  = 0;
static uint8_t  s_hid_rssi_count = 0;
static int8_t   s_hid_rssi_sample[RSSI_SAMPLE_MAX] = {0};
static uint8_t  s_hid_unlock_once = 0;
static int8_t   s_hid_sensitivty  = RSSI_OPEN_MIDDLE;
static ty_dev_hid_t s_dev_bond_arrary[TUYA_BLE_MAX_PEER_BOND_NUM] = {0};


static int ty_hid_bond_info_check(uint8_t *key)
{
    uint8_t i = 0;

    for(i = 0;i < TUYA_BLE_MAX_PEER_BOND_NUM; i++) {
        if(s_dev_bond_arrary[i].active && memcmp(s_dev_bond_arrary[i].key, key, 16) == 0) {
            TUYA_LOG_I(MOD_HID, "get the bond info, index = %d", i);
            return i;
        }else if(s_dev_bond_arrary[i].active){
            //TUYA_APP_LOG_HEXDUMP_INFO("ty_hid_bond_info_check KEY:", s_dev_bond_arrary[i].key, 16);
        }
    }

    return -1;
}

int ty_hid_bond_info_reset(void)
{
    uint32_t err_code=0;
	DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();

    if(tuya_ble_nv_erase(BOARD_FLASH_HID_BOND_INFO_ADDR,TUYA_NV_ERASE_MIN_SIZE)==TUYA_BLE_SUCCESS)
    {
        err_code = 0;
        memset((uint8_t *)&s_dev_bond_arrary, 0, sizeof(s_dev_bond_arrary));
    }
    else
    {
        if(tuya_ble_nv_erase(BOARD_FLASH_HID_BOND_INFO_ADDR,TUYA_NV_ERASE_MIN_SIZE)==TUYA_BLE_SUCCESS)
        {
            TUYA_LOG_I(MOD_HID, "write tuya_ble_hid_bond_info_reset data succeed!");
        }
        else
        {
            TUYA_LOG_E(MOD_HID, "write tuya_ble_hid_bond_info_reset data failed!");
            err_code = 2;
        }
    }

    // if reset, we need to reset bond flag, and do not update values
    run_param->hid_dev_bond = TY_HID_DEV_UNBOND;
    
    return 0;
}

static int ty_hid_bond_info_save(uint8_t *key)
{
    uint32_t err_code=0;
    uint8_t i = 0;
    uint8_t fill_flag = 0;
	DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();

    for(i = 0; i < TUYA_BLE_MAX_PEER_BOND_NUM;i ++) {
        if(memcmp(s_dev_bond_arrary[i].key, key, 16) == 0 || s_dev_bond_arrary[i].active == 0) {

		    s_dev_bond_arrary[i] = run_param->dev_hid;
		    memcpy(s_dev_bond_arrary[i].key, key, 16);
            s_dev_bond_arrary[i].active = 1;
            fill_flag = 1;
            break;
        }
    }

    if(i == TUYA_BLE_MAX_PEER_BOND_NUM && fill_flag == 0) { // if run out, we will use the last index
    
        TUYA_LOG_E(MOD_HID, "write tuya_ble_hid_bond_info_save no space!");
        s_dev_bond_arrary[TUYA_BLE_MAX_PEER_BOND_NUM-1] = run_param->dev_hid;
	    memcpy(s_dev_bond_arrary[TUYA_BLE_MAX_PEER_BOND_NUM-1].key, key, 16);
        s_dev_bond_arrary[TUYA_BLE_MAX_PEER_BOND_NUM-1].active = 1;
    }

    if(tuya_ble_nv_erase(BOARD_FLASH_HID_BOND_INFO_ADDR,TUYA_NV_ERASE_MIN_SIZE)==TUYA_BLE_SUCCESS)
    {
        err_code = tuya_ble_nv_write(BOARD_FLASH_HID_BOND_INFO_ADDR,(uint8_t *)s_dev_bond_arrary,sizeof(s_dev_bond_arrary));
    
        if(err_code == TUYA_BLE_SUCCESS)
        {
            TUYA_LOG_I(MOD_HID, "write tuya_ble_hid_bond_info_save data succeed!");
        }
        else
        {
            TUYA_LOG_E(MOD_HID, "write tuya_ble_hid_bond_info_save data failed!");
            err_code = 2;
        }
    }
    else
    {
        TUYA_LOG_E(MOD_HID, "erase tuya_ble_hid_bond_info_save data failed!");
        err_code = 3;
    }
    return err_code;
}

static int ty_hid_config_info_save(uint8_t *key)
{
    uint32_t err_code=0;
    uint8_t i = 0;
    uint8_t fill_flag = 0;
	DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();

    for(i = 0; i < TUYA_BLE_MAX_PEER_BOND_NUM;i ++) {
        if(memcmp(s_dev_bond_arrary[i].key, key, 16) == 0) {
            s_dev_bond_arrary[i] = run_param->dev_hid;
            s_dev_bond_arrary[i].active = 1;
            fill_flag = 1;
            break;
        }
    }

    if(i == TUYA_BLE_MAX_PEER_BOND_NUM && fill_flag == 0) {
        TUYA_LOG_E(MOD_HID, "write tuya_ble_hid_bond_info_save no space, or cannot find!");
        return 1;
    }

    if(tuya_ble_nv_erase(BOARD_FLASH_HID_BOND_INFO_ADDR,TUYA_NV_ERASE_MIN_SIZE)==TUYA_BLE_SUCCESS)
    {
        err_code = tuya_ble_nv_write(BOARD_FLASH_HID_BOND_INFO_ADDR,(uint8_t *)&s_dev_bond_arrary,TUYA_BLE_MAX_PEER_BOND_NUM*sizeof(ty_dev_hid_t));
    
        if(err_code == TUYA_BLE_SUCCESS)
        {
            TUYA_LOG_I(MOD_HID, "write tuya_ble_hid_config_info_save data succeed!");
        }
        else
        {
            TUYA_LOG_E(MOD_HID, "write tuya_ble_hid_config_info_save data failed!");
            err_code = 2;
        }
    }
    else
    {
        TUYA_LOG_E(MOD_HID, "erase tuya_ble_hid_config_info_save data failed!");
        err_code = 3;
    }
    return err_code;
}


int ty_hid_bond_info_disconnect(void)
{
    DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();
	uint8_t clock = TY_SWITCH_OFF;
	
    if(TY_HID_DEV_BONDED == run_param->hid_dev_bond) {
        ty_hid_config_info_save(run_param->dev_hid.key);
    }

    if(run_param->dev_hid.far_lock_switch) { // if disconnect, we also need to close lock
        tuya_vfm_base_event_post(SENSOR_EVENT_DEFENCE,0);
		TUYA_LOG_I(MOD_HID, "ty_hid_bond_info_disconnect");
    }

    // after disconnect. we will release bond
    s_hid_key_verify = 0;
    run_param->dev_hid.near_unlock_switch     = 0;
    run_param->dev_hid.far_lock_switch        = 0;
    s_hid_unlock_once = 0;
    
    s_hid_rssi_count = 0;
    memset(s_hid_rssi_sample, 0, RSSI_SAMPLE_MAX);

    run_param->hid_dev_bond = TY_HID_DEV_UNBOND;
    //memset(tuya_ble_hid_key, 0, 16); // for backup

    return 0;
}


int ty_hid_check_rssi(int8_t rssi)
{
    int32_t rssi_average = 0;
	uint8_t lock = TY_SWITCH_ON;
    uint8_t i = 0;
	DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();

    //TUYA_APP_LOG_INFO("rssi==%d",rssi);
    if(rssi > -15 || rssi < -95) 
    {
		return 0;
    }

    if(run_param->dev_hid.near_unlock_switch || run_param->dev_hid.far_lock_switch) 
	{
        if(s_hid_key_verify == 0 && ty_hid_bond_info_check(run_param->dev_hid.key) >= 0) {
            s_hid_key_verify = 1;
            s_hid_rssi_count = 0;
        }

        if(s_hid_key_verify) {
            s_hid_rssi_sample[s_hid_rssi_count] = rssi;
            s_hid_rssi_count ++;
            if(s_hid_rssi_count >= RSSI_SAMPLE_MAX) {
                s_hid_rssi_count = 0;
                for(i = 0; i< RSSI_SAMPLE_MAX; i++) {
                    rssi_average += s_hid_rssi_sample[i];
                }
                rssi_average = rssi_average/RSSI_SAMPLE_MAX;
                if(run_param->dev_hid.near_unlock_switch && s_hid_unlock_once == 0 &&rssi_average > (run_param->dev_hid.sensitivty + DEV_HID_RSSI_FILTER_DEBOUNCE)) {
                    tuya_vfm_base_event_post(SENSOR_EVENT_UNDEFENCE,0);
                    s_hid_unlock_once = 1;
                
                    TUYA_LOG_I(MOD_HID, "!!!!!!!!get near device, try to open lock,rssi = %d", rssi_average);
                }
                
                
                if(run_param->dev_hid.far_lock_switch && rssi_average < RSSI_CLOSE_NORMAL) { // if open the near-unlock, we need to do lock once going through the ebike
					s_hid_unlock_once = 0;
					lock = TY_SWITCH_OFF;
					tuya_vfm_base_event_post(SENSOR_EVENT_DEFENCE,0);
                
                    TUYA_LOG_I(MOD_HID, "******get far device, try to close lock,rssi = %d", rssi_average);
                }
            }
        }
    }else {
        s_hid_key_verify = 0;
        s_hid_rssi_count = 0;
        s_hid_unlock_once = 0;
    }
    return 0;
}

int ty_hid_bond_refresh(uint8_t *key)
{
    int index = -1;
	uint8_t bond_status = TY_HID_DEV_UNBOND;

	DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();

    if(key == NULL) { // Bond
        run_param->dev_hid.far_lock_switch = TY_SWITCH_OFF;
		run_param->dev_hid.near_unlock_switch = TY_SWITCH_OFF;
        locdef_dp_update(DPID_QUERY_BOND_STATUS, DT_ENUM,1,&bond_status);
		locdef_dp_update(DPID_NEAR_UNLOCK_OPEN_SWITCH, DT_BOOL,1,&run_param->dev_hid.near_unlock_switch);
		locdef_dp_update(DPID_FAR_LOCK_CLOSE_SWITCH, DT_BOOL,1,&run_param->dev_hid.far_lock_switch);
        return -1;
    }
    
    TUYA_APP_LOG_HEXDUMP_INFO("info key:", key, 16);
    tuya_ble_nv_read(BOARD_FLASH_HID_BOND_INFO_ADDR,(uint8_t *)s_dev_bond_arrary,sizeof(s_dev_bond_arrary));
    index = ty_hid_bond_info_check(key);

    if(key != NULL) {
        // try to save this info
        if(tuya_ble_connect_status_get() == BONDING_CONN) { // Only being in bonding-conn status, we will save key

            // report bond status to app
            bond_status = TY_HID_DEV_BONDED;
			TUYA_LOG_I(MOD_HID, "bond_status==1");
            locdef_dp_update(DPID_QUERY_BOND_STATUS, DT_ENUM,1,&bond_status);

            if(index < 0) {
                // if we dont have this key, try to save this key
                ty_hid_bond_info_save(key);
            }else {
                ty_hid_config_info_save(key);
            }
            index = ty_hid_bond_info_check(key);
        }
        
        if(index < 0) {
            TUYA_LOG_E(MOD_HID, "Not find key, stat(%d)",tuya_ble_connect_status_get());
            return -1;
        }else {
            run_param->dev_hid.far_lock_switch = s_dev_bond_arrary[index].far_lock_switch;
            run_param->dev_hid.near_unlock_switch = s_dev_bond_arrary[index].near_unlock_switch;
            run_param->dev_hid.sensitivty = s_dev_bond_arrary[index].sensitivty;
			if(run_param->dev_hid.sensitivty >= RSSI_OPEN_CLOSE || run_param->dev_hid.sensitivty < RSSI_OPEN_HIGH)
			{
			    run_param->dev_hid.sensitivty = RSSI_OPEN_MIDDLE;
			}
            TUYA_LOG_I(MOD_HID, "re-bond status(%d), info(far-%d, near-%d, sen-%d)", tuya_ble_connect_status_get(),\
				run_param->dev_hid.far_lock_switch, run_param->dev_hid.near_unlock_switch, run_param->dev_hid.sensitivty);
        }
        
        run_param->hid_dev_bond = TY_HID_DEV_BONDED;
        memcpy(run_param->dev_hid.key, key, 16);
        
        tuya_ble_connect_monitor_timer_stop(); // stop disconnect timer, for making sure link
    }
    return 0;
}

