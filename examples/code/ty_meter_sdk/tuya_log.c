
#include "tuya_log.h"
#include "tuya_type.h"
#include "stdarg.h"

#define WSYS_LOG_STR_LEN_MAX	(256)

static char s_kal_log_buffer[WSYS_LOG_STR_LEN_MAX];
static uint8_t g_level_flag[MOD_MAX] = {
	MOD_PUBLIC_LEVEL,
	MOD_AUDIO_LEVEL,
	MOD_NV_LEVEL,
	MOD_OS_LEVEL,
	MOD_AUDIOE_LEVEL,
	MOD_BATTERY_LEVEL,
	MOD_DEV_LEVEL,
	MOD_TIMER_LEVEL,
	MOD_FSM_LEVEL,
	MOD_APP_LEVEL,
	MOD_DP_LEVEL,
	MOD_HID_LEVEL,
	MOD_PERIPH_LEVEL,
	MOD_SERVER_LEVEL,
	MOD_SENSOR_LEVEL,
	MOD_RCU433_LEVEL,
	MOD_POWER_MGR_LEVEL
};
__TUYA_WEAK__ void tuya_log_output(const char *log, size_t size)
{
	;
}

static int tuya_log_snprintf(char *str, int size, const char *format, ...)
{

    int len = 0;
    va_list ap;
    va_start(ap,format);

    len = vsnprintf(str,size,format, ap);
    va_end(ap);
    return len;
}

void ty_core_log(TY_LOG_MODULE_E mod_id,const char *func_name,unsigned int line, TY_LOG_LVL_E level, const char* format,...)
{
	#define LOG_SINGLE_LEN_MAX    64
    char* level_str = "N";
    va_list ap;
    char single_line[LOG_SINGLE_LEN_MAX] = {0};
	int len = 0;

	
	if(mod_id >= MOD_MAX || level == TY_LOG_LVL_NONE)
		return ;

	if(!(level&g_level_flag[mod_id]))
		return;

	memset(single_line,0,sizeof(single_line));
	memset(s_kal_log_buffer,0,sizeof(s_kal_log_buffer));
	va_start(ap,format);
    vsnprintf(s_kal_log_buffer,sizeof(s_kal_log_buffer),format, ap);
    va_end(ap);

    switch(level)
    {
        case TY_LOG_LVL_INFO:
            level_str = "I";
            break;
        case TY_LOG_LVL_WORRING:
            level_str = "W";
            break;
        case TY_LOG_LVL_ERROR:
            level_str = "E";
            break;
    }
    tuya_log_snprintf(single_line, sizeof(single_line), "[WSYS][%s|%s|%d]", level_str,func_name,line);

    len = strlen(single_line);
	len = strlen(single_line) > LOG_SINGLE_LEN_MAX ? LOG_SINGLE_LEN_MAX : len;
	tuya_log_output(single_line,len);

	len = strlen(s_kal_log_buffer);
	len = len > WSYS_LOG_STR_LEN_MAX ? WSYS_LOG_STR_LEN_MAX : len;
	tuya_log_output(s_kal_log_buffer,strlen(s_kal_log_buffer));
	tuya_log_output("\r\n",sizeof("\r\n"));
	
}


