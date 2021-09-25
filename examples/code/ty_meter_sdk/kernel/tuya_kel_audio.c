
#include "tuya_kel_audio.h"

__TUYA_WEAK__ TUYA_RET_E tuya_kel_audio_play_start(char* file_path,TUYA_KEL_AUDIO_FORMAT_E format, tuya_kel_audio_play_cb cbk, uint8_t loop, void *user_param)
{
	return TUYA_OK;
}

__TUYA_WEAK__ TUYA_RET_E tuya_kel_audio_play_stop(void)
{
	return TUYA_OK;
}

__TUYA_WEAK__ void tuya_kel_audio_init(void)
{
	;
}



