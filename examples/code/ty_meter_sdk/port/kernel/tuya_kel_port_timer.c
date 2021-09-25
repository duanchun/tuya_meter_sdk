
#include "tuya_kel_timer.h"
#include "os_timer.h"
#include "tuya_log.h"

#define TY_CORE_TIMER_MAX_NUM    10

typedef struct {
    os_timer_t os_timer;
    uint8_t    is_occupy;
    uint32_t   ms;
    uint8_t    mode;
} ty_core_timer_t;

static ty_core_timer_t s_ty_core_timer_pool[TY_CORE_TIMER_MAX_NUM] = {0};

static ty_core_timer_t* ty_core_acquire_timer(uint32_t ms, uint8_t mode)
{
    for(uint8_t i=0; i<TY_CORE_TIMER_MAX_NUM; i++) {
        if (s_ty_core_timer_pool[i].is_occupy == 0) {
            s_ty_core_timer_pool[i].is_occupy = 1;
            s_ty_core_timer_pool[i].ms = ms;
            s_ty_core_timer_pool[i].mode = mode;
            return (void*)&s_ty_core_timer_pool[i].os_timer;
        }
    }
    return NULL;
}

static void ty_core_revert_timer(os_timer_t *core_timer)
{
    for(uint8_t i=0; i<TY_CORE_TIMER_MAX_NUM; i++) {
        if (&s_ty_core_timer_pool[i].os_timer == core_timer) {
            s_ty_core_timer_pool[i].is_occupy = 0;
            break;
        }
    }
}


TUYA_RET_E tuya_kel_timer_create(tuya_kel_timer_handle *handle, uint32_t ms, TUYA_KEL_TIMER_MODE_E mode, tuya_kel_timer_handler_cbk handler)
{

	ty_core_timer_t* timer_item = ty_core_acquire_timer(ms, mode);

	if(NULL == timer_item)
	{
	    TUYA_LOG_W(MOD_TIMER,"timer create failed");
		handle = NULL;
		return TUYA_COMMON_ERROR;
	}
    os_timer_init(&timer_item->os_timer, handler, NULL);

    *handle = timer_item;
    return TUYA_OK;
}


TUYA_RET_E tuya_kel_timer_delete(tuya_kel_timer_handle handle)
{
	if(NULL == handle)
	{
		TUYA_LOG_E(MOD_TIMER,"handle is null");
		return TUYA_INVALID_PARAMS;
	}
    ty_core_timer_t* timer_item = (ty_core_timer_t*)handle;
	ty_core_revert_timer(&timer_item->os_timer);
    os_timer_destroy(&timer_item->os_timer);
    return TUYA_OK;
}

TUYA_RET_E tuya_kel_timer_start(tuya_kel_timer_handle handle)
{
    ty_core_timer_t* timer_item = (ty_core_timer_t*)handle;

	if(NULL == handle)
	{
		TUYA_LOG_E(MOD_TIMER,"handle is null");
		return TUYA_INVALID_PARAMS;
	}
  
    if(timer_item->mode == TUYA_BLE_TIMER_SINGLE_SHOT) {
        os_timer_start(&timer_item->os_timer, timer_item->ms, false);
    } else if(timer_item->mode == TUYA_BLE_TIMER_REPEATED) {
        os_timer_start(&timer_item->os_timer, timer_item->ms, true);
    }
    return TUYA_OK;
}


TUYA_RET_E tuya_kel_timer_stop(tuya_kel_timer_handle handle)
{
	ty_core_timer_t* timer_item = (ty_core_timer_t*)handle;
	
	if(NULL == handle)
	{
		TUYA_LOG_E(MOD_TIMER,"handle is null");
		return TUYA_INVALID_PARAMS;
	}
	
    
    os_timer_stop(&timer_item->os_timer);
    return TUYA_OK;
}

TUYA_RET_E tuya_kel_timer_restart(tuya_kel_timer_handle handle, uint32_t ms)
{
	ty_core_timer_t* timer_item = (ty_core_timer_t*)handle;
	
	if(NULL == handle)
	{
		TUYA_LOG_E(MOD_TIMER,"handle is null");
		return TUYA_INVALID_PARAMS;
	}

	os_timer_stop(&timer_item->os_timer);

	timer_item->ms = ms;
	if(timer_item->mode == TUYA_BLE_TIMER_SINGLE_SHOT) {
        os_timer_start(&timer_item->os_timer, timer_item->ms, false);
    } else if(timer_item->mode == TUYA_BLE_TIMER_REPEATED) {
        os_timer_start(&timer_item->os_timer, timer_item->ms, true);
    }
	return TUYA_OK;
}


