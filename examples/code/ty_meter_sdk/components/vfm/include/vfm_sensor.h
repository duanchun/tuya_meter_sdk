#ifndef __TY_VFM_SENSOR_H_
#define __TY_VFM_SENSOR_H_

#include "vfm_base.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SENSOR_EVENT_LEFT = VFM_SHNSOR_EVENT_START,/*左转*/
  SENSOR_EVENT_RIGHT,                        /*右转*/ //0x02
  SENSOR_EVENT_TRUN,                         /*转弯*/
  SENSOR_EVENT_KNOCK,                        /*碰撞*/
  SENSOR_EVENT_SHAKE,                        /*震动*/ //0x05
  SENSOR_EVENT_SKEW,                         /*倾倒*/
  SENSOR_EVENT_ROTATION,                     /*轮动*/
  SENSOR_EVENT_UNROTATION,                   /*轮动释放*/ //0x08
  SENSOR_EVENT_DEFENCE,                      /*设防*/
  SENSOR_EVENT_UNDEFENCE,                    /*解防*/ //0x0A
  SENSOR_EVENT_ACCON,                        /*点火开关事件*/
  SENSOR_EVENT_ACCOFF,                       /*熄火开关事件*/ //熄火&设防
  SENSOR_EVENT_REMOTE_ACCON,                 /*远程点火*/ //0x0D     //撤防&点火
  SENSOR_EVENT_SEARCH,                       /*寻车*/
  SENSOR_EVENT_CUSHION,                      /*坐垫锁*/
  SENSOR_EVENT_OVERSPEED,                    /*超速*/ //0x10
  SENSOR_EVENT_SAFESPEED,                    /*安全速度*/
  SENSOR_EVENT_MAX,
}SENSOR_EVENT_E;

typedef enum {
  ACC_POWER_OFF,
  ACC_POWER_ON,
}ACC_POWER_E;

typedef struct {
  int (*read_speed)(uint32_t *speed);
  int (*read_mileage)(float *mileage);
  int (*read_total_mileage)(float *allmileage);
  int (*read_error)(uint32_t *err);
  int (*ctrl_acc)(ACC_POWER_E action);
  int (*read_acc)(ACC_POWER_E *type);
}tyVFMSensorIntf_t;


int ty_vfm_sensor_reg(tyVFMSensorIntf_t *intf);


/**
* @brief 发送传感器的事件。当设备的传感器发送某些事件的时候，用户调用该接口把事件
          发送给VFM.
* @param event
* @return 0: success  Other: fail
*/
int ty_vfm_sensor_event_post(SENSOR_EVENT_E event);

/**
* @brief 读取速度
* @param speed 单位100M/H
* @return 0: success  Other: fail
*/
int ty_vfm_sensor_read_speed(uint32_t *speed);
/**
* @brief 获取单次里程
* @param *mileage  单位KM
* @return 0: success  Other: fail
*/
int ty_vfm_sensor_read_mileage(float *mileage);

/**
* @brief 获取总里程
* @param *mileage 单位KM
* @return 0: success  Other: fail
*/
int ty_vfm_sensor_read_total_mileage(float *allmileage);

/**
* @brief 获取故障码
* @param *error
* @return 0: success  Other: fail
*/
int ty_vfm_sensor_read_error(uint32_t *error);

/**
* @brief ACC的控制功能
* @param type
* @param action
* @return 0: success  Other: fail
*/
int ty_vfm_acc_ctrl(ACC_POWER_E action);
/**
* @brief 获取ACC的状态
* @param type
* @param *action
* @return 0: success  Other: fail
*/
int ty_vfm_acc_read(ACC_POWER_E *action);
#ifdef __cplusplus
}
#endif

#endif
