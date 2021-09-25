/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"

#include "os_timer.h"
#include "os_mem.h"
#include "os_task.h"
#include "sys_utils.h"
#include "button.h"
#include "jump_table.h"

#include "driver_plf.h"
#include "driver_system.h"
#include "driver_i2s.h"
#include "driver_pmu.h"
#include "driver_uart.h"
#include "driver_rtc.h"
#include "driver_flash.h"
#include "driver_efuse.h"
#include "flash_usage_config.h"

#include "simple_gatt_service.h"    
#include "tuya_ble_api.h"
#include "ty_uart.h"
#include "ty_ble.h"
#include "tuya_ble_log.h"
#include "ty_ble_config.h"

typedef enum  {
    TASK_EVT_AT_COMMAND,
    TASK_EVT_BUTTON,
    TASK_EVT_TY_MAINLOOP,
}task_event_t;

typedef void (*ty_ble_evt_cb)(tuya_ble_cb_evt_param_t* event,void* param);
typedef struct {
	tuya_ble_cb_evt_t evt_type;
	ty_ble_evt_cb cbk;
}ty_ble_evt_tab_t;



extern uint16_t g_ty_app_task_id;
static os_timer_t s_ty_sdk_heart_driver_timer = {0};
static uint16_t s_ty_app_task_handle = 0;
static tuya_ble_timer_t s_disconnect_timer;
static tuya_ble_timer_t s_update_conn_param_timer;


static tuya_ble_device_param_t s_tuya_ble_device_param = {
    .use_ext_license_key = 0, //1-info in tuya_ble_sdk_demo.h, 0-auth info
    .device_id_len       = 0, //DEVICE_ID_LEN,
    .p_type              = TUYA_BLE_PRODUCT_ID_TYPE_PID,
    .product_id_len      = 8,
    .adv_local_name_len  = TUYA_BLE_ADV_LOCAL_NAME_MAX_LEN,
    .firmware_version    = TY_DEVICE_FVER_NUM,
    .hardware_version    = TY_DEVICE_HVER_NUM,
};

static void evt_connect_status_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_dp_data_send_response_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_dp_data_with_time_send_response_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_time_stamp_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_time_normal_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_unbond_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_anomaly_unbond_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_dev_reset_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_unbind_reset_response_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_dp_data_received_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_dp_query_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_ota_data_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_bulk_data_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_data_passthrough_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_weather_data_req_response_cb(tuya_ble_cb_evt_param_t* event,void* param);
static void evt_weather_data_received_cb(tuya_ble_cb_evt_param_t* event,void* param);



static const ty_ble_evt_tab_t s_ty_ble_evt_tab[] = {
	{TUYA_BLE_CB_EVT_CONNECTE_STATUS,evt_connect_status_cb},
	{TUYA_BLE_CB_EVT_DP_DATA_SEND_RESPONSE,evt_dp_data_send_response_cb},
	{TUYA_BLE_CB_EVT_DP_DATA_WITH_TIME_SEND_RESPONSE,evt_dp_data_with_time_send_response_cb},
	{TUYA_BLE_CB_EVT_TIME_STAMP,evt_time_stamp_cb},
	{TUYA_BLE_CB_EVT_TIME_NORMAL,evt_time_normal_cb},
	{TUYA_BLE_CB_EVT_APP_LOCAL_TIME_NORMAL,evt_time_normal_cb},
	{TUYA_BLE_CB_EVT_UNBOUND,evt_unbond_cb},
	{TUYA_BLE_CB_EVT_ANOMALY_UNBOUND,evt_anomaly_unbond_cb},
	{TUYA_BLE_CB_EVT_DEVICE_RESET,evt_dev_reset_cb},
	{TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE,evt_unbind_reset_response_cb},
	{TUYA_BLE_CB_EVT_DP_QUERY,evt_dp_query_cb},
	{TUYA_BLE_CB_EVT_OTA_DATA,evt_ota_data_cb},
	{TUYA_BLE_CB_EVT_BULK_DATA,evt_bulk_data_cb},
	{TUYA_BLE_CB_EVT_DATA_PASSTHROUGH,evt_data_passthrough_cb},
	{TUYA_BLE_CB_EVT_WEATHER_DATA_REQ_RESPONSE,evt_weather_data_req_response_cb},
	{TUYA_BLE_CB_EVT_WEATHER_DATA_RECEIVED,evt_weather_data_received_cb},
	{TUYA_BLE_CB_EVT_DP_DATA_RECEIVED,evt_dp_data_received_cb}
};

void tuya_ble_disconnect_and_reset_timer_start(void)
{
    tuya_ble_timer_start(s_disconnect_timer);
}

static void tuya_ble_update_conn_param_timer_start(void)
{
    tuya_ble_timer_start(s_update_conn_param_timer);
}

static void evt_connect_status_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
	#if TUYA_BLE_SDK_TEST
    tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_DEVICE_STATE, (void*)&event->connect_status, sizeof(uint8_t));
	#endif
    
    if(event->connect_status == BONDING_CONN) {
        TUYA_APP_LOG_INFO("bonding and connecting");
        
        tuya_ble_update_conn_param_timer_start();
    }
}

static void evt_dp_data_received_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
	TUYA_APP_LOG_INFO("receive dp data");
	ty_dp_data_received_handle(event->dp_received_data.p_data, event->dp_received_data.data_len); //TODO
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_WRITE, event->dp_received_data.p_data, event->dp_received_data.data_len);
#endif

}

static void evt_dp_data_send_response_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_dp_report_handler();
	tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_RSP, &event->dp_send_response_data.status, sizeof(uint8_t));
#endif
}

static void evt_dp_data_with_time_send_response_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_dp_report_handler();
	tuya_ble_sdk_test_send(TY_UARTV_CMD_DP_RSP, &event->dp_with_time_send_response_data.status, sizeof(uint8_t));
#endif
}

static void evt_time_stamp_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
	uint32_t timestamp_s = tuya_ble_ascii_to_int((char*)event->timestamp_data.timestamp_string, 10);
	uint32_t timestamp_ms = tuya_ble_ascii_to_int((char*)(event->timestamp_data.timestamp_string+10), 3);

	uint64_t timestamp = 0;
	timestamp = timestamp_s*1000 + timestamp_ms;

	ty_rtc_set_time(timestamp_s);

	TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_TIME_STAMP - time_zone: %d", event->timestamp_data.time_zone);
	TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_TIME_STAMP - timestamp: %d", timestamp_s);

#if TUYA_BLE_SDK_TEST
	tuya_ble_time_struct_data_t data = {0};;
	tuya_ble_utc_sec_2_mytime(timestamp_s, &data, false);

	tuya_ble_time_noraml_data_t normal_data = {0};
	memcpy(&normal_data, &data, sizeof(tuya_ble_time_struct_data_t));
	normal_data.time_zone = event->timestamp_data.time_zone;

	tuya_ble_sdk_test_get_time_rsp(&normal_data);
#endif

}

static void evt_time_normal_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_get_time_rsp(&event->time_normal_data);
#endif
}

static void evt_unbond_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_unbind_mode_rsp(0);
#endif
	ty_hid_bond_info_reset(); //TODO
	TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_UNBOUND");
}

static void evt_anomaly_unbond_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_unbind_mode_rsp(1);
#endif
	ty_hid_bond_info_reset(); //TODO
	TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_ANOMALY_UNBOUND");

}

static void evt_dev_reset_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_unbind_mode_rsp(2);
#endif
	ty_hid_bond_info_reset(); //TODO
	TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_DEVICE_RESET");

}

static void evt_unbind_reset_response_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	if(event->reset_response_data.type == RESET_TYPE_UNBIND) {
		if(event->reset_response_data.status == 0) {
			tuya_ble_sdk_test_unbind_mode_rsp(3);
		}
	} else if(event->reset_response_data.type == RESET_TYPE_FACTORY_RESET) {
		if(event->reset_response_data.status == 0) {
			tuya_ble_sdk_test_unbind_mode_rsp(4);
		}
	}
#endif

	TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_UNBIND_RESET_RESPONSE");

}

static void evt_dp_query_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
	TUYA_APP_LOG_INFO("Query DP Data");
	ty_dp_query_req();//TODO
	TUYA_APP_LOG_HEXDUMP_INFO("TUYA_BLE_CB_EVT_DP_QUERY", event->dp_query_data.p_data, event->dp_query_data.data_len);
}

static void evt_ota_data_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
	tuya_ble_ota_handler(&event->ota_data);
}

static void evt_bulk_data_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
	TUYA_APP_LOG_INFO("TUYA_BLE_CB_EVT_BULK_DATA");
	//tuya_ble_bulk_data_demo_handler(&event->bulk_req_data);
}

static void evt_data_passthrough_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	tuya_ble_sdk_test_send(TY_UARTV_CMD_PASSTHROUGH_WRITE, event->ble_passthrough_data.p_data, event->ble_passthrough_data.data_len);
#endif
}

static void evt_weather_data_req_response_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
	uint8_t  rsp_data[2] = {1};
	uint32_t rsp_len = 2;
	rsp_data[1] = event->weather_req_response_data.status;
	tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_WEATHER, rsp_data, rsp_len);
	TUYA_APP_LOG_INFO("received weather data request response result code =%d",event->weather_req_response_data.status);
#endif

}

static void evt_weather_data_received_cb(tuya_ble_cb_evt_param_t* event,void* param)
{
#if TUYA_BLE_SDK_TEST
    tuya_ble_wd_object_t *object;
    uint16_t object_len = 0;
    for (;;) {
        object = (tuya_ble_wd_object_t *)(event->weather_received_data.p_data + object_len);

        TUYA_APP_LOG_DEBUG("recvived weather data, n_days=[%d] key=[0x%08x] val_type=[%d] val_len=[%d]", \
                        object->n_day, object->key_type, object->val_type, object->value_len); 
        TUYA_APP_LOG_HEXDUMP_DEBUG("vaule :", (uint8_t *)object->vaule, object->value_len);	

        // TODO .. YOUR JOBS 
        
        object_len += (sizeof(tuya_ble_wd_object_t) + object->value_len);
        if (object_len >= event->weather_received_data.data_len)
            break;
    }
    
    tuya_ble_sdk_test_send(TY_UARTV_CMD_GET_WEATHER, event->weather_received_data.p_data, event->weather_received_data.data_len);
	#endif
}


static void tuya_ble_disconnect_and_reset_timer_cb(tuya_ble_timer_t timer)
{
	TUYA_APP_LOG_INFO("tuya_ble_disconnect_and_reset_timer_cb enter");
    tuya_ble_gap_disconnect();
    //tuya_ble_device_delay_ms(100);
    tuya_ble_device_delay_ms(1000);//TODO
    tuya_ble_device_reset();
}

static void tuya_ble_update_conn_param_timer_cb(tuya_ble_timer_t timer)
{
    ty_ble_set_conn_param(TY_CONN_INTERVAL_MIN, TY_CONN_INTERVAL_MAX, 0, 6000);
}

const struct jump_table_version_t _jump_table_version __attribute__((section("jump_table_3"))) = {
    .firmware_version = 0x00000015,//TODO 
};

const struct jump_table_image_t _jump_table_image __attribute__((section("jump_table_1"))) = {
    .image_type = IMAGE_TYPE_APP,
    .image_size = 0x32000,      
};

/*********************************************************************
 * @fn      user_custom_parameters
 *
 * @brief   initialize several parameters, this function will be called 
 *          at the beginning of the program. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_custom_parameters(void)
{
    struct chip_unique_id_t id_data;

    efuse_get_chip_unique_id(&id_data);
    __jump_table.addr.addr[0] = 0xBD;
    __jump_table.addr.addr[1] = 0xAD;
    __jump_table.addr.addr[2] = 0xD0;
    __jump_table.addr.addr[3] = 0xF0;
    __jump_table.addr.addr[4] = 0x17;
    __jump_table.addr.addr[5] = 0xc0;
    
    id_data.unique_id[5] |= 0xc0; // random addr->static addr type:the top two bit must be 1.
    memcpy(__jump_table.addr.addr, id_data.unique_id, 6);
    __jump_table.system_clk = SYSTEM_SYS_CLK_12M; //SYSTEM_SYS_CLK_48M;
    __jump_table.system_option |= SYSTEM_OPTION_UP_PARAM_REQ_REPORT;
    jump_table_set_static_keys_store_offset(JUMP_TABLE_STATIC_KEY_OFFSET);
    
    system_optimize_power_consumption_set(1);
    
    retry_handshake();
}

/*********************************************************************
 * @fn      user_entry_before_sleep_imp
 *
 * @brief   Before system goes to sleep mode, user_entry_before_sleep_imp()
 *          will be called, MCU peripherals can be configured properly before 
 *          system goes to sleep, for example, some MCU peripherals need to be
 *          used during the system is in sleep mode. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_before_sleep_imp(void)
{
//	uart_putc_noint_no_wait(UART1, 's');
}

/*********************************************************************
 * @fn      user_entry_after_sleep_imp
 *
 * @brief   After system wakes up from sleep mode, user_entry_after_sleep_imp()
 *          will be called, MCU peripherals need to be initialized again, 
 *          this can be done in user_entry_after_sleep_imp(). MCU peripherals
 *          status will not be kept during the sleep. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void user_entry_after_sleep_imp(void)
{
//	uart_putc_noint_no_wait(UART1, 's');
                
    ty_uart2_init();

    NVIC_EnableIRQ(PMU_IRQn);

	#if 0 //TODO
    os_event_t evt;
    evt.event_id = TASK_EVT_TY_MAINLOOP;
    evt.param = NULL;
    evt.param_len = 0;
    os_msg_post(g_ty_app_task_id, &evt);
	#endif
}

/*********************************************************************
 * @fn      user_entry_before_ble_init
 *
 * @brief   Code to be executed before BLE stack to be initialized.
 *          Power mode configurations, PMU part driver interrupt enable, MCU 
 *          peripherals init, etc. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_entry_before_ble_init(void)
{    
    /* set system power supply in BUCK mode */
    pmu_set_sys_power_mode(PMU_SYS_POW_BUCK);
    
#ifdef FLASH_PROTECT
    flash_protect_enable(1);
#endif
    
    pmu_enable_irq(PMU_ISR_BIT_ACOK
                   | PMU_ISR_BIT_ACOFF
                   | PMU_ISR_BIT_ONKEY_PO
                   | PMU_ISR_BIT_OTP
                   | PMU_ISR_BIT_LVD
                   | PMU_ISR_BIT_BAT
                   | PMU_ISR_BIT_ONKEY_HIGH);
    NVIC_EnableIRQ(PMU_IRQn);
    
    ty_system_init(0);
}

static void ty_ble_msg_callback(tuya_ble_cb_evt_param_t* event)
{
	//TUYA_APP_LOG_INFO("callback: %02x", event->evt);
 	uint8_t local_evt_size = sizeof(s_ty_ble_evt_tab)/sizeof(ty_ble_evt_tab_t);
	int index = 0;
	uint8_t flag = 0;

	for(index = 0;index < local_evt_size;index++)
	{
		if(event->evt == s_ty_ble_evt_tab[index].evt_type)
		{
			if(NULL != s_ty_ble_evt_tab[index].cbk)
			{
				s_ty_ble_evt_tab[index].cbk(event,NULL);
			}
			flag = 1;
			break;
		}
	}

	if(0 == flag)
	{
		TUYA_APP_LOG_INFO("tuya_ble_sdk_callback unknown event type 0x%04x", event->evt);
	}
	
}


static void ty_ble_sdk_init(void)
{
	memcpy(s_tuya_ble_device_param.device_id,       TY_DEVICE_DID, DEVICE_ID_LEN);
    memcpy(s_tuya_ble_device_param.auth_key,        TY_DEVICE_AUTH_KEY,  AUTH_KEY_LEN);
    memcpy(s_tuya_ble_device_param.mac_addr_string, TY_DEVICE_MAC,  MAC_STRING_LEN);
    memcpy(s_tuya_ble_device_param.product_id,      TY_DEVICE_PID,  s_tuya_ble_device_param.product_id_len);
    memcpy(s_tuya_ble_device_param.adv_local_name,  TY_DEVICE_NAME, s_tuya_ble_device_param.adv_local_name_len);
    tuya_ble_sdk_init(&s_tuya_ble_device_param);
    
    tuya_ble_callback_queue_register(ty_ble_msg_callback);
	TUYA_APP_LOG_INFO("ty_ble_sdk_init successed");
}

void tuya_mainloop_timer_start(void)
{
    os_timer_start(&s_ty_sdk_heart_driver_timer, 20, 1);
}

void tuya_mainloop_timer_stop(void)
{
    os_timer_stop(&s_ty_sdk_heart_driver_timer);
}

//tuya_ble_sdk 事件处理任务心跳驱动
static void ty_event_handle_heart_drive_cbk(void *arg)
{
    os_event_t evt;
    evt.event_id = TASK_EVT_TY_MAINLOOP;
    evt.param = NULL;
    evt.param_len = 0;
    os_msg_post(s_ty_app_task_handle, &evt);
}

static int ty_ble_sdk_core_exec_task_proc(os_event_t *param)
{
    switch(param->event_id)
    {
        case TASK_EVT_BUTTON: {
            struct button_msg_t *button_msg;
            const char *button_type_str[] = {
                                                "BUTTON_PRESSED",
                                                "BUTTON_RELEASED",
                                                "BUTTON_SHORT_PRESSED",
                                                "BUTTON_MULTI_PRESSED",
                                                "BUTTON_LONG_PRESSED",
                                                "BUTTON_LONG_PRESSING",
                                                "BUTTON_LONG_RELEASED",
                                                "BUTTON_LONG_LONG_PRESSED",
                                                "BUTTON_LONG_LONG_RELEASED",
                                                "BUTTON_COMB_PRESSED",
                                                "BUTTON_COMB_RELEASED",
                                                "BUTTON_COMB_SHORT_PRESSED",
                                                "BUTTON_COMB_LONG_PRESSED",
                                                "BUTTON_COMB_LONG_PRESSING",
                                                "BUTTON_COMB_LONG_RELEASED",
                                                "BUTTON_COMB_LONG_LONG_PRESSED",
                                                "BUTTON_COMB_LONG_LONG_RELEASED",
                                            };

            button_msg = (struct button_msg_t *)param->param;
            
			//TY_PRINTF("KEY 0x%08x, TYPE %s.", button_msg->button_index, button_type_str[button_msg->button_type]);
            
            if(button_msg->button_type == 0) {
                
				#if TUYA_BLE_SDK_TEST
                system_sleep_disable();
                
                ty_uart2_init();
                
                TY_PRINTF("button pressed");
                
                tuya_ble_sdk_test_wake_up_handler();
                
                tuya_mainloop_timer_start();
				#endif
            }
        } break;
        
        case TASK_EVT_TY_MAINLOOP: {
			//ty_ble_wdt_feed(); //TODO
            ty_system_mainloop();
			ty_vfm_multi_upload_proc();  //TODO
        } break;
    }

    return EVT_CONSUMED;//import
}


static void tuya_debug_log_print(void)
{
	ty_ble_mac_t mac = {0};
	extern tuya_ble_parameters_settings_t tuya_ble_current_para;
	
	ty_ble_get_mac(&mac);
	TY_HEXDUMP("Local bdaddr", mac.addr, 6);
	
    TUYA_APP_LOG_HEXDUMP_INFO("auth key", tuya_ble_current_para.auth_settings.auth_key, AUTH_KEY_LEN);
    TUYA_APP_LOG_HEXDUMP_INFO("device id", tuya_ble_current_para.auth_settings.device_id, DEVICE_ID_LEN);
    TUYA_APP_LOG_INFO("Device Bond(%d)", tuya_ble_current_para.sys_settings.bound_flag);

	TUYA_APP_LOG_INFO("system heap free:%u", os_get_free_heap_size());
}


/*********************************************************************
 * @fn      user_entry_after_ble_init
 *
 * @brief   Main entrancy of user application. This function is called after BLE stack
 *          is initialized, and all the application code will be executed from here.
 *          In that case, application layer initializtion can be startd here. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_entry_after_ble_init(void)
{
	system_sleep_disable();
    
    ty_system_init(1);
	ty_ble_sdk_init();
	tuya_ble_ota_init();
	
    tuya_app_init();
    // Application layer initialization, can included bond manager init, 
    // advertising parameters init, scanning parameter init, GATT service adding, etc.
	peripheral_init();

#if TUYA_BLE_SDK_TEST
    tuya_ble_sdk_test_init();
#endif
	
	s_ty_app_task_handle = os_task_create(ty_ble_sdk_core_exec_task_proc);
	if (s_ty_app_task_handle == TASK_ID_FAIL)
	{
		TUYA_APP_LOG_ERROR("start thread ty_app_task_proc err");
		return ;
	}

    os_timer_init(&s_ty_sdk_heart_driver_timer, ty_event_handle_heart_drive_cbk, NULL);
    tuya_mainloop_timer_start();

	tuya_ble_timer_create(&s_disconnect_timer, 1000, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_disconnect_and_reset_timer_cb);
    tuya_ble_timer_create(&s_update_conn_param_timer, 1000, TUYA_BLE_TIMER_SINGLE_SHOT, tuya_ble_update_conn_param_timer_cb);
    
    pmu_set_aldo_voltage(PMU_ALDO_MODE_NORMAL, PMU_ALDO_VOL_3_3);
	//����
    //system_sleep_enable();
    //system_power_off(true);

	tuya_debug_log_print();
}

