/**
****************************************************************************
* @file      locdef_pin.h
* @brief     locdef_pin
* @author    xiaojian
* @version   V0.0.1
* @date      2021-08
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/


#ifndef __LOCDEF_PIN_H__
#define __LOCDEF_PIN_H__

#include "tuya_ble_stdlib.h"
#include "driver_iomux.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TUYA_PIN_HIGH 1
#define TUYA_PIN_LOW 0
	
#define GPIO_BIT_OFFSET    4

#define TUYA_GPIO_INIT(FUNC,DIR,INIT_VALUE) \
	pmu_set_pin_to_PMU(FUNC##_PORT,(1<<FUNC##_GPIO_BIT)); \
	pmu_set_port_mux(FUNC##_PORT,FUNC##_GPIO_BIT,PMU_PORT_MUX_GPIO); \
	pmu_set_pin_dir(FUNC##_PORT,(1<<FUNC##_GPIO_BIT),DIR); \
	if(GPIO_DIR_OUT == DIR) \
	    pmu_set_gpio_value(FUNC##_PORT,(1<<FUNC##_GPIO_BIT),INIT_VALUE); 



typedef enum
{
	KEY_ACC_OFF = 0,
	KEY_ACC_ON,
}ONEKEY_FUNC_E;


/*************************** 电动车检测与控制 { ******************************/
//一键启动,输入,低有效
#define EBIKE_START_PORT      GPIO_PORT_A
#define EBIKE_START_GPIO_BIT  GPIO_BIT_4
#define EBIKE_START_GPIO      ((EBIKE_START_PORT<<4)|EBIKE_START_GPIO_BIT)

//电门锁检测脚，输入，高有效
#define EBIKE_ELECTRIC_DOOR_LOCK_DET_PORT      GPIO_PORT_D
#define EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO_BIT  GPIO_BIT_4
#define EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO      ((EBIKE_ELECTRIC_DOOR_LOCK_DET_PORT<<GPIO_BIT_OFFSET)|EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO_BIT)

//电门锁控制脚，输出，高有效
#define EBIKE_ELECTRIC_DOOR_LOCK_CTRL_PORT      GPIO_PORT_A
#define EBIKE_ELECTRIC_DOOR_LOCK_CTRL_GPIO_BIT  GPIO_BIT_6
#define EBIKE_ELECTRIC_DOOR_LOCK_CTRL_GPIO      ((EBIKE_ELECTRIC_DOOR_LOCK_CTRL_PORT<<GPIO_BIT_OFFSET)|EBIKE_ELECTRIC_DOOR_LOCK_CTRL_GPIO_BIT)

//超速报警，输入,低有效
#define EBIKE_SPEED_ALARM_PORT      GPIO_PORT_C
#define EBIKE_SPEED_ALARM_GPIO_BIT  GPIO_BIT_1
#define EBIKE_SPEED_ALARM_GPIO      ((EBIKE_SPEED_ALARM_PORT<<GPIO_BIT_OFFSET)|EBIKE_SPEED_ALARM_GPIO_BIT)

//坐垫锁控制（电磁式）,输出，高有效，持续500ms
#define EBIKE_BUCKET_LOCK_PORT      GPIO_PORT_A
#define EBIKE_BUCKET_LOCK_GPIO_BIT  GPIO_BIT_7
#define EBIKE_BUCKET_LOCK_GPIO      ((EBIKE_BUCKET_LOCK_PORT<<GPIO_BIT_OFFSET)|EBIKE_BUCKET_LOCK_GPIO_BIT)

//轮动检测信号,输入，检测PWM脉冲
#define EBIKE_ROTATION_ALARM_PORT       GPIO_PORT_C
#define EBIKE_ROTATION_ALARM_GPIO_BIT   GPIO_BIT_0
#define EBIKE_ROTATION_ALARM_GPIO       ((EBIKE_ROTATION_ALARM_PORT<<GPIO_BIT_OFFSET)|EBIKE_ROTATION_ALARM_GPIO_BIT)

//电机锁控制,输出，高有效
#define EBIKE_MOTOR_LOCK_PORT       GPIO_PORT_A
#define EBIKE_MOTOR_LOCK_GPIO_BIT   GPIO_BIT_5
#define EBIKE_MOTOR_LOCK_GPIO       ((EBIKE_MOTOR_LOCK_PORT<<GPIO_BIT_OFFSET)|EBIKE_MOTOR_LOCK_GPIO_BIT)

//振动检测信号,输入，边沿触发
#define EBIKE_VIBRATE_DETECT_PORT      GPIO_PORT_C
#define EBIKE_VIBRATE_DETECT_GPIO_BIT  GPIO_BIT_2
#define EBIKE_VIBRATE_DETECT_GPIO      ((EBIKE_VIBRATE_DETECT_PORT<<GPIO_BIT_OFFSET)|EBIKE_VIBRATE_DETECT_GPIO_BIT)

/*************************** 电动车检测与控制 } ******************************/

/*************************** 灯和按键 { ******************************/
//BT灯,低电平亮
#define EBIKE_BT_LED_PORT      GPIO_PORT_C
#define EBIKE_BT_LED_GPIO_BIT  GPIO_BIT_3
#define EBIKE_BT_LED_GPIO      ((EBIKE_BT_LED_PORT<<GPIO_BIT_OFFSET)|EBIKE_BT_LED_GPIO_BIT)


/*************************** 灯和按键 } ******************************/

/*************************** 蓄电池电压检测 { ******************************/
//48V电池电压检测,ADC脚：ADC2
#define EBIKE_BAT_48V_ADC_PORT      GPIO_PORT_D
#define EBIKE_BAT_48V_ADC_GPIO_BIT  GPIO_BIT_6
#define EBIKE_BAT_48V_ADC_GPIO      ((EBIKE_BAT_48V_ADC_PORT<<GPIO_BIT_OFFSET)|EBIKE_BAT_48V_ADC_GPIO_BIT)

//48V电池采样使能控制,输出,高有效
#define EBIKE_BAT_48V_ADC_EN_PORT      GPIO_PORT_C
#define EBIKE_BAT_48V_ADC_EN_GPIO_BIT  GPIO_BIT_4
#define EBIKE_BAT_48V_ADC_EN_GPIO      ((EBIKE_BAT_48V_ADC_EN_PORT<<GPIO_BIT_OFFSET)|EBIKE_BAT_48V_ADC_EN_GPIO_BIT)

//48V掉电告警,ADC脚：ADC1
#define EBIKE_BAT_48V_ALARM_PORT       GPIO_PORT_D
#define EBIKE_BAT_48V_ALARM_GPIO_BIT   GPIO_BIT_5
#define EBIKE_BAT_48V_ALARM_GPIO       ((EBIKE_BAT_48V_ALARM_PORT<<GPIO_BIT_OFFSET)|EBIKE_BAT_48V_ALARM_GPIO_BIT)

/*************************** 蓄电池电压检测 } ******************************/

/*************************** 433MHz遥控 { ******************************/
//433使能控制
#define EBIKE_433_EN_PORT       GPIO_PORT_B
#define EBIKE_433_EN_GPIO_BIT   GPIO_BIT_6
#define EBIKE_433_EN_GPIO       ((EBIKE_433_EN_PORT<<GPIO_BIT_OFFSET)|EBIKE_433_EN_GPIO_BIT)

//433数据通讯
#define EBIKE_433_DATA_PORT       GPIO_PORT_B
#define EBIKE_433_DATA_GPIO_BIT   GPIO_BIT_7
#define EBIKE_433_DATA_GPIO       ((EBIKE_433_DATA_PORT<<GPIO_BIT_OFFSET)|EBIKE_433_DATA_GPIO_BIT)

/*************************** 433MHz遥控 } ******************************/

/*************************** 喇叭信号 { ******************************/
//音频功放使能控制
#define EBIKE_PA_EN_PORT      GPIO_PORT_C
#define EBIKE_PA_EN_GPIO_BIT  GPIO_BIT_5
#define EBIKE_PA_EN_GPIO      ((EBIKE_PA_EN_PORT<<GPIO_BIT_OFFSET)|EBIKE_PA_EN_GPIO_BIT)

/*************************** 喇叭信号 } ******************************/

/*************************** 加速度传感器 { ******************************/
//I2C信号,作为I2C的SCL0
#define EBIKE_GSENSOR_SCL0_PORT      GPIO_PORT_B
#define EBIKE_GSENSOR_SCL0_GPIO_BIT  GPIO_BIT_4
#define EBIKE_GSENSOR_SCL0_GPIO      ((EBIKE_GSENSOR_SCL0_PORT<<GPIO_BIT_OFFSET)|EBIKE_GSENSOR_SCL0_GPIO_BIT)

//I2C信号,作为I2C的SDA0
#define EBIKE_GSENSOR_SDA0_PORT        GPIO_PORT_B
#define EBIKE_GSENSOR_SDA0_GPIO_BIT    GPIO_BIT_5
#define EBIKE_GSENSOR_SDA0_GPIO        ((EBIKE_GSENSOR_SDA0_PORT<<GPIO_BIT_OFFSET)|EBIKE_GSENSOR_SDA0_GPIO_BIT)

//CS使能脚,高有效
#define EBIKE_GSENSOR_CS_PORT       GPIO_PORT_B
#define EBIKE_GSENSOR_CS_GPIO_BIT   GPIO_BIT_2
#define EBIKE_GSENSOR_CS_GPIO       ((EBIKE_GSENSOR_CS_PORT<<GPIO_BIT_OFFSET)|EBIKE_GSENSOR_CS_GPIO_BIT)

//SA0地址脚,配置I2C地址,可能不需要GPIO控制
#define EBIKE_GSENSOR_SA0_PORT      GPIO_PORT_B
#define EBIKE_GSENSOR_SA0_GPIO_BIT  GPIO_BIT_1
#define EBIKE_GSENSOR_SA0_GPIO      ((EBIKE_GSENSOR_SA0_PORT<<GPIO_BIT_OFFSET)|EBIKE_GSENSOR_SA0_GPIO_BIT)

//INT1
#define EBIKE_GSENSOR_INT1_PORT      GPIO_PORT_B
#define EBIKE_GSENSOR_INT1_GPIO_BIT  GPIO_BIT_3
#define EBIKE_GSENSOR_INT1_GPIO      ((EBIKE_GSENSOR_INT1_PORT<<GPIO_BIT_OFFSET)|EBIKE_GSENSOR_INT1_GPIO_BIT)

//INT2
#define EBIKE_GSENSOR_INT2_PORT       GPIO_PORT_D
#define EBIKE_GSENSOR_INT2_GPIO_BIT   GPIO_BIT_3
#define EBIKE_GSENSOR_INT2_GPIO       ((EBIKE_GSENSOR_INT2_PORT<<GPIO_BIT_OFFSET)|EBIKE_GSENSOR_INT2_GPIO_BIT)


/*************************** 加速度传感器 } ******************************/

/*************************** 坐垫锁（电机式） { ******************************/
//H桥sleep信号
#define EBIKE_BUCKET_SLEEP_PORT       GPIO_PORT_D
#define EBIKE_BUCKET_SLEEP_GPIO_BIT   GPIO_BIT_0
#define EBIKE_BUCKET_SLEEP_GPIO       ((EBIKE_BUCKET_SLEEP_PORT<<GPIO_BIT_OFFSET)|EBIKE_BUCKET_SLEEP_GPIO_BIT)

//H桥PWM1信号
#define EBIKE_BUCKET_PWM1_PORT        GPIO_PORT_D
#define EBIKE_BUCKET_PWM1_GPIO_BIT    GPIO_BIT_1
#define EBIKE_BUCKET_PWM1_GPIO        ((EBIKE_BUCKET_PWM1_PORT<<GPIO_BIT_OFFSET)EBIKE_BUCKET_PWM1_GPIO_BIT)

//H桥PWM2信号
#define EBIKE_BUCKET_PWM2_PORT        GPIO_PORT_D
#define EBIKE_BUCKET_PWM2_GPIO_BIT    GPIO_BIT_2
#define EBIKE_BUCKET_PWM2_GPIO        ((EBIKE_BUCKET_PWM2_PORT<<GPIO_BIT_OFFSET)|EBIKE_BUCKET_PWM2_GPIO_BIT)

//H桥中间的负载开关使能脚
#define EBIKE_BUCKET_SWITCH_EN_PORT         GPIO_PORT_B
#define EBIKE_BUCKET_SWITCH_EN_GPIO_BIT     GPIO_BIT_1
#define EBIKE_BUCKET_SWITCH_EN_GPIO         ((EBIKE_BUCKET_SWITCH_EN_PORT<<GPIO_BIT_OFFSET)|EBIKE_BUCKET_SWITCH_EN_GPIO_BIT)


/*************************** 坐垫锁（电机式） } ******************************/

/*************************** UART转485信号 { ******************************/
//使能脚
#define EBIKE_485_SWITCH_EN_PORT         GPIO_PORT_D
#define EBIKE_485_SWITCH_EN_GPIO_BIT     GPIO_BIT_7
#define EBIKE_485_SWITCH_EN_GPIO         ((EBIKE_BUCKET_SWITCH_EN_PORT<<4)|EBIKE_BUCKET_SWITCH_EN_GPIO_BIT)

/*************************** UART转485信号 } ******************************/

int32_t tuya_app_dev_init(void);


#ifdef __cplusplus
}
#endif
#endif //__TY_BLE_PIN_H__


