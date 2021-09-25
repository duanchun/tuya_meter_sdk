
#include "tuya_kel_os.h"

__TUYA_WEAK__ tuya_kel_task_handle tuya_kel_task_create(
	int (*task_entry)(void *),
	void *arg,
	uint16_t nMsgQSize,
	uint16_t nMsgQMax,
	uint32_t nStackSize,
	uint8_t nPriority,
	const char *pTaskName)
{
	return NULL;
}

__TUYA_WEAK__ void *tuya_kel_malloc(uint16_t size)
{
	return NULL;
}

__TUYA_WEAK__ void tuya_kel_free(void* ptr)
{
    ;
}

__TUYA_WEAK__ TUYA_RET_E tuya_kel_wait_msg(tuya_kel_task_handle task_handle, void *msg_recv, uint32_t timeout)
{
	return TUYA_OK;
}

__TUYA_WEAK__ TUYA_RET_E tuya_kel_send_msg (tuya_kel_task_handle task_handle, void *msg, uint32_t timeout)
{
	return TUYA_OK;
}

__TUYA_WEAK__ uint8_t tuya_kel_msg_cnt_get (tuya_kel_task_handle task_handle)
{
	return 0;
}


__TUYA_WEAK__ void tuya_kel_sleep_enable(void)
{
}

__TUYA_WEAK__ void tuya_kel_sleep_disable(void)
{
}



