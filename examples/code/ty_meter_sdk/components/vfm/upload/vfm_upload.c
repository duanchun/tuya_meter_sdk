#include "vfm_upload.h"
#include <stdio.h>
#include <string.h>
#include "vfm_fsm.h"
#include "vfm_lock.h"
#include "vfm_sensor.h"
#include "vfm_vbat.h"
#include "tuya_kel_os.h"
#include "tuya_kel_system.h"

#if 0
#define LOCAL_LOG_I  VFM_INFO
#define LOCAL_LOG_E    VFM_ERR
#else
#define LOCAL_LOG_I(log_type, fmt, args...)
#define LOCAL_LOG_E(log_type, fmt, args...)
#endif

#define Malloc tuya_kel_malloc
#define Free tuya_kel_free

// 最小统计上传时间周期间隔
#define VFM_UPLOAD_TIMEVAL_MIN                  (100)
//使用4字节对齐模式
#define MEM_ALIGNMENT                           (4)
#define MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))

typedef void (*vfm_dp_upload)(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index);

typedef struct {
    VFM_UPLOAD_CFG_T cfg;
    //统计上传task句柄
    void* st_handle;
    //事件上传task句柄
    void* evt_handle;
    //上传时间的数组指针
    uint32_t* last_time_arr;
} VFM_UPLOAD_T;

typedef struct
{
    //DP功能
    VFM_DP_FUN dp_fun;
    //DP功能对应处理函数
	vfm_dp_upload dp_handle;
}VFM_DP_FUN_T;


static VFM_UPLOAD_T s_vfm_upload;
static VFM_UPLOAD_T* sp_vfm_upload = &s_vfm_upload;

static void ty_vfm_upload_bool_dp_package(uint8_t dpid, int dp_bool, TY_OBJ_DP_S *dp_data, uint32_t *index)
{

    dp_data->dpid = dpid;
    dp_data->type = PROP_BOOL;
	dp_data->value.dp_bool = dp_bool;
	(*index) ++;
	LOCAL_LOG_I(VFM_UPLOAD_LOG, "333 dp dpid==%d,type==%d,index==%d,value==%d",dp_data->dpid,dp_data->type,*index,dp_data->value.dp_bool);

}

static void ty_vfm_upload_value_dp_package(uint8_t dpid, int dp_value, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "upload value dp");
    dp_data[(*index)].dpid = dpid;
    dp_data[(*index)].type = PROP_VALUE;
    dp_data[(*index)].value.dp_value = dp_value;
    (*index) ++;
}

static void ty_vfm_upload_str_dp_package(uint8_t dpid, char *dp_str, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "upload str dp");
    dp_data[(*index)].dpid = dpid;
    dp_data[(*index)].type = PROP_STR;
    dp_data[(*index)].value.dp_str = dp_str;
    (*index) ++;
}

static void ty_vfm_upload_enum_dp_package(uint8_t dpid, uint32_t dp_enum, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "upload enum dp");
    dp_data[(*index)].dpid = dpid;
    dp_data[(*index)].type = PROP_ENUM;
    dp_data[(*index)].value.dp_enum = dp_enum;
    (*index) ++;
}

static void ty_vfm_motor_lock_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    LOCK_ACTION_E action;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter motor_lock_dp_upload");
    ty_vfm_lock_read(LOCK_TYPE_MOTOR_LOCK, &action);
    ty_vfm_upload_bool_dp_package(dp_id, action, dp_data, index);
	{
	    LOCAL_LOG_I(VFM_UPLOAD_LOG, "TCM [%d,%d,%d]",dp_data->dpid,dp_data->type,dp_data->value.dp_bool);
	}
}

static void ty_vfm_def_lock_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    LOCK_ACTION_E action;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter def_lock_dp_upload");
    ty_vfm_lock_read(LOCK_TYPE_DEF_LOCK, &action);
    ty_vfm_upload_bool_dp_package(dp_id, action, dp_data, index);
}

static void ty_vfm_userlock_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    LOCK_ACTION_E action;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter userlock_dp_upload");
    ty_vfm_lock_read(LOCK_TYPE_USER_LOCK, &action);
    ty_vfm_upload_bool_dp_package(dp_id, action, dp_data, index);
}

static void ty_vfm_speed_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    uint32_t speed;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter speed_dp_upload dp_id==%u",dp_id);
    ty_vfm_sensor_read_speed(&speed);
	LOCAL_LOG_I(VFM_UPLOAD_LOG, "speed==%u",speed);
    ty_vfm_upload_value_dp_package(dp_id, speed, dp_data, index);
}

static void ty_vfm_mile_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    float mileage;
    uint32_t nMileage = 0;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter mile_dp_upload");
    ty_vfm_sensor_read_mileage(&mileage);
    nMileage = (int)(mileage*10);
    ty_vfm_upload_value_dp_package(dp_id, nMileage, dp_data, index);
}

static void ty_vfm_sensor_error_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    uint32_t error;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter sensor_error_dp_upload");
    ty_vfm_sensor_read_error(&error);
    ty_vfm_upload_value_dp_package(dp_id, error, dp_data, index);
}

static void ty_vfm_acc_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    ACC_POWER_E action;

    ty_vfm_acc_read(&action);
    ty_vfm_upload_bool_dp_package(dp_id, action, dp_data, index);
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter acc_dp_upload: %s", action == ACC_POWER_ON ? "ACC_ON":"ACC_OFF");
}

static void ty_vfm_ext_rsoc_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    uint8_t rsoc;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "ENTER");
    ty_vfm_cellular_read_rsoc(VBAT_EXTION, &rsoc);
    ty_vfm_upload_value_dp_package(dp_id, rsoc, dp_data, index);
}

#if 0
static void ty_vfm_hid_far_lock_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    LOCK_ACTION_E action;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter ty_vfm_hid_far_lock_dp_upload_handle");
    ty_vfm_lock_read(LOCK_TYPE_MOTOR_LOCK, &action);
    ty_vfm_upload_value_dp_package(dp_id, temp, dp_data, index);
}

static void ty_vfm_hid_near_unlock_dp_upload_handle(uint8_t dp_id, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
    int temp;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "ENTER");
    ty_vfm_vbt_read_temperature(VBAT_EXTION, &temp);
    ty_vfm_upload_value_dp_package(dp_id, temp, dp_data, index);
}
#endif

static const VFM_DP_FUN_T s_vfm_dp_funs[] =  {
	{DP_FUN_MOTOR_LOCK,         ty_vfm_motor_lock_dp_upload_handle     },
    {DP_FUN_DEF_LOCK,           ty_vfm_def_lock_dp_upload_handle       },
	{DP_FUN_USER_LOCK,          ty_vfm_userlock_dp_upload_handle       },
	{DP_FUN_SPEED,              ty_vfm_speed_dp_upload_handle          },
	{DP_FUN_MILE,               ty_vfm_mile_dp_upload_handle           },
	{DP_FUN_SENSOR_ERROR,       ty_vfm_sensor_error_dp_upload_handle   },
	{DP_FUN_ACC,                ty_vfm_acc_dp_upload_handle            },
	{DP_FUN_EXT_RSOC,           ty_vfm_ext_rsoc_dp_upload_handle       }
};

/**
 * @brief   dp上传处理
 *
 * @param   dp_fun 上传功能点
 *
 * @return  无
 */
static int ty_vfm_dp_upload_proc(VFM_DP_FUN dp_fun, uint8_t  dpid, TY_OBJ_DP_S *dp_data, uint32_t *index)
{
	uint32_t ind;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "dp_fun is ....:0x%x", dp_fun);
	for(ind = 0; ind < sizeof(s_vfm_dp_funs)/sizeof(VFM_DP_FUN_T); ind++)
	{
		if(dp_fun == s_vfm_dp_funs[ind].dp_fun) {
			break;
		}
	}
	if(ind == sizeof(s_vfm_dp_funs)/sizeof(VFM_DP_FUN_T)) {
        LOCAL_LOG_E(VFM_UPLOAD_LOG, "there is no index:%d\n", ind);
		return -1;
	}
	if(NULL == s_vfm_dp_funs[ind].dp_handle) {
        LOCAL_LOG_E(VFM_UPLOAD_LOG, "function is null");
		return -1;
	}
	LOCAL_LOG_I(VFM_UPLOAD_LOG, "dp_fun match:0x%x", dp_fun);
    
    s_vfm_dp_funs[ind].dp_handle(dpid, dp_data, index);

	return 0;
}

/**
 * @brief   查找事件表dp_fun对应的dp参数
 *
 * @param   dp_fun 上传功能点
 * @param   dpid   传出对应的dpid值
 * @param   enable 该状态下使能位
 * @return  无
 */
static void ty_vfm_evt_find_dp_param(VFM_DP_FUN dp_fun, uint16_t *dpid, int *enable)
{
    uint32_t  stateIndex;

    *enable = false;
    *dpid = 0;

    ty_vfm_fsm_get_now_state(NULL, &stateIndex);
    for(int i = 0; i < sp_vfm_upload->cfg.evt_nums; i ++) {
        if(sp_vfm_upload->cfg.evt_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].fun != dp_fun) {
            continue;
        }
        *dpid = sp_vfm_upload->cfg.evt_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].dpid;
        *enable = sp_vfm_upload->cfg.evt_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].en;
    }
}

/**
 * @brief   查找批量事件表dp_fun对应的dp参数
 *
 * @param   dp_fun 上传功能点
 * @param   dpid   传出对应的dpid值
 * @param   enable 该状态下使能位
 * @return  无
 */
static void ty_vfm_multi_evt_find_dp_param(VFM_DP_FUN dp_fun, uint16_t *dpid, int *enable)
{
    uint32_t  stateIndex;

    *enable = false;
    *dpid = 0;

    ty_vfm_fsm_get_now_state(NULL, &stateIndex);
    for(int i = 0; i < sp_vfm_upload->cfg.multi_nums; i ++) {
        if(sp_vfm_upload->cfg.multi_evt_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].fun != dp_fun) {
            continue;
        }
        *dpid = sp_vfm_upload->cfg.multi_evt_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].dpid;
        *enable = sp_vfm_upload->cfg.multi_evt_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].en;
    }
}


//求两个数的最大公约数
uint32_t ty_vfm_upload_gcd(uint32_t a, uint32_t b) {
    do {
        a %= b;
        b ^= a; a ^= b; b ^= a;
    } while (b);
    return a;
}

/**
 * @brief   批量上传线程 包括状态切换批量和统计上传批量
 *
 * @param   NULL
 *
 * @return  无
 */
static void ty_vfm_multi_upload_proc_task(void *param)
{
    
}

/**
 * @brief   批量上传线程 包括状态切换批量和统计上传批量
 *
 * @param   NULL
 *
 * @return  无
 */
static int ty_vfm_multi_upload_proc_task_adapt(void* param)
{
    int  op_ret = 0;
    uint32_t  stateIndex;
	uint32_t now_fsm_status = 0;
    uint32_t  curTime = 0;
    uint32_t  uploadTime = 0;
    uint32_t  prevTime = 0;
    uint32_t  timeval = 0;
    uint32_t  fun = 0;
    uint32_t  dpid = 0;
    uint32_t  dp_data_cnt = 0;
	TY_OBJ_DP_S temp_dp = {0};

    //周期批量上传
    ty_vfm_fsm_get_now_state(&now_fsm_status, &stateIndex);
	
    curTime = tuya_kel_sys_get_curr_time();
    for(uint32_t i = 0; i < sp_vfm_upload->cfg.st_nums; i ++) {
		
        prevTime = sp_vfm_upload->last_time_arr[i*sp_vfm_upload->cfg.state_nums+stateIndex];
        timeval = sp_vfm_upload->cfg.st_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].timeval;
        // 忽略周期为0 或者时间间隔过小的DP事件
        if(timeval < VFM_UPLOAD_TIMEVAL_MIN) {
            continue;
        }
        
        if((( curTime - prevTime ) > timeval ) || ( prevTime == 0 )) {
            dp_data_cnt = 0;
            uploadTime = tuya_kel_sys_get_curr_time();
			LOCAL_LOG_I(VFM_UPLOAD_LOG, "now_fsm_status==0x%X",now_fsm_status);
            //查找满足上传条件的相同周期FUN 从零开始保证时间戳不同 FUN一起上报
            for(uint32_t ind = 0; ind < sp_vfm_upload->cfg.st_nums; ind ++) {
				//LOCAL_LOG_I(VFM_UPLOAD_LOG,"INner for index==%d",ind);
                if(timeval == sp_vfm_upload->cfg.st_table[ind*sp_vfm_upload->cfg.state_nums+stateIndex].timeval) {
                    dpid = sp_vfm_upload->cfg.st_table[ind*sp_vfm_upload->cfg.state_nums+stateIndex].dpid;
                    fun = sp_vfm_upload->cfg.st_table[ind*sp_vfm_upload->cfg.state_nums+stateIndex].fun;
				    
                    //ty_vfm_dp_upload_proc(fun, dpid, st_dp_data, &dp_data_cnt);
                    dp_data_cnt = 0;
                    ty_vfm_dp_upload_proc(fun, dpid, &temp_dp, &dp_data_cnt);
					
					op_ret = ty_dp_report_sync(&temp_dp);
					if(0 != op_ret) {
	                    LOCAL_LOG_E(VFM_UPLOAD_LOG, "dev_report_dp_json_async op_ret:%d",op_ret);
	                }
					if(( temp_dp.type == PROP_STR ) && temp_dp.value.dp_str) {
						LOCAL_LOG_I(VFM_UPLOAD_LOG, "dpid==%u",temp_dp.dpid);
                        Free(temp_dp.value.dp_str);
                    }
					LOCAL_LOG_I(VFM_UPLOAD_LOG, "uploadTime==%u,[%d,%d,%d]",uploadTime,i,sp_vfm_upload->cfg.st_nums,ind);
                    //更新上传时间
                    sp_vfm_upload->last_time_arr[ind * sp_vfm_upload->cfg.state_nums + stateIndex] = uploadTime;
                }
            }
            if(dp_data_cnt > 0) {
                dp_data_cnt = 0;
            }
        }
        else {
            //LOCAL_LOG_I(VFM_UPLOAD_LOG, "[%u,%u,%u,%d,%d]",curTime, prevTime, timeval,i,sp_vfm_upload->cfg.st_nums);
        }
		
    }
	
	return 0;//must return 0
}

void ty_vfm_multi_upload_proc(void)
{
    ty_vfm_multi_upload_proc_task_adapt(NULL);
}


//周期上转处理任务心跳驱动
static void st_evt_upload_heart_drive_cbk(void *arg)
{
    ;
}


/**
 * @brief   统计上传初始化
 *
 * @param   void
 *
 * @return  0 成功 -1 失败
 */
static int ty_vfm_st_upload_init(void)
{
    int  op_ret = 0;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter ty_vfm_st_upload_init");

    return op_ret;
}

static int vfm_evt_upload_proc_task_adapt(void *param)
{
    int dp_fun_temp = NULL;
	int dp_fun = 0;
    int  op_ret = 0;
    uint16_t dpid = 0;
    uint16_t fun = 0;
    int en = false;
    TY_OBJ_DP_S dp_data;
    uint32_t  stateIndex = 0;
    uint32_t  dp_data_cnt = 0;
	
    tuya_kel_wait_msg(sp_vfm_upload->evt_handle,(void*)&dp_fun_temp,0);
	if(NULL == dp_fun_temp)
	{
		LOCAL_LOG_I(VFM_UPLOAD_LOG,"tuya_kel_wait_msg faild");
		goto END;
	}
	dp_fun = dp_fun_temp;
    dp_data_cnt = 0;
    LOCAL_LOG_I(VFM_UPLOAD_LOG,"recevice dp_fun event 0x%04x",dp_fun);
    //状态机切换批量功能
    if(dp_fun <= VFM_FSM_STATE_MAX) {
		LOCAL_LOG_I(VFM_UPLOAD_LOG,"upload many evt dp");
        //memset(multi_dp_data,0,sizeof(TY_OBJ_DP_S)*sp_vfm_upload->cfg.multi_nums);
        ty_vfm_fsm_get_now_state(NULL, &stateIndex);
	    LOCAL_LOG_I(VFM_UPLOAD_LOG,"stateIndex:%u",stateIndex);
        for(uint32_t i = 0; i < sp_vfm_upload->cfg.multi_nums; i ++) {
            fun = sp_vfm_upload->cfg.multi_evt_table[i*sp_vfm_upload->cfg.state_nums+stateIndex].fun;
            ty_vfm_multi_evt_find_dp_param(fun, &dpid, &en);
            if(en) {
				TY_OBJ_DP_S temp_dp = {0};
				//ty_vfm_dp_upload_proc(fun, dpid, multi_dp_data, &dp_data_cnt);
				dp_data_cnt = 0;
				ty_vfm_dp_upload_proc(fun, dpid, &temp_dp, &dp_data_cnt);
			    op_ret = ty_dp_report_sync(&temp_dp);
				if(0 != op_ret) {
                    LOCAL_LOG_E(VFM_UPLOAD_LOG,"dev_report_dp_json_async op_ret:%d",op_ret);
                }
				if(( temp_dp.type == PROP_STR ) && temp_dp.value.dp_str) {
                    Free(temp_dp.value.dp_str);
                }
            }
        }
        if(0 == dp_data_cnt) {
            LOCAL_LOG_E(VFM_UPLOAD_LOG,"can not find event dp param");
        }
    }else {//单事件上传功能
        LOCAL_LOG_I(VFM_UPLOAD_LOG,"upload one evt dp");
        ty_vfm_evt_find_dp_param(dp_fun, &dpid, &en);
        if(en) {
            ty_vfm_dp_upload_proc(dp_fun, dpid, &dp_data, &dp_data_cnt);
			op_ret = ty_dp_report_sync(&dp_data,dp_data_cnt);
            if(0 != op_ret) {
                LOCAL_LOG_E(VFM_UPLOAD_LOG,"dev_report_dp_json_async op_ret:%d",op_ret);
            }
            // 释放PROP_STR类型指针
            if(( dp_data.type == PROP_STR ) && dp_data.value.dp_str) {
                Free(dp_data.value.dp_str);
            }
        }
    }
END:	
    return 0;//must return 0
}

/**
 * @brief   事件上传线程
 *
 * @param   param 线程参数可为空
 *
 * @return  无
 */
static void vfm_evt_upload_proc_task(void *param)
{
    
}

/**
 * @brief   事件上传初始化
 *
 * @param   void
 *
 * @return  0 成功 -1 失败
 */
static int ty_vfm_evt_upload_init(void)
{
    int  op_ret = 0;
    LOCAL_LOG_I(VFM_UPLOAD_LOG,"enter ty_vfm_evt_upload_init");

    sp_vfm_upload->evt_handle = (void*)tuya_kel_task_create(vfm_evt_upload_proc_task_adapt,NULL,4,10,0,0,NULL);
    return op_ret;
}

/**
 * @brief   上传初始化
 *
 * @param   void
 *
 * @return  0 成功 -1 失败
 */
static int ty_vfm_upload_init(void)
{
    int  op_ret = 0;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter ty_vfm_upload_init");
    if(sp_vfm_upload->st_handle == NULL) {
        op_ret = ty_vfm_st_upload_init();
    }
    if (op_ret != 0) {
        LOCAL_LOG_E(VFM_UPLOAD_LOG,"ty_vfm_st_upload_init err");
        return op_ret;
    }
    if(sp_vfm_upload->evt_handle == NULL) {
        op_ret = ty_vfm_evt_upload_init();
    }
    if (op_ret != 0) {
        LOCAL_LOG_E(VFM_UPLOAD_LOG,"ty_vfm_evt_upload_init err");
        return op_ret;
    }

    return op_ret;
}

/**
 * @brief   上传注册函数
 *
 * @param
 *
 * @return
 *
 * @note
 */
int ty_vfm_upload_register(VFM_UPLOAD_CFG_T* pcfg)
{
    int  op_ret = 0;
    LOCAL_LOG_I(VFM_UPLOAD_LOG, "enter ty_vfm_upload_register");

    sp_vfm_upload->cfg.st_table = pcfg->st_table;
    sp_vfm_upload->cfg.evt_table = pcfg->evt_table;
    sp_vfm_upload->cfg.multi_evt_table = pcfg->multi_evt_table;
    sp_vfm_upload->cfg.st_nums = pcfg->st_nums;
    sp_vfm_upload->cfg.evt_nums = pcfg->evt_nums;
    sp_vfm_upload->cfg.multi_nums = pcfg->multi_nums;
    sp_vfm_upload->cfg.state_nums = pcfg->state_nums;

    op_ret = ty_vfm_upload_init();
    if (op_ret != 0) {
        LOCAL_LOG_E(VFM_UPLOAD_LOG,"ty_vfm_upload_init err");
        return op_ret;
    }

    /* 仅分配一次内存空间 */
    if (sp_vfm_upload->last_time_arr == NULL) {
        sp_vfm_upload->last_time_arr = (uint32_t *)Malloc(sp_vfm_upload->cfg.state_nums * sp_vfm_upload->cfg.st_nums * sizeof(uint32_t));
    }
    else {
        LOCAL_LOG_I(VFM_UPLOAD_LOG,"prevTime array initialized");
    }

    if (sp_vfm_upload->last_time_arr == NULL) {
        LOCAL_LOG_E(VFM_UPLOAD_LOG,"ty_vfm_upload_init err");
        return op_ret;
    }

    return op_ret;
}

/**
 * @brief   上报事件接口
 *
 * @param
 *
 * @return
 *
 * @note
 */
int ty_vfm_upload_event_send(VFM_DP_FUN dpFun)
{
	tuya_kel_send_msg(sp_vfm_upload->evt_handle,&dpFun,0);

    return 0;
}
