
#include "tuya_kel_system.h"
#include "driver_system.h"
#include "sys_utils.h"

uint32_t tuya_kel_sys_get_curr_time(void)
{
	return system_get_curr_time();
}

void tuya_kel_system_delay_ms(uint32_t ms)
{
    co_delay_100us(ms*10);
}

void tuya_kel_system_delay_us(uint32_t us)
{
    if(us<10) {
        us = 1;
    } else {
        us = us/10;
    }
    co_delay_10us(us);
}



