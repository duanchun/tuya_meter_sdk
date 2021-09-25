#include <stdio.h>
#include <string.h>
#include "ev1527.h"
#include "tuya_ble_queue.h"
#include "rcu433_srv.h"
#include "tuya_ble_log.h"
#include "tuya_kel_os.h"
#include "tuya_log.h"

static RCU_KEY_MAP_T s_keymap[4] = {
    {0x02,RCU_KEY_LOCK},
    {0x01,RCU_KEY_UNLOCK},
    {0x04,RCU_KEY_POWERON},
    {0x08,RCU_KEY_RING}
};
static TY_RCU433_CALLBACK   s_rcu_callback = NULL;

static rcu_addr_t           s_rcu_addr[2];
static uint8_t              s_learn_store_index = 0;
static uint32_t               s_timer_us = 60;

static tuya_kel_task_handle s_rcu433_task_handle = 0;

static RCU_KEY_VALUE_E tuya_rcu_get_logic_key(uint32_t phykey)
{
    uint8_t i = 0;
    for (i = 0; i < 4; i ++) {
        if (s_keymap[i].phy_vaule == phykey) {
            return s_keymap[i].logic_key;
        }
    }
    return RCU_KEY_INVAILD;
}

int32_t tuya_rcu433_get_addr_nums(uint8_t *addr_cnt)
{
    uint32_t buff_len = 0;
    uint8_t *buff = NULL;
    uint8_t local_addr_cnt = 0;
    int32_t op_ret = 0;
    op_ret = tuya_mid_wd_common_read(RCU_ADDRESS1,&buff,&buff_len);
    if (buff) {
        memcpy(&s_rcu_addr[0],buff,sizeof(rcu_addr_t));
        tuya_ble_free(buff);
        buff = NULL;
        TUYA_LOG_I(MOD_RCU433, "read addr1 addr=0x%02x,0x%02x,0x%02x",s_rcu_addr[0].addr[0],
            s_rcu_addr[0].addr[1],
            s_rcu_addr[0].addr[2]);
    }
    tuya_mid_wd_common_read(RCU_ADDRESS2,&buff,&buff_len);//TODO
    if (buff) {
        memcpy(&s_rcu_addr[1],buff,sizeof(rcu_addr_t));
        tuya_ble_free(buff);
        TUYA_LOG_I(MOD_RCU433, "read addr2 addr=0x%02x,0x%02x,0x%02x",s_rcu_addr[1].addr[0],
            s_rcu_addr[1].addr[1],
            s_rcu_addr[1].addr[2]);
        buff = NULL;
    }
    if (s_rcu_addr[0].addr_flag != 0x55) {
        memset(&s_rcu_addr[0],0,sizeof(rcu_addr_t));
    } else {
        local_addr_cnt++;
    }
    if (s_rcu_addr[1].addr_flag != 0x55) {
        memset(&s_rcu_addr[1],0,sizeof(rcu_addr_t));
    } else {
        local_addr_cnt++;
    }

    if (addr_cnt) {
        *addr_cnt = local_addr_cnt;
    }

    return op_ret;
}


static bool tuya_rcu433_verify_key(uint8_t rcu_addr[3])
{
    uint8_t i = 0;
    for(i = 0; i < 2; i ++) {
		#if 1
		TUYA_LOG_I(MOD_RCU433, "[%d][0x%X][0x%02X::0x%02X::0x%02X]",i+1,s_rcu_addr[i].addr_flag,s_rcu_addr[i].addr[0],
			s_rcu_addr[i].addr[1],s_rcu_addr[i].addr[2]);
		#endif
        if (s_rcu_addr[i].addr_flag == 0x55) {
            if (memcmp(rcu_addr,s_rcu_addr[i].addr,3) == 0) {
                return true;
            }
        }
    }
    TUYA_LOG_I(MOD_RCU433, "tuya_rcu433_verify_key fail");
    return false;
}

static void tuya_rcu433_learn(uint8_t recv_addr[3])
{
    if (s_learn_store_index > 1) {
        return ;
    }
    if (memcmp(s_rcu_addr[s_learn_store_index].addr,recv_addr,3)) {
        if (s_learn_store_index == 0) {

            s_rcu_addr[s_learn_store_index].addr_flag = 0x55;
            memcpy(s_rcu_addr[s_learn_store_index].addr,recv_addr,3);
            tuya_mid_wd_common_write(RCU_ADDRESS1,(uint8_t*)&s_rcu_addr[s_learn_store_index],sizeof(rcu_addr_t));//TODO
            s_learn_store_index ++;
            TUYA_LOG_I(MOD_RCU433, "write addr1 addr=0x%02x,0x%02x,0x%02x",recv_addr[0],recv_addr[1],recv_addr[2]);
        }
        else if (s_learn_store_index == 1) {
            s_rcu_addr[s_learn_store_index].addr_flag = 0x55;
            memcpy(s_rcu_addr[s_learn_store_index].addr,recv_addr,3);
            tuya_mid_wd_common_write(RCU_ADDRESS2,(uint8_t*)&s_rcu_addr[s_learn_store_index],sizeof(rcu_addr_t));//TODO
            TUYA_LOG_I(MOD_RCU433, "write addr2 addr=0x%02x,0x%02x,0x%02x",recv_addr[0],recv_addr[1],recv_addr[2]);
            s_learn_store_index = 0;
        }
     }
}

static int rcu433_msg_handle(void *param)
{
    #define REPEAD_TIME_EVT   300//200 //ms
    TUYA_RET_E ret = TUYA_OK;
    static uint32_t recv_time = 0;
    uint32_t current_time = 0;
    RCU_KEY_VALUE_E logic_key_val = 0;
	RCU_MSG_T rcu_msg = {0};

	ret = tuya_kel_wait_msg(s_rcu433_task_handle,(void*)&rcu_msg,0);
	if(TUYA_OK != ret)
	{
	    TUYA_LOG_W(MOD_SERVER, "tuya_kel_wait_msg failed");
		goto END;
	}

	
    switch (rcu_msg.type)
    {
        case RCU_START_MONITOR:
            TUYA_LOG_I(MOD_RCU433, "RCU_START_MONITOR");
            tuya_ev1527_start(s_timer_us);
            break;
        case RCU_STOP_MONITOR:
            TUYA_LOG_I(MOD_RCU433, "RCU_STOP_MONITOR");
            tuya_ev1527_stop();
            break;
        case RCU_PAUSE_MONITOR:
            break;
        case RCU_RESUME_MONITOR:
            break;
        case RCU_LEARN_START:
			TUYA_LOG_I(MOD_RCU433, "RCU_LEARN_START");
            tuya_ev1527_set_learn_mode(true);
            break;
        case RCU_LEARN_STOP:
			TUYA_LOG_I(MOD_RCU433, "RCU_LEARN_STOP");
            tuya_ev1527_set_learn_mode(false);
            break;
        case RCU_GET_KEY:
			TUYA_LOG_I(MOD_RCU433, "RCU_GET_KEY");
            tuya_ev1527_pause();
			current_time = system_get_curr_time();
            if (current_time - recv_time > REPEAD_TIME_EVT) {
                recv_time = current_time;
            } else {
                TUYA_LOG_I(MOD_RCU433, "RCU_GET_KEY interval < %d [%d]",REPEAD_TIME_EVT,current_time - recv_time);
                tuya_ev1527_resume(s_timer_us);
                break;
            }

            TUYA_LOG_I(MOD_RCU433, "RCU_GET_KEY keyvalue = 0x%02x addr = 0x%02x,0x%02x,0x%02x",
                                rcu_msg.keyvalue,rcu_msg.rcu_addr[0],rcu_msg.rcu_addr[1],rcu_msg.rcu_addr[2]);


            if (tuya_rcu433_verify_key(rcu_msg.rcu_addr)) {
                logic_key_val = tuya_rcu_get_logic_key(rcu_msg.keyvalue);
                if (logic_key_val != RCU_KEY_INVAILD) {
                    if (s_rcu_callback) {
                        s_rcu_callback(TYPE_KEY, logic_key_val);
                    }
                } else {
                    TUYA_LOG_I(MOD_RCU433, "RCU_GET_KEY get logic key fail");
                }
                
            }
            tuya_ev1527_resume(s_timer_us);
            break;
        case RCU_LEARN_SET:
			TUYA_LOG_I(MOD_RCU433, "RCU_LEARN_SET");

            tuya_ev1527_pause();
			current_time = system_get_curr_time();
            if (current_time - recv_time > REPEAD_TIME_EVT) {
                recv_time = current_time;
            }
            else {
				//丢弃200ms内的 重复学习 事件
                TUYA_LOG_I(MOD_RCU433, "RCU_LEARN_SET interval < %d ",REPEAD_TIME_EVT);
                tuya_ev1527_resume(s_timer_us);
                break;
            }
            TUYA_LOG_I(MOD_RCU433, "RCU_LEARN_SET addr = 0x%02x,0x%02x,0x%02x",rcu_msg.rcu_addr[0],
                            rcu_msg.rcu_addr[1],rcu_msg.rcu_addr[2]);
            tuya_rcu433_learn(rcu_msg.rcu_addr);
            if (s_rcu_callback) {
                s_rcu_callback(TYPE_LEARN,rcu_msg.keyvalue);
            }
            tuya_ev1527_resume(s_timer_us);
            break;
        default:
            break;
    }
END:
	//TUYA_LOG_I(MOD_RCU433, "system heap free:%u", os_get_free_heap_size());
	return 0;//must return 0
}
// 内部使用
int tuya_rcu433_msg_send_internal(RCU_MSG_T rcu_msg)
{
	TUYA_LOG_I(MOD_RCU433, "dpFun:%04x",rcu_msg.type);
	
    tuya_kel_send_msg(s_rcu433_task_handle,&rcu_msg,0);
	return 0;
}
void tuya_rcu433_keyvalue_register(RCU_KEY_MAP_T *map,uint8_t cnt)
{
    if (cnt > sizeof(s_keymap)/sizeof(RCU_KEY_MAP_T)) {
        return;
    }
    if (map) {
        memcpy(s_keymap,map,cnt);
    }
}

int32_t tuya_rcu433_service_start(uint32_t timeout_us)
{
    RCU_MSG_T rcu_msg;

    s_timer_us = timeout_us;
    rcu_msg.type = RCU_START_MONITOR;

    return tuya_rcu433_msg_send_internal(rcu_msg);
}

int32_t tuya_rcu433_service_stop(void)
{
    RCU_MSG_T rcu_msg;

    rcu_msg.type = RCU_STOP_MONITOR;

    return tuya_rcu433_msg_send_internal(rcu_msg);
}

int32_t tuya_rcu433_learn_contrl(bool enable)
{
    RCU_MSG_T rcu_msg;
    if (enable) {
        rcu_msg.type = RCU_LEARN_START;
    }
    else {
        rcu_msg.type = RCU_LEARN_STOP;
    }
    s_learn_store_index = 0;
    return tuya_rcu433_msg_send_internal(rcu_msg);
}

int32_t tuya_rcu433_service_init(TY_RCU433_CALLBACK cb,uint8_t data_pin,uint8_t shut_pin,
                                    RCU_CODE_BITS_E bits)
{
    int32_t op_ret = 0;
    s_rcu_callback = cb;

	s_rcu433_task_handle = (void*)tuya_kel_task_create(rcu433_msg_handle,NULL,sizeof(RCU_MSG_T),10,0,0,NULL);
	if (s_rcu433_task_handle == NULL)
	{
		TUYA_LOG_E(MOD_PUBLIC,"create task rcu433_msg_handle err,rcu433_service_init failed");
		return 1;
	}
	
    tuya_rcu433_get_addr_nums(NULL);
    tuya_ev1527_set_code_bits(bits);
    op_ret = tuya_ev1527_init(data_pin,shut_pin);
    TUYA_LOG_I(MOD_RCU433, "rcu433_service_init successed");
    return op_ret;
}


