
/**
****************************************************************************
* @file      tuya_kel_os.h
* @brief     tuya_kel_os
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_KEL_OS_H__
#define __TUYA_KEL_OS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

typedef void* tuya_kel_task_handle;

/**
* 接口名称: ty_task_create
* 描述:
*	创建任务
* 参数:
*	task_entry：任务入口函数
*	arg：用户参数
*	nMsgElemSize：和任务关联的消息队列单个项的大小
*	nMsgQMax：和任务关联的消息队列的大小
*	nStackSize：任务的堆栈大小
*	nPriority：任务优先级
*	pTaskName：任务名称
* 返回值: 
*	成功：任务句柄 失败：NULL
*/
tuya_kel_task_handle tuya_kel_task_create(
	int (*task_entry)(void *),
	void *arg,
	uint16_t nMsgElemSize,
	uint16_t nMsgQMax,
	uint32_t nStackSize,
	uint8_t nPriority,
	const char *pTaskName);

/**
* 接口名称: ty_wait_msg
* 描述:
*	等待也与task_handle关联的任务的消息
* 参数:
*	task_handle(in)：任务句柄
*	msg_recv(out)：消息内容
*	timeout(in)：等待超时（对于没有操作系统的环境，此参数无效）
* 返回值: 
*	TUYA_OK:成功，其它: 失败
*/
TUYA_RET_E tuya_kel_wait_msg(tuya_kel_task_handle task_handle, void *msg_recv, uint32_t timeout);

/**
* 接口名称: ty_wait_msg
* 描述:
*	发送消息给task_handle关联的任务
* 参数:
*	task_handle(in)：任务句柄
*	msg(in)：消息内容
*	timeout(in)：等待超时（对于没有操作系统的环境，此参数无效）
* 返回值: 
*	TUYA_OK:成功，其它: 失败
*/
TUYA_RET_E tuya_kel_send_msg (tuya_kel_task_handle task_handle, void *msg, uint32_t timeout);

uint8_t tuya_kel_msg_cnt_get (tuya_kel_task_handle task_handle);


void *tuya_kel_malloc(uint16_t size);

void tuya_kel_free(void* ptr);

void tuya_kel_sleep_enable(void);

void tuya_kel_sleep_disable(void);

#ifdef __cplusplus
}
#endif

#endif


