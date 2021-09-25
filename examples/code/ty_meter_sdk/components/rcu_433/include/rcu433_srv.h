#ifndef __TUYA_RCU433_SRV_H__
#define __TUYA_RCU433_SRV_H__


#ifdef __cplusplus
extern "C" {
#endif


#include "tuya_type.h"
#include "tuya_mid_nv.h"


#define TY_RCU433_DEV_REG_MAX    2
#define RCU_ADDRESS1   1
#define RCU_ADDRESS2   2


typedef enum {
    EV1527_CODE_BIT_24  = 24,
    EV1527_CODE_BIT_28  = 28,
}RCU_CODE_BITS_E;
// 一般电动车的遥控器为一下4个按键功能
typedef enum {
    RCU_KEY_INVAILD = 0,
    RCU_KEY_LOCK,           // 落锁防盗
    RCU_KEY_UNLOCK,         // 解锁，并且解除防盗
    RCU_KEY_POWERON,        // 上电启动
    RCU_KEY_RING,           // 寻车，按下电瓶车发出铃声
}RCU_KEY_VALUE_E;

typedef enum {
    RCU_START_MONITOR      =   1,
    RCU_STOP_MONITOR,
    RCU_PAUSE_MONITOR,
    RCU_RESUME_MONITOR,
    RCU_LEARN_START,
    RCU_LEARN_STOP,
    RCU_GET_KEY,
    RCU_LEARN_SET,

}MSG_TYPE_E;

typedef struct {
    uint32_t phy_vaule;          //收到的遥控器的物理键值
    RCU_KEY_VALUE_E logic_key;  //业务逻辑键值
}RCU_KEY_MAP_T;


typedef struct {
    MSG_TYPE_E type;
    uint8_t rcu_addr[3];
    uint8_t keyvalue;
}RCU_MSG_T;

typedef enum {
    TYPE_KEY = 0,
    TYPE_LEARN,
}CALLBACK_TYPE;

/**
 * @brief 433遥控器获取到的事件回调函数，提供给应用接口使用
 *
 * @param type:TYPE_KEY获取到的遥控器的键值消息。TYPE_LEARN学习模式下获取的地址完成，
 *        这个时候忽略keyvalue
 * @param keyvalue ：键值。
 * @return void
 */
typedef void (*TY_RCU433_CALLBACK)(CALLBACK_TYPE type, int32_t keyvalue);


int tuya_rcu433_msg_send_internal(RCU_MSG_T rcu_msg);


/**
 * @brief 注册物理键值和逻辑键值的对应关系
 *
 * @param map 物理键值和逻辑兼职的map,
 * @param cnt ：map个数
 * @return void
 */
void tuya_rcu433_keyvalue_register(RCU_KEY_MAP_T *map,uint8_t cnt);

/**
 * @brief 初始化RCU433遥控器服务
 *
 * @param cb RCU433事件回调函数
 * @param data_pin ：RCU接收芯片的data脚
 * @param shut_pin : RCU接收芯片的断电脚
 * @param bits     : ev1527的编码位数
 * @return int32_t
 */
int32_t tuya_rcu433_service_init(TY_RCU433_CALLBACK cb,uint8_t data_pin,uint8_t shut_pin,
        RCU_CODE_BITS_E bits);

/**
 * @brief 启动遥控器接收功能
 *
 * @param timeout_us 433芯片的数据接收的定时器超时时间。
 * @return int32_t
 */
int32_t tuya_rcu433_service_start(uint32_t timeout_us);

/**
 * @brief 关闭遥控器接收功能
 *
 * @param
 * @return int32_t
 */
int32_t tuya_rcu433_service_stop(void);

/**
 * @brief 控制RCU433遥控器是否开启学习模式
 *
 * @param enable TRUE,开启;FALSE 关闭
 * @return int32_t
 */
int32_t tuya_rcu433_learn_contrl(bool enable);

/**
 * @brief 获取RCU433遥控器地址数量
 *
 * @param addr_cnt 获取的地址数量，0表示未学习过，非0表示已学习地址个数
 * @return int32_t
 */
int32_t tuya_rcu433_get_addr_nums(uint8_t *addr_cnt);






#ifdef __cplusplus
} // extern "C"
#endif

#endif // __COMPONENT_HELLO_H__
