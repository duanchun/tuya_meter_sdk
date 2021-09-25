#include "ty_rtc.h"
#include "driver_rtc.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
static uint32_t s_local_timestamp = 0;
static uint32_t s_local_timestamp_when_update = 0;
static uint32_t s_app_timestamp_when_update = 0;

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
__attribute__((section("ram_code"))) void rtc_isr_ram(uint8_t rtc_idx)
{
    if(rtc_idx == RTC_A) {
        s_local_timestamp++;
    }
    //ty_rtc_init();
		if(rtc_get_tick()==0xffffffff)
    {
         ty_rtc_init();
    }
}

/*********************************************************
FN: 
*/
uint32_t ty_rtc_init(void)
{
    rtc_init();
    rtc_alarm(RTC_A, 1000);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_rtc_set_time(uint32_t timestamp)
{
    s_local_timestamp_when_update = s_local_timestamp;
    s_app_timestamp_when_update = timestamp;
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_rtc_get_time(uint32_t* p_timestamp)
{
    *p_timestamp = (s_app_timestamp_when_update + (s_local_timestamp - s_local_timestamp_when_update));
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_rtc_get_local_time(uint32_t* p_timestamp)
{
    *p_timestamp = s_local_timestamp;
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_rtc_get_old_time(uint32_t old_local_timestamp, uint32_t* p_timestamp)
{
    uint32_t timestamp = 0;
    ty_rtc_get_time(&timestamp);
    *p_timestamp = (timestamp - (s_local_timestamp - old_local_timestamp));
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_rtc_control(uint8_t cmd, void* arg)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_rtc_uninit(void)
{
    rtc_disalarm(RTC_A);
    return 0;
}













