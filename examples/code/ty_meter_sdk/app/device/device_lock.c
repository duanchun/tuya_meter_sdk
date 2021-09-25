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
#include "locdef_pin.h"
#include "vfm_lock.h"
#include "driver_exti.h" //TODO
#include "vfm_base.h"
#include "tuya_log.h"

static int device_lock_ctrl(VFM_LOCK_TYPE_E type,LOCK_ACTION_E action);
static int device_lock_read(VFM_LOCK_TYPE_E type,LOCK_ACTION_E *action);


static tyVFMLockIntf_t m_intf = {
    .ctrl = device_lock_ctrl,
    .read = device_lock_read,
};

static int device_lock_ctrl(VFM_LOCK_TYPE_E type,LOCK_ACTION_E action)
{
    int ret = 0;
    switch(type)
    {
        case LOCK_TYPE_STEERING_LOCK:
            break;
        case LOCK_TYPE_CUSHION_LOCK:
            if (action == UNLOCK) {
                locdef_cushion_lock(); 
            }
            break;
        case LOCK_TYPE_MOTOR_LOCK:
            if (action == LOCK) {
                ret = tuya_pin_write(EBIKE_MOTOR_LOCK_GPIO, TUYA_PIN_HIGH);
            }
            else {
                ret = tuya_pin_write(EBIKE_MOTOR_LOCK_GPIO, TUYA_PIN_LOW);
            }
            break;
        case LOCK_TYPE_DEF_LOCK:
            if (action == LOCK) {
                locdef_vib_alarm_ctrl(true);
            }
            else {
                locdef_vib_alarm_ctrl(false);
            }
            break;
        default:break;
    }
    return ret;
}


static int device_lock_read(VFM_LOCK_TYPE_E type,LOCK_ACTION_E *action)
{
    int ret = 0;
    switch(type)
    {
        case LOCK_TYPE_STEERING_LOCK:
            break;
        case LOCK_TYPE_CUSHION_LOCK:
            break;
        case LOCK_TYPE_MOTOR_LOCK:
			#if 0
			if(TUYA_PIN_HIGH == tuya_pin_read(EBIKE_MOTOR_LOCK_GPIO))
			{
			    *action = LOCK;
			} else {
				*action = UNLOCK;
			}
			#endif
            break;
        case LOCK_TYPE_DEF_LOCK:
			
            if (locdef_vib_alarm_enable()) {
                *action = UNLOCK;
            } else {
                *action = LOCK;
            }
            break;
        default:break;
    }
    return ret;
}



/**
 * @brief   电门状态检测回调处理函数
 *
 * @param
 *
 * @return  void
 */
void locdef_elec_door_det_irq_cb(void *args)
{
	tuya_pin_irq_disable(EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO);
    locdef_io_filter_notify(EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO, 0);
    //TUYA_LOG_I(MOD_DEV," locdef_elec_door_det_irq_cb");
}





/**
 * @brief   电机锁、电门锁状态、坐垫锁，GPIO引脚初始化
 *
 * @return
 *
 * @note
 */
int locdef_gpio_lock_init(void)
{
    int op_ret = 0;

    /* 电机锁引脚配置，高有效 */
    TUYA_GPIO_INIT(EBIKE_MOTOR_LOCK,GPIO_DIR_OUT,0);

    //电门锁检测脚，输入，边沿触发中断(RF8018不支持边沿中断，所以要加逻辑处理)
    pmu_set_pin_to_CPU(EBIKE_ELECTRIC_DOOR_LOCK_DET_PORT, (1<<EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO_BIT));
    system_set_port_mux(EBIKE_ELECTRIC_DOOR_LOCK_DET_PORT,EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO_BIT,PORTD4_FUNC_D4);
	gpio_set_dir(EBIKE_ELECTRIC_DOOR_LOCK_DET_PORT,EBIKE_ELECTRIC_DOOR_LOCK_DET_GPIO_BIT,GPIO_DIR_IN);
	ext_int_set_port_mux(EXTI_11,EXTI_11_PD4);
    ext_int_set_type(EXTI_11, EXT_INT_TYPE_POS);
    ext_int_enable(EXTI_11);

    //坐垫锁控制（电磁式）,输出，高有效，持续500ms
    TUYA_GPIO_INIT(EBIKE_BUCKET_LOCK,GPIO_DIR_OUT,1);

    return op_ret;
}

int device_lock_register(void)
{
    int op_ret = 0;
    op_ret = locdef_gpio_lock_init();
    ty_vfm_lock_reg(&m_intf);
    return op_ret;
}





