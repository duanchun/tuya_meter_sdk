#include "vfm_base.h"
#include "tuya_kel_os.h"


static tuya_kel_task_handle s_fsm_pro_task_handle = 0;

static char* log_type[VFM_MAX_LOG] = {
    "VFM_APP_LOG",
    "VFM_GNSS_LOG",
    "VFM_LBS_LOG",
    "VFM_LOCK_LOG",
    "VFM_SENSOR_LOG",
    "VFM_VBAT_LOG",
    "VFM_FSM_LOG",
    "VFM_UPLOAD_LOG",
    "VFM_COMMON_LOG",
};

/**
 * @brief   获取LOG类型标识
 *
 * @param   VOID
 *
 * @return  无
 */
char* tuya_vfm_get_log_type(VFM_LOG_TYPE_E type)
{
    return (type >= VFM_MAX_LOG) ? NULL : log_type[type];
}


void tuya_vfm_base_init(const tuya_kel_task_handle task_handle)
{
    s_fsm_pro_task_handle = task_handle;
}


int tuya_vfm_base_event_post(uint32_t event,uint32_t usr)
{
    BASE_EVENT_t msg;
        
	msg.event = event;
    msg.usr = usr;
	
	tuya_kel_send_msg(s_fsm_pro_task_handle,&msg,0);

	return 0;
}

