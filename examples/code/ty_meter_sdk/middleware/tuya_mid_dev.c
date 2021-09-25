
#include "tuya_mid_dev.h"
#include "locdef_config.h"
#include "rcu433_srv.h"
#include "board.h"
#include "custom_tuya_ble_config.h"
#include "tuya_log.h"

typedef struct {
	tuya_dev_cfg_t dev_config;
	rcu_addr_t rcu_addr[TY_RCU433_DEV_REG_MAX];
}ty_locdef_flash_t;

TUYA_RET_E tuya_mid_wd_common_write(const uint8_t key_id, const uint8_t *value, const uint32_t len)
{
#if 1
    ty_locdef_flash_t temp = {0};

	memset(&temp,0,sizeof(ty_locdef_flash_t));
    tuya_ble_nv_read(BOARD_FLASH_EBIKE_CONFIG_INFO_ADDR,(uint8_t *)&temp,sizeof(temp));

	if(tuya_ble_nv_erase(BOARD_FLASH_EBIKE_CONFIG_INFO_ADDR,TUYA_NV_ERASE_MIN_SIZE)==TUYA_BLE_SUCCESS)//TODO
	{
		if(RCU_ADDRESS1 == key_id)
		{
		
		    memcpy(&temp.rcu_addr[0],value,len);
			tuya_ble_nv_write(BOARD_FLASH_EBIKE_CONFIG_INFO_ADDR,(uint8_t*)&temp,sizeof(ty_locdef_flash_t));
			return 0;
		}else if(RCU_ADDRESS2 == key_id){
			
			memcpy(&temp.rcu_addr[0],value,len);
			tuya_ble_nv_write(BOARD_FLASH_EBIKE_CONFIG_INFO_ADDR,(uint8_t*)&temp,sizeof(ty_locdef_flash_t));
			return 0;
		}else if(DEV_CFG_PARAM == key_id){
			
			memcpy(&temp.dev_config,value,len);
			tuya_ble_nv_write(BOARD_FLASH_EBIKE_CONFIG_INFO_ADDR,(uint8_t*)&temp,sizeof(ty_locdef_flash_t));
			return 0;
		}
		TUYA_LOG_I(MOD_DEV, "wd_common_read malloc not map one keyID");
	}else{
		TUYA_LOG_E(MOD_DEV, "tuya_ble_nv_erase BOARD_FLASH_HID_BOND_INFO_ADDR failed");
	}
    #endif
	return TUYA_OK;
}

TUYA_RET_E ty_mid_wd_user_param_read( uint8_t **buf,uint32_t *len)
{
    //TODO
    return TUYA_OK;
}


TUYA_RET_E tuya_mid_wd_common_read(const uint8_t key_id, uint8_t **value, uint32_t *len)
{
#if 1
    ty_locdef_flash_t temp = {0};
	uint8_t *value_temp = NULL;

	memset(&temp,0,sizeof(ty_locdef_flash_t));
    tuya_ble_nv_read(BOARD_FLASH_EBIKE_CONFIG_INFO_ADDR,(uint8_t *)&temp,sizeof(temp));

	if(RCU_ADDRESS1 == key_id)
	{
	
	    value_temp = (uint8_t*)tuya_ble_malloc(sizeof(rcu_addr_t));
		if(NULL == value_temp)
		{
		    TUYA_LOG_E(MOD_DEV, "wd_common_read malloc failed");
		    *value = NULL;
		    return 1;
		}
	    memcpy(value_temp,&temp.rcu_addr[0],sizeof(rcu_addr_t));
		*value = value_temp;
		*len = sizeof(rcu_addr_t);
		return 0;
	}else if(RCU_ADDRESS2 == key_id){
		
		value_temp = (uint8_t*)tuya_ble_malloc(sizeof(rcu_addr_t));
		if(NULL == value_temp)
		{
		    TUYA_LOG_E(MOD_DEV, "wd_common_read malloc failed");
		    *value = NULL;
		    return 1;
		}
	    memcpy(value_temp,&temp.rcu_addr[1],sizeof(rcu_addr_t));
		*value = value_temp;
		*len = sizeof(rcu_addr_t);
		return 0;
	}else if(DEV_CFG_PARAM == key_id){

	    if(0xFFFFFFFF == temp.dev_config.wheel)
    	{
    	    TUYA_LOG_I(MOD_DEV, "DEV_CFG_PARAM Invalid data");
    	    *value =  NULL;
			return 0;
    	}
		value_temp = (uint8_t*)tuya_ble_malloc(sizeof(tuya_dev_cfg_t));
		if(NULL == value_temp)
		{
		    TUYA_LOG_E(MOD_DEV, "wd_common_read malloc failed");
		    *value = NULL;
		    return 1;
		}
	    memcpy(value_temp,&temp.dev_config,sizeof(tuya_dev_cfg_t));
		*value = value_temp;
		*len = sizeof(tuya_dev_cfg_t);
		return 0;
	}
    TUYA_LOG_E(MOD_DEV, "wd_common_read malloc not map one keyID");
	*value =  NULL;
	#endif
	return TUYA_OK;
}


