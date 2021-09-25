
#include "tuya_kel_timer.h"


__TUYA_WEAK__ TUYA_RET_E tuya_kel_timer_create(tuya_kel_timer_handle *handle, uint32_t ms, TUYA_KEL_TIMER_MODE_E mode, tuya_kel_timer_handler_cbk handler)
{
    return TUYA_OK;
}


__TUYA_WEAK__ TUYA_RET_E tuya_kel_timer_delete(tuya_kel_timer_handle handle)
{
    return TUYA_OK;
}


__TUYA_WEAK__ TUYA_RET_E tuya_kel_timer_start(tuya_kel_timer_handle handle)
{
    return TUYA_OK;
}


__TUYA_WEAK__ TUYA_RET_E tuya_kel_timer_stop(tuya_kel_timer_handle handle)
{
    return TUYA_OK;
}

__TUYA_WEAK__ TUYA_RET_E tuya_kel_timer_restart(tuya_kel_timer_handle handle, uint32_t ms)
{
	return TUYA_OK;
}


