/**                       版权所有 (C), 2015-2021, 涂鸦科技
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    locdef_config.h
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.12
 * @brief   该文件用于实现定防器参数配置管理
 *
******************************************************************************/

#ifndef __LOCDEF_AUDIO_H_
#define __LOCDEF_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_type.h"

typedef enum {
    PLAY_TYPE_ONCE = 0,
    PLAY_TYPE_LOOP,
} LOCDEF_TONE_PLAY_TYPE;

typedef enum {
    TONE_MUTE = 0x00,
    TONE_ALARM,
    TONE_SEARCH,
    TONE_SPEEDING,
    TONE_LOCK,
    TONE_UNLOCK,
    TONE_START_UP,
    TONE_STOP,
    TONE_MAX,
}TONE_TYPE_E;

typedef struct {
    TONE_TYPE_E tone_type;
    LOCDEF_TONE_PLAY_TYPE loop;
    uint32_t timeout;
} LOCDEF_AUDIO_PLAY_CTX_T;


#define TONE_ALARM_PATH                     "5"
#define TONE_SEARCH_PATH                    "1"
#define TONE_SPEEDING_PATH                  "5"
#define TONE_LOCK_PATH                      "5"
#define TONE_UNLOCK_PATH                    "4"
#define TONE_START_UP_PATH                  "6"
#define TONE_STOP_PATH                      "6"


int locdef_audio_init(void);

TONE_TYPE_E locdef_get_cur_play_tone(void);

int locdef_dev_audio_play(TONE_TYPE_E tone_type, LOCDEF_TONE_PLAY_TYPE loop, uint32_t timeout);

int locdef_dev_audio_stop(void);


#ifdef __cplusplus
}
#endif
#endif

