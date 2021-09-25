

/**
****************************************************************************
* @file      tuya_kel_audio.h
* @brief     tuya_kel_audio
* @author    xiaojian
* @version   V0.0.1
* @date      2021-09
* @note
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2021 Tuya </center></h2>
*/

#ifndef __TUYA_KEL_AUDIO_H__
#define __TUYA_KEL_AUDIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "tuya_type.h"

typedef enum
{
	TY_AUDIO_STATUS_IDLE = 0,
	TY_AUDIO_STATUS_BUSY
}TY_AUDIO_STATUS_E;


typedef enum
{
	TY_KEL_AUDIO_PLAY_START = 0x01,
	TY_KEL_AUDIO_PLAY_SUSPEND = 0x02,
	TY_KEL_AUDIO_PLAY_FINISH = 0x03,
	TY_KEL_AUDIO_PLAY_ERROR = 0x04
}TUYA_KEL_AUDIO_PLAY_EVT_E;

typedef enum
{
	TY_KEL_AUDIO_FORMAT_UNKNOWN, ///< placeholder for unknown format
	TY_KEL_AUDIO_FORMAT_PCM,	 ///< raw PCM data
	TY_KEL_AUDIO_FORMAT_WAVPCM,  ///< WAV, PCM inside
	TY_KEL_AUDIO_FORMAT_MP3,	 ///< MP3
	TY_KEL_AUDIO_FORMAT_AMRNB,	 ///< AMR-NB
	TY_KEL_AUDIO_FORMAT_AMRWB,	 ///< AMR_WB
	TY_KEL_AUDIO_FORMAT_SBC 	///< bt SBC
} TUYA_KEL_AUDIO_FORMAT_E;


typedef void (*tuya_kel_audio_play_cb)(void *user_param,TUYA_KEL_AUDIO_PLAY_EVT_E event);

TUYA_RET_E tuya_kel_audio_play_start(char* file_path,TUYA_KEL_AUDIO_FORMAT_E format, tuya_kel_audio_play_cb cbk, uint8_t loop, void *user_param);

TUYA_RET_E tuya_kel_audio_play_stop(void);

void tuya_kel_audio_init(void);


#ifdef __cplusplus
}
#endif

#endif



