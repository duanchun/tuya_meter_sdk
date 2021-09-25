#ifndef __TY_VFM_BASE_H_
#define __TY_VFM_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_type.h"
#include "tuya_kel_os.h"


typedef enum
{
    VFM_APP_LOG = 0x00,
    VFM_GNSS_LOG,
    VFM_LBS_LOG,
    VFM_LOCK_LOG,
    VFM_SENSOR_LOG,
    VFM_VBAT_LOG,
    VFM_FSM_LOG,
    VFM_UPLOAD_LOG,
    VFM_COMMON_LOG,
    VFM_MAX_LOG
}VFM_LOG_TYPE_E;

#if 0
#define VFM_ERR(log_type, fmt, args...) \
     PR_ERR("[%s_ERR/%s:%d]: " fmt, tuya_vfm_get_log_type(log_type), __FUNCTION__, __LINE__, ##args)

#define VFM_INFO(log_type, fmt, args...) \
     PR_INFO("[%s_INFO] " fmt, tuya_vfm_get_log_type(log_type), ##args)
#else
#define VFM_ERR(log_type, fmt, args...) 
	
#define VFM_INFO(log_type, fmt, args...) 

#endif

#define TY_VFM_BASE_ERROR_RETURN(A,__EXPRESS, __CODE)   \
    if (NULL == A) {                 \
        return __CODE;                                  \
    }                                                   \
    if (__EXPRESS)  {                                   \
        return __CODE;                                  \
    }


typedef enum{
  VFM_SHNSOR_EVENT_START	= 0x01,
  VFM_VBAT_EVENT_START	= 0x20,
}VFM_BASE_EVENT_E;

#define VFM_FSM_STATE_MAX  0x4000


typedef struct {
  uint32_t  event;
  uint32_t  usr;
}BASE_EVENT_t;

void tuya_vfm_base_init(const tuya_kel_task_handle fsm_pro_task_handle);

int tuya_vfm_base_event_post(uint32_t event,uint32_t usr);

char* tuya_vfm_get_log_type(VFM_LOG_TYPE_E type);

#ifdef __cplusplus
}

#endif

#endif
