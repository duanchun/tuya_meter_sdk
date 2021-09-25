
#include "locdef_dp.h"
#include "tuya_ble_mutli_tsf_protocol.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_log.h"
#include "locdef_main.h"
#include "vfm_sensor.h"
#include "vfm_lock.h"
#include "vfm_upload.h"
#include "locdef_service.h"
#include "locdef_audio.h"
#include "tuya_log.h"

static VFM_UPLOAD_CFG_T g_upload_cfg = {0};


typedef void (*evt_db_handle_cb)(const ty_dp_header_t *dp_heade,const uint8_t *data);

typedef struct {
	uint8_t dpid;
	evt_db_handle_cb cbk;
}evt_handle_tab_t;

static uint32_t s_pd_report_sn         = 0;

void locdef_dp_update(uint8_t dpid, uint8_t type,uint16_t len,void *param);
int16_t ty_dp_report_proc(ty_dp_data_t *dp_data,void *param);
static bool dp_cmd_search_handle(bool data);



static void evt_dp_motor_clock_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_edoor_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_battery_level_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_rssi_sensitivity_set_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_far_lock_close_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_near_unlock_open_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_query_bond_status_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_cushiow_ctrl_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_search_cb(const ty_dp_header_t *dp_header,const uint8_t *data);
static void evt_dp_wheel_diamete_cb(const ty_dp_header_t *dp_header,const uint8_t *data);


static const evt_handle_tab_t s_evt_dp_tab[] = {
	//电机锁开关[下发 上报] 
	{DPID_DEF_CTRL,evt_dp_motor_clock_switch_cb},
	//电门开关[下发 上报] 
	{DPID_ACC_CTRL,evt_dp_edoor_switch_cb},
	//电池电量[下上报] 
	{DPID_POWER_LEVEL,evt_dp_battery_level_cb},
	//自动锁车距离 灵敏度设置[下发 上报]           
	{DPID_RSSI_SENSITIVITY,evt_dp_rssi_sensitivity_set_cb},
	//远离锁车开关[下发] 
	{DPID_FAR_LOCK_CLOSE_SWITCH,evt_dp_far_lock_close_switch_cb},
	//靠近解锁开关[下发] 
	{DPID_NEAR_UNLOCK_OPEN_SWITCH,evt_dp_near_unlock_open_switch_cb},
	//设备HID绑定状态[上报] 
	{DPID_QUERY_BOND_STATUS,evt_dp_query_bond_status_cb},
	//坐桶锁控制[上报] 
	{DPID_CUSHION_CTRL,evt_dp_cushiow_ctrl_cb},
	//寻车[下发 上报] 
	{DPID_SEARCH,evt_dp_search_cb},
	//轮径[下发 上报] 
	{DPID_WHEEL_DIAMETE,evt_dp_wheel_diamete_cb}
};

static void evt_dp_search_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
    uint8_t bool_res = dp_cmd_search_handle(data[0]);
    locdef_dp_update(dp_header->dpid,dp_header->type,dp_header->len,(void*)&bool_res);
}


static void evt_dp_cushiow_ctrl_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
	if(dp_header->len != 0) {
		if(TY_SWITCH_OFF == data[0])//
		{
			
		}else{//
			ty_vfm_sensor_event_post(SENSOR_EVENT_CUSHION);
		}
	}
	locdef_dp_update(dp_header->dpid,dp_header->type,dp_header->len,(void*)&data[0]);
}


static void evt_dp_motor_clock_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
    //对设备状态有影响，需要抛到 FSM处理
	if(dp_header->len != 0) {
		if(TY_SWITCH_ON == data[0])//解锁
		{
			ty_vfm_sensor_event_post(SENSOR_EVENT_UNDEFENCE);
		}else{//上锁
			ty_vfm_sensor_event_post(SENSOR_EVENT_DEFENCE);
		}
	}
}

static void evt_dp_edoor_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
	if(dp_header->len != 0) {
		if(TY_SWITCH_ON == data[0])//打开电门
		{
			ty_vfm_sensor_event_post(SENSOR_EVENT_REMOTE_ACCON);
		}else{
			ty_vfm_sensor_event_post(SENSOR_EVENT_ACCOFF);
		}
		
    }
}

static void evt_dp_battery_level_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
}

static void evt_dp_rssi_sensitivity_set_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
    DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();
	if(dp_header->len != 0) {
		TUYA_LOG_I(MOD_DP, "Sensitivity Level = %d\n", data[0]);
		switch(data[0])
		{
		    case 0:
				run_param->dev_hid.sensitivty = RSSI_OPEN_LOW;
				break;
			case 1:
				run_param->dev_hid.sensitivty = RSSI_OPEN_MIDDLE;
				break;
			case 2:
				run_param->dev_hid.sensitivty = RSSI_OPEN_HIGH;
				break;
			default:
				run_param->dev_hid.sensitivty = RSSI_OPEN_MIDDLE;
			    break;
		}
		
		ty_hid_bond_refresh(run_param->dev_hid.key);
		locdef_dp_update(dp_header->dpid,dp_header->type,dp_header->len,(void*)&data[0]);
	}
}

static void evt_dp_far_lock_close_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
    DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();
	
	if(dp_header->len != 0) {
	   if(data[0]) {
			TUYA_LOG_I(MOD_DP, "Far-Lock Open\n");
			run_param->dev_hid.far_lock_switch = TY_SWITCH_ON;
			if(TY_HID_DEV_UNBOND == run_param->hid_dev_bond ) {
				run_param->hid_svc_change_on = TY_SWITCH_ON;
				ble_peripheral_bond_request();
			}else{
				ty_hid_bond_refresh(run_param->dev_hid.key);
			}
		}else {
			TUYA_LOG_I(MOD_DP, "Far-Lock shut down\n");
			run_param->dev_hid.far_lock_switch = TY_SWITCH_OFF;
			ty_hid_bond_refresh(run_param->dev_hid.key);
		}
		locdef_dp_update(dp_header->dpid,dp_header->type,dp_header->len,(void*)&data[0]);
	}
}

static void evt_dp_near_unlock_open_switch_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
    DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();
	
	if(dp_header->len != 0) {
        if(data[0]) {
            TUYA_LOG_I(MOD_DP, "Near-Unlock Open\n");
            run_param->dev_hid.near_unlock_switch = TY_SWITCH_ON;
            if(TY_HID_DEV_UNBOND == run_param->hid_dev_bond) {
                run_param->hid_svc_change_on = TY_SWITCH_ON;
                ble_peripheral_bond_request();
            }else{
				ty_hid_bond_refresh(run_param->dev_hid.key);
            }
        }else {
            TUYA_LOG_I(MOD_DP, "Near-Unlock close\n");
            run_param->dev_hid.near_unlock_switch = TY_SWITCH_OFF;
			ty_hid_bond_refresh(run_param->dev_hid.key);
        }
        
        locdef_dp_update(dp_header->dpid,dp_header->type,dp_header->len,(void*)&data[0]);
    }
}

static void evt_dp_wheel_diamete_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
	if(dp_header->len != 0) {
		if(TUYA_OK == locdef_wheel_diamete_update(data[0]))
		{
			locdef_dp_update(dp_header->dpid,dp_header->type,dp_header->len,(void*)&data[0]);
		}
    }
	
}


/*****************************************************************************
@brief  dp_download_search_handle
@brief  DPID_SEARCH处理函数
@param  value:下发的dp值

@return 处理完成后的dp值
@note   可下发可上报类型的处理函数,在处理完成需要将处理后的结果返回，传给上报函数
*****************************************************************************/
static bool dp_cmd_search_handle(bool data)
{
    int op_ret = 0;
    TONE_TYPE_E cur_tone = locdef_get_cur_play_tone();

    if (data == 0) {
        if (cur_tone == TONE_SEARCH) {
            op_ret = locdef_dev_audio_stop();
            if (op_ret != 0) {
                TUYA_LOG_E(MOD_DP, " audio stop failed");
                data = 1;
            }
        }

    }else{
        TONE_TYPE_E cur_tone = locdef_get_cur_play_tone();

        /* 当没有报警音时才播放寻车提示音 */
        if (cur_tone != TONE_ALARM) {
            ty_vfm_sensor_event_post(SENSOR_EVENT_SEARCH);
        }
    }
    return data ? true : false;
}


static void evt_dp_query_bond_status_cb(const ty_dp_header_t *dp_header,const uint8_t *data)
{
}

/**
 * @brief  dp数据上报函数
 *
 * @param  obj_dp
 *
 * @return 无
 */
void locdef_dp_update(uint8_t dpid, uint8_t type,uint16_t len,void *param)
{
    ty_dp_data_t dp_data;
	
    memset(&dp_data, 0, sizeof(dp_data));
	
    dp_data.header.dpid = dpid;
    dp_data.header.type = type;
    dp_data.header.len = len;
	dp_data.value = (uint8_t*)param;
    TUYA_LOG_I(MOD_DP, "dpid:%d type:%d len:%d",dp_data.header.dpid,type,len);
	ty_dp_report_proc(&dp_data,NULL);
}


int ty_dp_report(uint8_t *dp_data, uint16_t dp_len)
{
    //TUYA_APP_LOG_HEXDUMP_INFO("Fill Report Dp:", dp_data, dp_len);

    if(tuya_ble_connect_status_get() != BONDING_CONN) {
        TUYA_LOG_I(MOD_DP, "Not BONDING_CONN");
        return -1;
    }
	
    return tuya_ble_dp_data_send(s_pd_report_sn++, DP_SEND_TYPE_ACTIVE, DP_SEND_FOR_CLOUD_PANEL, DP_SEND_WITHOUT_RESPONSE, dp_data,dp_len);
}


/*
*    
*/
int ty_dp_data_received_handle(uint8_t *dp_data, uint16_t dp_len)
{
	ty_dp_header_t *dp = NULL;
    uint16_t pos = 0;
	uint8_t index = 0;
	uint8_t flag = 0;
	uint8_t dp_tab_size = sizeof(s_evt_dp_tab)/sizeof(evt_handle_tab_t);

    if(dp_len == 0) {
        TUYA_LOG_I(MOD_DP, "Wrong Dp Len\n");
        return 0;
    }

    TUYA_APP_LOG_HEXDUMP_INFO("DP Data:", dp_data, dp_len);

    do
	{
		flag = 0;
		if(dp_len - pos < sizeof(ty_dp_header_t))
		{
			TUYA_LOG_W(MOD_DP, "error dp data from tuya_ble_sdk\n");
			break;
		}
		dp = (ty_dp_header_t *)&dp_data[pos];
		dp->len = (uint16_t)(int16_t)BSWAP_16(dp->len);
		pos += sizeof(ty_dp_header_t);
		TUYA_LOG_I(MOD_DP, "dpid:%02x,type:%02x,len:%04x",dp->dpid,dp->type,dp->len);
		for(index = 0;index < dp_tab_size;index++)
		{
			if(s_evt_dp_tab[index].dpid == dp->dpid)
			{
				s_evt_dp_tab[index].cbk(dp,&dp_data[pos]);
				flag = 1;
				break;
			}
		}
		pos += dp->len;

		if(0 == flag)
		{
			TUYA_LOG_I(MOD_DP, "Not Support Dp\n");
		}
	}while(pos < dp_len);
    
    return 0;
}

void ty_dp_query_req(void)
{
	uint8_t i = 0;
	uint8_t para[100] = {0}; // TUYA_BLE_SEND_MAX_DATA_LEN
	uint8_t len = 0;
	uint8_t edoor_status = 0;
    uint8_t whole_dp[] = {DPID_DEF_CTRL, DPID_ACC_CTRL, DPID_POWER_LEVEL,DPID_RSSI_SENSITIVITY, 
		DPID_QUERY_BOND_STATUS, DPID_NEAR_UNLOCK_OPEN_SWITCH, DPID_FAR_LOCK_CLOSE_SWITCH,
		DPID_WHEEL_DIAMETE};
	uint16_t whole_dp_size = sizeof(whole_dp)/sizeof(uint8_t);
	int32_t value_dp;
    uint8_t bool_dp;
	uint8_t lock_status = 0;
	ty_dp_header_t dp_header = {0};
	DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();

    memset(para,0,sizeof(para));
    len = 0;

	for(i = 0; i < whole_dp_size; i++)
	{
		switch(whole_dp[i])
		{
		    case DPID_SPEED:
				dp_header.dpid = DPID_SPEED;
				dp_header.type = DT_VALUE;
				dp_header.len = (int16_t)BSWAP_16(4);
				value_dp = (int32_t)BSWAP_32(20);
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                memcpy((uint8_t*)&para[len + 4],(uint8_t*)&value_dp,4);
				len += 8;
				break;
            case DPID_POWER_LEVEL:
				dp_header.dpid = DPID_POWER_LEVEL;
				dp_header.type = DT_VALUE;
				dp_header.len = (int16_t)BSWAP_16(4);
				value_dp = (int32_t)BSWAP_32(locdef_get_vat_percent());
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                memcpy((uint8_t*)&para[len + 4],(uint8_t*)&value_dp,4);
				len += 8;
				break;
            case DPID_ACC_CTRL:
				ty_vfm_lock_read(DPID_ACC_CTRL,&lock_status);
				edoor_status = lock_status;
				dp_header.dpid = DPID_ACC_CTRL;
				dp_header.type = DT_BOOL;
				dp_header.len = (int16_t)BSWAP_16(1);
				bool_dp = lock_status;
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                para[len + 4] = bool_dp;
				len += 5;
				break;
            case DPID_DEF_CTRL:
				ty_vfm_lock_read(DPID_DEF_CTRL,&lock_status);
				dp_header.dpid = DPID_DEF_CTRL;
				dp_header.type = DT_BOOL;
				dp_header.len = (int16_t)BSWAP_16(1);
				bool_dp = lock_status;
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                para[len + 4] = bool_dp;
				len += 5;
				break;
            case DPID_NEAR_UNLOCK_OPEN_SWITCH:
				dp_header.dpid = DPID_NEAR_UNLOCK_OPEN_SWITCH;
				dp_header.type = DT_BOOL;
				dp_header.len = (int16_t)BSWAP_16(1);
				bool_dp = run_param->dev_hid.near_unlock_switch;
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                para[len + 4] = bool_dp;
				len += 5;
				break;
            case DPID_FAR_LOCK_CLOSE_SWITCH:
				dp_header.dpid = DPID_FAR_LOCK_CLOSE_SWITCH;
				dp_header.type = DT_BOOL;
				dp_header.len = (int16_t)BSWAP_16(1);
				bool_dp = run_param->dev_hid.far_lock_switch;
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                para[len + 4] = bool_dp;
				len += 5;
				break;
            case DPID_QUERY_BOND_STATUS:
				dp_header.dpid = DPID_QUERY_BOND_STATUS;
				dp_header.type = DT_ENUM;
				dp_header.len = (int16_t)BSWAP_16(1);
				bool_dp = run_param->hid_dev_bond;
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                para[len + 4] = bool_dp;
				len += 5;
				break;
            case DPID_RSSI_SENSITIVITY:
                dp_header.dpid = DPID_RSSI_SENSITIVITY;
				dp_header.type = DT_ENUM;
				dp_header.len = (int16_t)BSWAP_16(1);
				switch(run_param->dev_hid.sensitivty)
				{
					case RSSI_OPEN_LOW:
						bool_dp = 0;
						break;
					case RSSI_OPEN_MIDDLE:
						bool_dp = 1;
						break;
					case RSSI_OPEN_HIGH:
						bool_dp = 2;
						break;
					default:
						bool_dp = 1;
						break;
				}
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                para[len + 4] = bool_dp;
				len += 5;
				break;
			case DPID_WHEEL_DIAMETE:
				dp_header.dpid = DPID_WHEEL_DIAMETE;
				dp_header.type = DT_ENUM;
				dp_header.len = (int16_t)BSWAP_16(1);
				bool_dp = locdef_wheel_diamete_get();
				memcpy((uint8_t*)&para[len + 0],(uint8_t*)&dp_header,4);
                para[len + 4] = bool_dp;
				len += 5;
				break;
			default:
				break;
		}
    }
	
    TUYA_LOG_I(MOD_DP, "DpReport, Start(%d)-Lock(%d)-NearUnlock(%d)-FarLock(%d)-Bond(%d)-Sen(%d)-wheel(%d)", 
	    edoor_status, lock_status,run_param->dev_hid.near_unlock_switch, 
	    run_param->dev_hid.far_lock_switch,run_param->hid_dev_bond, run_param->dev_hid.sensitivty,locdef_wheel_diamete_get());
	
    ty_dp_report(para,len);
}


int16_t ty_dp_report_proc(ty_dp_data_t *dp_data,void *param)
{
    uint8_t send_buf[100] = {0};
	uint16_t send_buf_len = 0;

	memset(send_buf,0,sizeof(send_buf));
	send_buf_len = sizeof(dp_data->header) + dp_data->header.len;

	dp_data->header.len = (uint16_t)BSWAP_16(dp_data->header.len);
	memcpy(send_buf,&dp_data->header,sizeof(dp_data->header));
	memcpy(&send_buf[sizeof(dp_data->header)],dp_data->value,send_buf_len - sizeof(dp_data->header));
	
    return ty_dp_report(send_buf,send_buf_len);
}

/*
tuya ble sdk:
#define DT_RAW     0
#define DT_BOOL    1
#define DT_VALUE   2
#define DT_INT     DT_VALUE
#define DT_STRING  3
#define DT_ENUM    4
#define DT_BITMAP  5
#define DT_CHAR    7       //Not currently supported 
#define DT_UCHAR   8       //Not currently supported 
#define DT_SHORT   9       //Not currently supported 
#define DT_USHORT  10      //Not currently supported 
#define DT_LMT    DT_USHORT

cat1 SDK
#define PROP_BOOL 0
#define PROP_VALUE 1
#define PROP_STR 2
#define PROP_ENUM 3
#define PROP_BITMAP 4


*/

int ty_dp_report_sync(const TY_OBJ_DP_S *dp_data)
{
    int index = 0;
	int32_t value_temp = 0;
	ty_dp_data_t dp_local = {0};

    memset(&dp_local,0,sizeof(dp_local));
	dp_local.header.dpid = dp_data->dpid;
	//dp_local.header.type = dp_data->type;
    TUYA_LOG_I(MOD_DP, "dpid :%d",dp_data->dpid);
	switch(dp_data->type)
	{
	    case PROP_BOOL:
			TUYA_LOG_I(MOD_DP, "dp type PROP_BOOL");
			dp_local.header.type = DT_BOOL;
			dp_local.header.len = 1;
			dp_local.value = (void*)&dp_data->value.dp_bool;
			TUYA_LOG_I(MOD_DP, "dp type :%d",dp_data->type);
			break;
		case PROP_VALUE:
			dp_local.header.type = DT_VALUE;
			TUYA_LOG_I(MOD_DP, "dp type PROP_VALUE");
			dp_local.header.len = 4;
			value_temp = (int32_t)BSWAP_32(dp_data->value.dp_value);
			dp_local.value = (void*)&value_temp;
			break;
		case PROP_STR:
			dp_local.header.type = DT_STRING;
			TUYA_LOG_I(MOD_DP, "dp type PROP_STR");
			dp_local.header.len = strlen(dp_data->value.dp_str); 
			dp_local.value = (void*)&dp_data->value.dp_str;
			break;
		case PROP_ENUM:
			dp_local.header.type = DT_ENUM;
			TUYA_LOG_I(MOD_DP, "dp type PROP_VALUE");
			dp_local.header.len = 1;
			dp_local.value = (void*)&dp_data->value.dp_enum;
			break;
		case PROP_BITMAP:
			dp_local.header.type = DT_BITMAP;
			TUYA_LOG_I(MOD_DP, "dp type PROP_VALUE");
			dp_local.header.len = 4;//TODO
			value_temp = (int32_t)BSWAP_32(dp_data->value.dp_bitmap);
			dp_local.value = (void*)&value_temp;
			break;
		default:
			TUYA_LOG_I(MOD_DP, "dp type error");
			return -1;
	}
	return ty_dp_report_proc(&dp_local,NULL);
}



// 用户配置 可供开发者扩展自定义状态 若增加状态机状态 须按FSM INDEX顺序在表列新添加状态列
static VFM_DC_STAT_T s_vfm_dc_st_table[] = {
  // FSM_ACCOFF_UNDEF   FSM_ACCOFF_DEF   FSM_ACCON   FSM_MOVING   FSM_WARNING
    {DPID_ACC_CTRL, DP_FUN_ACC, 1*DP_HOUR},
    {DPID_ACC_CTRL, DP_FUN_ACC, 1*DP_HOUR},
    {DPID_ACC_CTRL, DP_FUN_ACC, 5*DP_SECOND},
    {DPID_ACC_CTRL, DP_FUN_ACC, 5*DP_SECOND},
    {DPID_ACC_CTRL, DP_FUN_ACC, 5*DP_SECOND},

    {DPID_SIG_MILEAGE, DP_FUN_MILE, 1*DP_HOUR},
    {DPID_SIG_MILEAGE, DP_FUN_MILE, 1*DP_HOUR},
    {DPID_SIG_MILEAGE, DP_FUN_MILE, 5*DP_SECOND},
    {DPID_SIG_MILEAGE, DP_FUN_MILE, 5*DP_SECOND},
    {DPID_SIG_MILEAGE, DP_FUN_MILE, 5*DP_SECOND},

    {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, 1*DP_HOUR},
    {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, 1*DP_HOUR},
    {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, 5*DP_SECOND},
    {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, 5*DP_SECOND},
    {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, 5*DP_SECOND},
	
    {DPID_SPEED, DP_FUN_SPEED, 0},
    {DPID_SPEED, DP_FUN_SPEED, 0},
    {DPID_SPEED, DP_FUN_SPEED, 5*DP_SECOND},
    {DPID_SPEED, DP_FUN_SPEED, 500*DP_MILLIS},
    {DPID_SPEED, DP_FUN_SPEED, 0},
};

// 状态事件配置 一个事件上传类型为一行 VFM FSM状态INDEX为一列
static VFM_DC_EVT_T s_vfm_dc_evt_table[] = {
  // FSM_ACCOFF_UNDEF   FSM_ACCOFF_DEF   FSM_ACCON   FSM_MOVING   FSM_WARNING
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, true},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, true},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, false},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, false},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, false},

  //{DPID_SEARCH,   //寻车
  //{DPID_DEV_STATUS,//设备状态
  {DPID_ACC_CTRL, DP_FUN_ACC, true},
  {DPID_ACC_CTRL, DP_FUN_ACC, true},
  {DPID_ACC_CTRL, DP_FUN_ACC, true},
  {DPID_ACC_CTRL, DP_FUN_ACC, true},
  {DPID_ACC_CTRL, DP_FUN_ACC, true},

  {DPID_CUSHION_CTRL, DP_FUN_CUSHION_LOCK, true},
  {DPID_CUSHION_CTRL, DP_FUN_CUSHION_LOCK, true},
  {DPID_CUSHION_CTRL, DP_FUN_CUSHION_LOCK, true},
  {DPID_CUSHION_CTRL, DP_FUN_CUSHION_LOCK, true},
  {DPID_CUSHION_CTRL, DP_FUN_CUSHION_LOCK, true},

  {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, true},
  {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, true},
  {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, true},
  {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, true},
  {DPID_POWER_LEVEL, DP_FUN_EXT_RSOC, true},

  {DPID_SPEED, DP_FUN_SPEED, false},
  {DPID_SPEED, DP_FUN_SPEED, false},
  {DPID_SPEED, DP_FUN_SPEED, true},
  {DPID_SPEED, DP_FUN_SPEED, true},
  {DPID_SPEED, DP_FUN_SPEED, false},
  /*
  {DPID_FAR_LOCK_CLOSE_SWITCH, DP_FUN_SPEED, true},
  {DPID_FAR_LOCK_CLOSE_SWITCH, DP_FUN_SPEED, true},
  {DPID_FAR_LOCK_CLOSE_SWITCH, DP_FUN_SPEED, true},
  {DPID_FAR_LOCK_CLOSE_SWITCH, DP_FUN_SPEED, true},
  {DPID_FAR_LOCK_CLOSE_SWITCH, DP_FUN_SPEED, true},

  {DPID_NEAR_UNLOCK_OPEN_SWITCH, DP_FUN_SPEED, true},
  {DPID_NEAR_UNLOCK_OPEN_SWITCH, DP_FUN_SPEED, true},
  {DPID_NEAR_UNLOCK_OPEN_SWITCH, DP_FUN_SPEED, true},
  {DPID_NEAR_UNLOCK_OPEN_SWITCH, DP_FUN_SPEED, true},
  {DPID_NEAR_UNLOCK_OPEN_SWITCH, DP_FUN_SPEED, true},*/
};

// 状态切换单次批量上传配置 每一列会在切换到当前状态下批量上传 不上传en位置为false
static VFM_DC_EVT_T s_vfm_dc_multi_evt_table[] = {
  // FSM_ACCOFF_UNDEF   FSM_ACCOFF_DEF   FSM_ACCON   FSM_MOVING   FSM_WARNING
  {DPID_ACC_CTRL, DP_FUN_ACC, true},
  {DPID_ACC_CTRL, DP_FUN_ACC, true},
  {DPID_ACC_CTRL, DP_FUN_ACC, true},
  {DPID_ACC_CTRL, DP_FUN_ACC, false},
  {DPID_ACC_CTRL, DP_FUN_ACC, false},

  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, true},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, true},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, true},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, false},
  {DPID_DEF_CTRL, DP_FUN_DEF_LOCK, false},
};


/**
 * @brief   dp上报注册函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
void locdef_dp_uplink_register(void)
{
    g_upload_cfg.st_table = s_vfm_dc_st_table;
    g_upload_cfg.evt_table = s_vfm_dc_evt_table;
    g_upload_cfg.multi_evt_table = s_vfm_dc_multi_evt_table;
    g_upload_cfg.state_nums = 5;
    g_upload_cfg.st_nums = sizeof(s_vfm_dc_st_table)/sizeof(s_vfm_dc_st_table[0])/g_upload_cfg.state_nums;
    g_upload_cfg.evt_nums = sizeof(s_vfm_dc_evt_table)/sizeof(s_vfm_dc_evt_table[0])/g_upload_cfg.state_nums;
    g_upload_cfg.multi_nums = sizeof(s_vfm_dc_multi_evt_table)/sizeof(s_vfm_dc_multi_evt_table[0])/g_upload_cfg.state_nums;

    if (ty_vfm_upload_register(&g_upload_cfg) != 0) {
        TUYA_LOG_E(MOD_DP, "uplink register fail");
    } else {
        TUYA_LOG_I(MOD_DP, "state_nums[%d] st_nums[%d] evt_nums[%d] multi_nums[%d]",
                g_upload_cfg.state_nums, g_upload_cfg.st_nums, g_upload_cfg.evt_nums, g_upload_cfg.multi_nums);
    }

}



