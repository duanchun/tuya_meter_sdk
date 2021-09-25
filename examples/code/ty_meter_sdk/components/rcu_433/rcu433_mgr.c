//#include "tuya_rcu433_mgr.h"
#include "rcu433_srv.h"
#include "tuya_ble_log.h"
#include "locdef_pin.h"
#include "tuya_kel_timer.h"
#include "vfm_sensor.h"
#include "vfm_lock.h"
#include "tuya_log.h"


#define LOCDEF_433_DATA_PIN         (EBIKE_433_DATA_GPIO) /* 433数据通信中断 */
#define LOCDEF_433_SHUT_PIN         (EBIKE_433_EN_GPIO) /* 433使能引脚 */

static tuya_kel_timer_handle s_433_learn_timer_id = NULL;

/**
 * @brief   遥控钥匙回调处理函数
 *
 * @param   VOID
 *
 * @return  无
 */
void locdef_rcu433_key_proc_cb(CALLBACK_TYPE type, int keyvalue)
{
    uint8_t lock_status = 0;
	ACC_POWER_E action = 0;
    TUYA_LOG_I(MOD_RCU433, " locdef_rcu433_key_proc_cb %d-%d", type, keyvalue);
    if (type == TYPE_KEY) {
        switch (keyvalue) {
            case RCU_KEY_LOCK:
                ty_vfm_sensor_event_post(SENSOR_EVENT_DEFENCE);
                break;
            case RCU_KEY_UNLOCK:
                ty_vfm_sensor_event_post(SENSOR_EVENT_UNDEFENCE);
                break;
            case RCU_KEY_POWERON:
				ty_vfm_lock_read(LOCK_TYPE_DEF_LOCK,&lock_status);
				ty_vfm_acc_read(&action);

				if(UNLOCK == lock_status)
				{
					ty_vfm_sensor_event_post(SENSOR_EVENT_REMOTE_ACCON);
				}else{
					if(ACC_POWER_OFF == action)
					{
						ty_vfm_sensor_event_post(SENSOR_EVENT_REMOTE_ACCON);
					}else{
                    	ty_vfm_sensor_event_post(SENSOR_EVENT_ACCOFF);
					}
			    }
                break;
            case RCU_KEY_RING:
                ty_vfm_sensor_event_post(SENSOR_EVENT_SEARCH);
                break;
            default:
                TUYA_LOG_I(MOD_RCU433, " unknown key value: 0x%02X", keyvalue);
            break;
        }
    }

}


static void ty_433_learn_timer_handler(tuya_ble_timer_t param)
{
    tuya_rcu433_learn_contrl(false);
}

void tuya_rcu_433_mgr_init(void)
{
	uint32_t ret = 0;
	uint16_t timeout_cnt = 0;
    uint8_t addr_cnt = 0;
    RCU_KEY_MAP_T s_keymap[4] = {
		{0x02,RCU_KEY_LOCK},
		{0x01,RCU_KEY_UNLOCK},
		{0x04,RCU_KEY_POWERON},
		{0x08,RCU_KEY_RING}
    };

    TUYA_LOG_I(MOD_RCU433, " user_device_init succ");
	
	ret = tuya_rcu433_service_init(locdef_rcu433_key_proc_cb, LOCDEF_433_DATA_PIN,
                                   LOCDEF_433_SHUT_PIN, EV1527_CODE_BIT_28);

    tuya_rcu433_keyvalue_register(s_keymap, 4);

	if (ret == 0) {
		TUYA_LOG_I(MOD_RCU433, " rcu433 service init successed %d",ret);
		
        ret = tuya_rcu433_service_start(50);
        if  (ret != 0) {
            TUYA_LOG_I(MOD_RCU433, " rcu433 service start failed %d",ret);
        }
        tuya_rcu433_get_addr_nums(&addr_cnt);
        TUYA_LOG_I(MOD_RCU433, " rcu433 addr_cunt: %d", addr_cnt);
		#if 0
        if (addr_cnt < 2) {
			tuya_rcu433_learn_contrl(true);
			tuya_kel_timer_create(&s_433_learn_timer_id, 5*1000, TY_KEL_TIMER_SINGLE_SHOT, ty_433_learn_timer_handler);
			if (s_433_learn_timer_id == NULL)
			{
				TUYA_LOG_E(MOD_PUBLIC,"create 433 learn timer failed");
			}else{
				tuya_kel_timer_start(s_433_learn_timer_id);
			}
			
        }
		#endif
    }else{
        TUYA_LOG_I(MOD_RCU433, " rcu433 service init failed %d",ret);
    }
	
}
