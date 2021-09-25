#include "locdef_main.h"
#include "vfm_fsm.h"
#include "tuya_log.h"
#include "tuya_kel_timer.h"
#include "tuya_kel_os.h"
#include "locdef_service.h"
#include "locdef_config.h"
#include "locdef_audio.h"
#include "vfm_base.h"


void tuya_app_init(void)
{
	int op_ret = 0;
	uint32_t fsm_init_state = FSM_ACCOFF_DEF; 

	tuya_kel_audio_init();

	//tuya_mid_power_mgr_init();

	locdef_param_init();

    /* 音频初始化 */
    locdef_audio_init();
	
	device_lock_register();
	device_vbat_register();
    device_sensor_register();
	
	tuya_gpio_init();

	/* 设备上电处理 */
    locdef_dev_power_on_handle(&fsm_init_state);

    /* 初始化状态机 */
    locdef_fsm_register(fsm_init_state);

	tuya_fsm_init();

	/* 注册上报接口 */
    locdef_dp_uplink_register();
	
	tuya_rcu_433_mgr_init();
	
}

