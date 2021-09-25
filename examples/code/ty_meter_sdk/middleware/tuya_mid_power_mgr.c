
#include "tuya_mid_power_mgr.h"
#include "tuya_log.h"
#include "tuya_kel_timer.h"

#define TUYA_PWR_MGR_MODULE_MAX    8

#define TUYA_SLEEP_TIME    (5*1000) //ms

typedef struct {
	uint32_t mod_bit_offset;
	tuya_power_mgr_cbk mod_cbk;
}tuya_core_sub_mod_t;

typedef struct {
	uint8_t sys_status;
	uint8_t sub_mod_index;//记录总的低功耗相关子模块的个数
	uint32_t mod_sleep_flag;
	uint32_t mod_all_flag;
	uint32_t wakeup_source;
	tuya_core_sub_mod_t sub_mode[TUYA_PWR_MGR_MODULE_MAX];
	tuya_kel_timer_handle timer_hanlder;
}tuya_core_power_mgr_t;


static tuya_core_power_mgr_t s_core_power_mgr = {0};


void tuya_mid_sleep_enable(void)
{
	tuya_kel_timer_start(s_core_power_mgr.timer_hanlder);
	//tuya_kel_sleep_enable();
}

void tuya_mid_sleep_disable(void)
{
	tuya_kel_timer_stop(s_core_power_mgr.timer_hanlder);
	//tuya_kel_sleep_disable();
}

void tuya_mid_mod_sleep_active(tuya_power_mgr_handle sub_mod_handle)
{
	tuya_core_sub_mod_t *sub_mod_info = (tuya_core_sub_mod_t *)sub_mod_handle;
	s_core_power_mgr.mod_sleep_flag |= (0x01 << sub_mod_info->mod_bit_offset);
}

void tuya_mid_mod_sleep_inactive(tuya_power_mgr_handle sub_mod_handle)
{
	tuya_core_sub_mod_t *sub_mod_info = (tuya_core_sub_mod_t *)sub_mod_handle;
	s_core_power_mgr.mod_sleep_flag &= ~(0x01 << sub_mod_info->mod_bit_offset);
}


TUYA_RET_E tuya_mid_wakeup_source_set(uint32_t flag)
{
	s_core_power_mgr.wakeup_source |= flag;
	return TUYA_OK;
}

uint32_t tuya_mid_wakeup_source_get(void)
{
	return s_core_power_mgr.wakeup_source;
}

void tuya_mid_wakeup_source_clear(uint32_t flag)
{
	s_core_power_mgr.wakeup_source &= ~flag;
}

void tuya_mid_wakeup_source_reset(void)
{
	s_core_power_mgr.wakeup_source = 0;
}

tuya_power_mgr_handle tuya_mid_pwr_mgr_register(tuya_power_mgr_cbk cbk)
{
	int i =0;

	for(i = 0; i < TUYA_PWR_MGR_MODULE_MAX;i++)
	{
		if(0 == s_core_power_mgr.sub_mode[i].mod_cbk)
		{
			s_core_power_mgr.sub_mode[i].mod_cbk = cbk;
			s_core_power_mgr.sub_mode[i].mod_bit_offset = s_core_power_mgr.sub_mod_index;
			s_core_power_mgr.sub_mod_index++;
			s_core_power_mgr.mod_all_flag = ((s_core_power_mgr.mod_sleep_flag<<0x01) | 0x01);
			return &s_core_power_mgr.sub_mode[i];
		}
	}

	TUYA_LOG_E(MOD_POWER_MGR," mode cbk buff is full !!!!!!!!");
	return NULL;
}

static void tuya_sys_mod_suspend(void)
{
	int i =0;

	for(i = 0; i < TUYA_PWR_MGR_MODULE_MAX;i++)
	{
		if(0 != s_core_power_mgr.sub_mode[i].mod_cbk)
		{
			s_core_power_mgr.sub_mode[i].mod_cbk(SYS_EVT_SUSPEND);
		}
	}
}

static void tuya_sys_mod_resume(void)
{
	int i =0;

	for(i = 0; i < TUYA_PWR_MGR_MODULE_MAX;i++)
	{
		if(0 != s_core_power_mgr.sub_mode[i].mod_cbk)
		{
			s_core_power_mgr.sub_mode[i].mod_cbk(SYS_EVT_RESUME);
		}
	}
	s_core_power_mgr.mod_sleep_flag = 0;
	s_core_power_mgr.sys_status = SYS_ACTIVE;
	tuya_kel_timer_start(s_core_power_mgr.timer_hanlder);
}

static void tuya_core_power_mgr_timeout_cb(void* param)
{
	TUYA_LOG_I(MOD_POWER_MGR,"flag==[0x%X][0x%X]",s_core_power_mgr.mod_sleep_flag,s_core_power_mgr.mod_all_flag);
    if(s_core_power_mgr.mod_sleep_flag == s_core_power_mgr.mod_all_flag)
	{
		TUYA_LOG_I(MOD_POWER_MGR,"ready to sleep mode");
		tuya_sys_mod_suspend();
		s_core_power_mgr.sys_status = SYS_DEEP_SLEEP;
		tuya_mainloop_timer_stop();
		tuya_kel_sleep_enable();
	}else{
		TUYA_LOG_I(MOD_POWER_MGR,"check sleep mode again");
		tuya_kel_timer_restart(s_core_power_mgr.timer_hanlder,TUYA_SLEEP_TIME);
	}
}

TUYA_RET_E tuya_mid_power_mgr_init(void)
{
	memset(&s_core_power_mgr,0,sizeof(s_core_power_mgr));
	tuya_kel_sleep_disable();
 
	s_core_power_mgr.sys_status = SYS_ACTIVE;

	tuya_kel_timer_create(&s_core_power_mgr.timer_hanlder, TUYA_SLEEP_TIME, TY_KEL_TIMER_SINGLE_SHOT, tuya_core_power_mgr_timeout_cb);
	if(NULL == s_core_power_mgr.timer_hanlder)
	{
		TUYA_LOG_E(MOD_POWER_MGR,"create power manager timer failed");
		return TUYA_COMMON_ERROR;
	}
    tuya_kel_timer_start(s_core_power_mgr.timer_hanlder);

	return TUYA_OK;
}





