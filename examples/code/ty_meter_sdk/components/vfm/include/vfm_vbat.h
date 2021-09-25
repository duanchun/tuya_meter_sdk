#ifndef __TY_VFM_VBAT_H_
#define __TY_VFM_VBAT_H_

#include "vfm_base.h"

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum{
    VBAT_INTERNAL,	/*内部电池*/
    VBAT_EXTION,	/*外部电池*/
  }VBAT_TYPE_E;


  typedef enum {
    VBAT_CHG_EVENT_START = VFM_VBAT_EVENT_START,
    VBAT_CHG_EVENT_FINISH,
    VBAT_CHG_EVENT_WARNING,
    VBAT_CHG_EVENT_BATT_OFF,
    VBAT_CHG_EVENT_CAPACITY,
    VBAT_CHG_EVENT_DISCONNECT,
    VBAT_CHG_EVENT_FAULT,
  }VBAT_EVENT_T;

  typedef struct {
    int (*read_rsoc)(VBAT_TYPE_E type,uint8_t *rsoc);
    int (*read_voltage)(VBAT_TYPE_E type,int *mV);
    int (*read_current)(VBAT_TYPE_E type,int *mA);
    int (*read_temperature)(VBAT_TYPE_E type,int *temp);
  }tyVFMVbatIntf_t;

  int ty_vfm_vbat_reg(tyVFMVbatIntf_t *info);
  /**
  * @brief 发送电池的事件。当电池的发生某些事件的时候，用户调用该接口把事件
           发送给VFM.
  * @param event
  * @return 0: success  Other: fail
  */
  int ty_vfmk_vbat_event_post(VBAT_TYPE_E type,VBAT_EVENT_T event);

  /**
  * @brief 获取电池剩余电量百分比
  * @param rsoc 剩余电量百分比
  * @return 0: success  Other: fail
  */
  int ty_vfm_cellular_read_rsoc(VBAT_TYPE_E type,uint8_t *rsoc);

 /**
 * @brief 获取设备电源当前输出电压。
 *
 * @param type 电池类型
 * @param mV
 * return 0: success  Other: fail
 *
 **/
  int ty_vfm_vbt_read_voltage(VBAT_TYPE_E type,int *mV);

 /**
 * @brief 获取电池的输出电流。
 *
 * @param type 电池类型
 * @param mA
 * return 0: success  Other: fail
 *
 **/
  int ty_vfm_vbt_read_current(VBAT_TYPE_E type,int *mA);

 /**
 * @brief 获取电池的温度。
 *
 * @param type 电池类型
 * @param temp温度，单位摄氏度
 * return 0: success  Other: fail
 *
 **/
  int ty_vfm_vbt_read_temperature(VBAT_TYPE_E type,int *temp);


#ifdef __cplusplus
}
#endif

#endif
