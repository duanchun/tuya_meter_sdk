
#include "tuya_kel_os.h"
#include "os_task.h"
#include "tuya_ble_queue.h"
#include "tuya_log.h"
#include "tuya_ble_mem.h"

typedef int (*os_task_cbk)(os_event_t *param);

typedef struct {
	uint16_t task_handle;
	tuya_ble_queue_t queue_handle;
	uint8_t *queue_buff;
}ty_task_context_t;

void *tuya_kel_malloc(uint16_t size)
{
    return tuya_ble_malloc(size);
}

void tuya_kel_free(void* ptr)
{
    tuya_ble_free(ptr);
}

tuya_kel_task_handle tuya_kel_task_create(
	int (*task_entry)(void *),
	void *arg,
	uint16_t nMsgElemSize,
	uint16_t nMsgQMax,
	uint32_t nStackSize,
	uint8_t nPriority,
	const char *pTaskName)
{
    uint16_t task_id = 0;
	uint8_t *queue_buff = NULL;
	ty_task_context_t *handle = NULL;

	handle = tuya_kel_malloc(sizeof(ty_task_context_t));
	if(NULL == handle)
	{
	    TUYA_LOG_E(MOD_OS, "malloc failed");
		return NULL;
	}

	queue_buff = tuya_kel_malloc(nMsgElemSize*nMsgQMax);
	if(NULL == queue_buff)
	{
	    tuya_kel_free(handle);
	    TUYA_LOG_E(MOD_OS, "queue buff malloc failed");
		return NULL;
	}
	
	tuya_ble_queue_init(&handle->queue_handle,queue_buff,nMsgQMax,nMsgElemSize);
	
    task_id = os_task_create((os_task_cbk)task_entry);
	if(0xff == task_id)
	{
	    tuya_kel_free(queue_buff);
		tuya_kel_free(handle);
		TUYA_LOG_E(MOD_OS, "os task create failed");
	    return NULL;
	}
	handle->task_handle = task_id;
	return (void*)handle;
}

TUYA_RET_E tuya_kel_wait_msg(tuya_kel_task_handle task_handle, void *msg_recv, uint32_t timeout)
{
	ty_task_context_t *handle = NULL;
	tuya_ble_status_t ret = TUYA_BLE_SUCCESS;

	if(NULL == task_handle || NULL == msg_recv)
	{
		TUYA_LOG_E(MOD_OS, "invalid params");
		return TUYA_INVALID_PARAMS;
	}
    handle = (ty_task_context_t *)task_handle;

	if(TUYA_BLE_SUCCESS != tuya_ble_dequeue(&handle->queue_handle,msg_recv))
	{
		TUYA_LOG_E(MOD_OS, "tuya_ble_dequeue failed");
		return TUYA_COMMON_ERROR;
	}
	return TUYA_OK;
}

TUYA_RET_E tuya_kel_send_msg (tuya_kel_task_handle task_handle, void *msg, uint32_t timeout)
{
 	ty_task_context_t *handle = NULL;
	os_event_t evt;

	if(NULL == task_handle || NULL == msg)
	{
		TUYA_LOG_E(MOD_OS, "invalid params");
		return TUYA_INVALID_PARAMS;
	}
	
	handle = (ty_task_context_t *)task_handle;
	memset(&evt,0,sizeof(evt));

	if(TUYA_BLE_SUCCESS != tuya_ble_enqueue(&handle->queue_handle,msg))
	{
		TUYA_LOG_E(MOD_OS, "tuya_ble_enqueue failed");
		return TUYA_COMMON_ERROR;
	}
	
    os_msg_post(handle->task_handle, &evt);
	
	return TUYA_OK;
}

uint8_t tuya_kel_msg_cnt_get (tuya_kel_task_handle task_handle)
{
	ty_task_context_t *handle = task_handle;
	uint8_t cnt = 0;

	if(NULL == handle )
	{
		TUYA_LOG_E(MOD_OS, "invalid params");
		return 0;
	}

	cnt = tuya_ble_get_queue_used(&handle->queue_handle);
	return cnt;
}


void tuya_kel_sleep_enable(void)
{
	system_sleep_enable();
}

void tuya_kel_sleep_disable(void)
{
	system_sleep_disable();
}



