#ifndef __TY_VFM_LOCK_H_
#define __TY_VFM_LOCK_H_


#ifdef __cplusplus
extern "C" {
#endif
  typedef enum {
    LOCK_TYPE_STEERING_LOCK, /*龙头锁*/
    LOCK_TYPE_CUSHION_LOCK,	 /*坐筒锁*/
    LOCK_TYPE_MOTOR_LOCK,	 /*电机锁*/
    LOCK_TYPE_DEF_LOCK,      /*布防锁 */    //布防锁的LOCK 和 UNLOCK 状态是反的
    LOCK_TYPE_USER_LOCK,	 /*用户自定义*/
  }VFM_LOCK_TYPE_E;
  typedef enum {
    UNLOCK,
    LOCK,
  }LOCK_ACTION_E;

  typedef struct {
     /**
     * @brief 锁的开关功能
     * @param type
     * @param action
     * @return 0: success  Other: fail
     */
    int (*ctrl)(VFM_LOCK_TYPE_E type,LOCK_ACTION_E action);
    int (*read)(VFM_LOCK_TYPE_E type,LOCK_ACTION_E *action);
  }tyVFMLockIntf_t;

  int ty_vfm_lock_reg(tyVFMLockIntf_t *intf);

  /**
  * @brief 锁的开关功能
  * @param type
  * @param action
  * @return 0: success  Other: fail
  */
  int ty_vfm_lock_ctrl(VFM_LOCK_TYPE_E type,LOCK_ACTION_E action);
  /**
  * @brief 获取锁的状态
  * @param type
  * @param *action
  * @return 0: success  Other: fail
  */
  int ty_vfm_lock_read(VFM_LOCK_TYPE_E type,LOCK_ACTION_E *action);

#ifdef __cplusplus
}
#endif

#endif
