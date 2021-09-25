/*
 Copyright (c) 2021 Tuya Inc

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <string.h>
#include "tuya_ble_stdlib.h"
#include "ty_timer.h"
#include "ev1527.h"
#include "rcu433_srv.h"
#include "tuya_ble_port.h"
#include "tuya_ble_log.h"
#include "locdef_pin.h"
#include "driver_timer.h"
#include "driver_rtc.h"
#include "tuya_log.h"

#define RCU433_HARD_TIMER_ID  TIMER1
#define RCU433_TIMER1_RERIOD  1
#define RCU433_TIMER1_RERIOD_E  100

//

/*
//同步码时间控制宏
#define SYN_L_MIN     11000
#define SYN_L_MAX     13000
#define SYN_H_MIN     390
#define SYN_H_MAX     500

//0码和1码时间控制宏
#define TIME_H_MIN    1100
#define TIME_H_MAX    1300
#define TIME_L_MIN    390
#define TIME_L_MAX    500
*/

//同步码时间控制宏
#define SYN_L_MIN     300
#define SYN_L_MAX     600
#define SYN_H_MIN     100
#define SYN_H_MAX     101

//0码和1码时间控制宏
#define TIME_H_MIN    38//38
#define TIME_H_MAX    60//50
#define TIME_L_MIN    10
#define TIME_L_MAX    30//20

//精度 35u


#define DATA_IO tuya_pin_read(s_data_pin)

static uint8_t code_bit = 24;
static int learn_mode = false;
static uint8_t s_data_pin = 0;
static uint8_t s_shut_pin = 0;
static EV1527_DECODE_T s_decode;
static RCU_WORK_MODE_E  s_work_mode = RCU_WORK_SHUTDOWN;

static uint32_t H_time = 0;						//高电平时间
static uint32_t L_time = 0;						//低电平时间
static uint32_t before_time = 0;		        //上一次时间
static uint32_t now_time = 0;			    //当前时间

static uint8_t status = 0;					//状态
static uint8_t first_into = 0;			    //第一次进入标识
static uint8_t period_flay = 0;		        //周期完成标识
static uint8_t syn_flay = 0;				//同步码标识
static uint32_t get_code = 0;					//获取的码数
static uint8_t mode = 0;						//保存上一次电平状态

static void work_mode(RCU_WORK_MODE_E mode)
{
    TUYA_LOG_I(MOD_RCU433, " work_mode: %d", mode);
    if (mode == RCU_WORK_SHUTDOWN) {
		tuya_pin_write(s_shut_pin,true);

    }
    else if (mode == RCU_WORK_RUNING) {
        tuya_pin_write(s_shut_pin,false);
    }
    s_work_mode = mode;

}
static void rcu_send(MSG_TYPE_E type,uint8_t addr[3],uint8_t Key_Val)
{
    RCU_MSG_T rcu_msg;
	
	TUYA_LOG_I(MOD_RCU433, " rcu_send type: %d", type);
    rcu_msg.type = type;
    memcpy(rcu_msg.rcu_addr,addr,3);
    rcu_msg.keyvalue  = Key_Val;
    tuya_rcu433_msg_send_internal(rcu_msg);
}

uint32_t tuya_cellular_sys_get_up_time_us(void)
{
	uint32_t tt = rtc_get_tick();
	return  tt;
}

uint8_t get_level(void)
{
	uint8_t xx = 0;
  	xx = gpio_get_pin_value(EBIKE_433_DATA_PORT,EBIKE_433_DATA_GPIO_BIT);
	return xx;
}

static uint32_t tick = 0;
uint8_t i = 0;
static uint8_t uart_flag = 0;
static 	uint32_t hight= 0,low = 0;
static 	uint32_t hight_list[10]= {0x00},low_list[10] = {0x00};

static uint8_t head_flag = 0;


void tuya_ble_433_irq_cb(void *param)
{
	uint8_t level = 0;
	uint32_t tick2 = 0;
	ext_int_disable(EXTI_15);
	//获取电平状态
	if(get_level() == 0x00)
	{
		tick2 = tuya_cellular_sys_get_up_time_us();
		hight = tick2 - tick;
		tick =tick2; 
		uart_flag = 0;
		level = 0;
		mode = 0;
	}
	else{
		tick2 = tuya_cellular_sys_get_up_time_us();
		low = tick2 - tick;
		tick =tick2; 
		//打印标记
		uart_flag =1;
		level = 1;
		mode = 1;
	}
	
	  if(level == 0)
		{
		  ext_int_set_type(EXTI_15, EXT_INT_TYPE_POS); 
		}else
		{
		  ext_int_set_type(EXTI_15, EXT_INT_TYPE_NEG);
		}
	
		ext_int_enable(EXTI_15);
		if(uart_flag == 1)
		{

			if((low>SYN_L_MIN)&&(low<SYN_L_MAX))
			{
			  //收到正确码头
				head_flag = 1;
				code_bit = 24;
				get_code = 0;
			  
			}
			if(head_flag == 1)
			{
				
				  if(((hight>TIME_H_MIN)&&(hight<TIME_H_MAX))&&((low<TIME_L_MAX)&&(low>TIME_L_MIN)))// 1码
					{
						
			      code_bit--;
			      get_code |= (1 << code_bit);
						
					}else if(((low>TIME_H_MIN)&&(low<TIME_H_MAX))&&((hight<TIME_L_MAX)&&(hight>TIME_L_MIN)))//0码
					{
					  code_bit--;
			      get_code |= (0 << code_bit);	
						
					}
					
					 if(!code_bit) {
			            s_decode.Verify_Code[0] = (get_code >> 16);
			            s_decode.Verify_Code[1] = ((get_code >> 8) & 0xff);
			            s_decode.Verify_Code[2] = ((get_code >> 4) & 0x0f);
			            s_decode.Key_Val = (get_code & 0x0f);
						if (learn_mode) {   //学习模式
						    rcu_send(RCU_LEARN_SET,s_decode.Verify_Code,s_decode.Key_Val);
						    //TUYA_LOG_I(MOD_RCU433, " 11  Verify_Code = %02x %02x %02x  Key_Val=%d\r\n",s_decode.Verify_Code[0],s_decode.Verify_Code[1],s_decode.Verify_Code[2],s_decode.Key_Val);
						}
						else {
						    rcu_send(RCU_GET_KEY,s_decode.Verify_Code,s_decode.Key_Val);
						    //TUYA_LOG_I(MOD_RCU433, " 22  Verify_Code = %02x %02x %02x  Key_Val=%d\r\n",s_decode.Verify_Code[0],s_decode.Verify_Code[1],s_decode.Verify_Code[2],s_decode.Key_Val);
						}
			            code_bit = 24;
						
					}
        }

			}
}



int tuya_ev1527_init(uint8_t data_pin, uint8_t shut_pin)
{
	
    int32_t op_ret = 0;
    //433数据通讯
    pmu_set_pin_to_CPU(EBIKE_433_DATA_PORT, (1<<EBIKE_433_DATA_GPIO_BIT));
    system_set_port_mux(EBIKE_433_DATA_PORT,EBIKE_433_DATA_GPIO_BIT,PORTB7_FUNC_B7);
	gpio_set_dir(EBIKE_433_DATA_PORT,EBIKE_433_DATA_GPIO_BIT,GPIO_DIR_IN);
	ext_int_set_port_mux(EXTI_15,EXTI_15_PB7);
    //ext_int_set_type(EXTI_15, EXT_INT_TYPE_POS);
  	ext_int_set_type(EXTI_15, EXT_INT_TYPE_NEG);
    ext_int_enable(EXTI_15);//TODO
	
    //433使能控制,高--》关闭接收器              低--》打开接收器
    TUYA_GPIO_INIT(EBIKE_433_EN,GPIO_DIR_OUT,1);
	
    s_data_pin = data_pin;
    s_shut_pin = shut_pin;
    return op_ret;
}

void tuya_ev1527_start(uint32_t us)
{
	TUYA_LOG_I(MOD_RCU433, " tuya_ev1527_start us: %d", us);
    work_mode(RCU_WORK_RUNING);
    tuya_pin_irq_enable(s_data_pin);
}

void tuya_ev1527_stop(void)
{
	TUYA_LOG_I(MOD_RCU433, " tuya_ev1527_stop");
    work_mode(RCU_WORK_SHUTDOWN);
    s_decode.Decode_Compile = 0;
    memset(&s_decode,0,sizeof(s_decode));
    tuya_pin_irq_disable(s_data_pin);
}

void tuya_ev1527_resume(uint32_t us)
{
    TUYA_LOG_I(MOD_RCU433, " tuya_ev1527_resume");
    tuya_pin_irq_enable(s_data_pin);
}

void tuya_ev1527_pause(void)
{
    TUYA_LOG_I(MOD_RCU433, " tuya_ev1527_pause");
    tuya_pin_irq_disable(s_data_pin);
    s_decode.Decode_Compile = 0;
    memset(&s_decode,0,sizeof(s_decode));
}
void  tuya_ev1527_set_learn_mode(int learn)
{
    learn_mode = learn;
}

void tuya_ev1527_set_code_bits(uint8_t bits)
{
    code_bit =  bits;
}
