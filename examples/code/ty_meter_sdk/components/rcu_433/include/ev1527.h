#ifndef __EV1525_H_
#define __EV1525_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "tuya_ble_stdlib.h"

typedef struct{
	uint8_t Last_Level;//上一次电平
	uint16_t HL_W;//此次高电平宽度
	uint16_t LL_W;//此次低电平宽度
	uint16_t Code_Num;//接收到第几个编码
	uint8_t Key_Val;//后四位键值
	uint8_t Last_Key_Val;//后四位键值
	uint8_t Verify_Code[3];//前二十位校验码
	uint8_t Sync_Flag;//同步标志
	uint8_t Decode_Compile;
	uint8_t Addr_Code[3];
	uint32_t Start_Time;//开始解码时间戳
	uint8_t LP_Start_Flag;
	uint8_t UP_Flag;
}EV1527_DECODE_T;

typedef enum {
    RCU_WORK_SHUTDOWN,
    RCU_WORK_RUNING,
}RCU_WORK_MODE_E;

int tuya_ev1527_init(uint8_t data_pin, uint8_t shut_pin);

void tuya_ev1527_start(uint32_t us);

void tuya_ev1527_stop(void);

void tuya_ev1527_resume(uint32_t us);

void tuya_ev1527_pause(void);

void  tuya_ev1527_set_learn_mode(int learn);

void tuya_ev1527_set_code_bits(uint8_t bits);

#ifdef __cplusplus
} // extern "C"
#endif
#endif
