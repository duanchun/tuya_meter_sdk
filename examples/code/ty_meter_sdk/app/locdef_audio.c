/*
 Copyright (c) 2021 Tuya Inc

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * @file    locdef_audio.c
 * @author  CNIOT-tianyu
 * @version v0.0.1
 * @date    2021.08.12
 * @brief   该文件用于实现音频相关功能
 *
******************************************************************************/
#include "vfm_base.h"
#include "locdef_audio.h"
#include "locdef_config.h"
#include "locdef_pin.h"
#include "locdef_dp.h"
#include "locdef_service.h"
#include "tuya_kel_audio.h"
#include "tuya_ble_mutli_tsf_protocol.h"//TODO
#include "tuya_log.h"
#include "tuya_kel_timer.h"
#include "tuya_kel_system.h"

static tuya_kel_timer_handle s_audio_player_timer = NULL;//音频播放器定时器           
static volatile TONE_TYPE_E g_cur_play_tone = TONE_MUTE;

static int locdef_audio_play_tone(TONE_TYPE_E tone_type, LOCDEF_TONE_PLAY_TYPE loop);
static void locdef_audio_player_timeout_cb(void* param);

/**
 * @brief   获取音频文件地址
 *
 * @param   type 音频类型
 * @param
 *
 * @return  无
 */
char* tuya_audio_get_file_path(TONE_TYPE_E type)
{
    static char* tone_path[] = {
        NULL,                // TONE_MUTE
        TONE_ALARM_PATH,     // TONE_ALARM
        TONE_SEARCH_PATH,    // TONE_SEARCH
        TONE_SPEEDING_PATH,  // TONE_SPEEDING
        TONE_LOCK_PATH,      // TONE_LOCK
        TONE_UNLOCK_PATH,    // TONE_UNLOCK
        TONE_START_UP_PATH,  // TONE_START_UP
        TONE_STOP_PATH,      // TONE_STOP
    };

    return (type > TONE_STOP) ? NULL : tone_path[type];
}

/**
 * @brief   获取当前播放的音频
 *
 * @return  无
 */
TONE_TYPE_E locdef_get_cur_play_tone(void)
{
    return g_cur_play_tone;
}


/**
 * @brief   音频单次播放回调
 *
 * @param
 * @param
 *
 * @return  无
 */
static void tuya_audio_once_player_cb(void *param, TUYA_KEL_AUDIO_PLAY_EVT_E event)
{
    if (g_cur_play_tone != TONE_SPEEDING) {
        g_cur_play_tone = TONE_MUTE;
    }

    TUYA_LOG_I(MOD_AUDIOE,"audio play once callback");
}


/**
 * @brief   音频循环播放回调
 *
 * @param
 * @param
 *
 * @return  无
 */
static void tuya_audio_loop_player_cb(void *param, TUYA_KEL_AUDIO_PLAY_EVT_E event)
{
}


static int locdef_audio_play_tone(TONE_TYPE_E tone_type, LOCDEF_TONE_PLAY_TYPE loop)
{
    int op_ret = 0;
    char* tone_path = NULL;
    static LOCDEF_AUDIO_PLAY_CTX_T play_ctx;

    if (tone_type > TONE_MAX) {
        TUYA_LOG_E(MOD_AUDIOE,"tone type error: %d", tone_type);
        return -1;
    }

    tuya_kel_audio_play_stop(); 
    g_cur_play_tone = tone_type;

    tone_path = tuya_audio_get_file_path(g_cur_play_tone);
	memset(&play_ctx,0,sizeof(play_ctx));
    if (loop == PLAY_TYPE_LOOP) {
        play_ctx.tone_type = g_cur_play_tone;
        play_ctx.loop = loop;
        play_ctx.timeout = 0;
        /* 增加判断,避免tone_path出现NULL的情况 */
        if (tone_path){
			tuya_kel_audio_play_start(tone_path,TY_KEL_AUDIO_FORMAT_WAVPCM,tuya_audio_loop_player_cb,true,&play_ctx);
        }

    } else {
        /* 增加判断,避免tone_path出现NULL的情况 */
        if (tone_path){
            tuya_kel_audio_play_start(tone_path,TY_KEL_AUDIO_FORMAT_WAVPCM,tuya_audio_once_player_cb,false,&play_ctx);
        }

    }

    tuya_kel_system_delay_ms(100);

    if (op_ret == 0) {
        TUYA_LOG_E(MOD_AUDIOE,"audio play tone %s succ, loop: %d, cur_tone_type: %d", tone_path, loop, g_cur_play_tone);
    } else {
        TUYA_LOG_E(MOD_AUDIOE,"audio play tone: %s failed, loop: %d, cur_tone_type: %d", tone_path, loop, g_cur_play_tone);
    }

    return op_ret;
}

/**
 * @brief   音频播放
 *
 * @param   file_path 提示音路径
 * @param
 *
 * @return  无
 */
int locdef_dev_audio_play(TONE_TYPE_E tone_type, LOCDEF_TONE_PLAY_TYPE loop, uint32_t timeout)
{
    int op_ret = 0;

    op_ret = locdef_audio_play_tone(tone_type, loop);
    if (0 != op_ret)
        return op_ret;

    if (timeout > 0) {
		if(NULL != s_audio_player_timer)
		{
		    TUYA_LOG_I(MOD_AUDIOE,"delete locdef audio play timer");
		    tuya_kel_timer_stop(s_audio_player_timer);
			tuya_kel_timer_delete(s_audio_player_timer);
			s_audio_player_timer = NULL;
				
		}
        tuya_kel_timer_create(&s_audio_player_timer, timeout, TY_KEL_TIMER_SINGLE_SHOT, locdef_audio_player_timeout_cb);
		if(NULL == s_audio_player_timer)
		{
			TUYA_LOG_E(MOD_AUDIOE,"create waring audio play timer failed");
			return 1;
		}
		TUYA_LOG_I(MOD_AUDIOE,"timeout==%u",timeout);
        tuya_kel_timer_start(s_audio_player_timer);
    }

    return 0;
}

static void locdef_audio_stop_tone(void)
{
    tuya_kel_audio_play_stop();
    g_cur_play_tone = TONE_MUTE;
    TUYA_LOG_I(MOD_AUDIOE,"audio player stop");
}

/**
 * @brief   音频停止
 *
 * @param
 * @param
 *
 * @return  无
 */
int locdef_dev_audio_stop(void)
{
    if(NULL != s_audio_player_timer)
	{
	    TUYA_LOG_I(MOD_AUDIOE,"locdef audio stop");
		tuya_kel_timer_stop(s_audio_player_timer);
	}
    locdef_audio_stop_tone();

    return 0;
}


/**
 * @brief   音频播放器播放超时回调
 *
 * @param   timerID   定时器id
 * @param   pTimerArg 定时器函数处理参数
 *
 * @return  无
 */
static void locdef_audio_player_timeout_cb(void* param)
{
    //DEV_RUNNING_PARAM_T *run_param = locdef_get_run_param();
    tuya_dev_cfg_t *cfg_param = locdef_get_dev_cfg_param();

    TUYA_LOG_I(MOD_AUDIOE,"audio player timeout g_cur_play_tone:%d", g_cur_play_tone);

    if (g_cur_play_tone == TONE_SPEEDING) {
		
		locdef_dev_audio_play(TONE_SPEEDING, PLAY_TYPE_ONCE,cfg_param->limit_speed_alarm_interval*1000);

    } else {
        if (g_cur_play_tone == TONE_SEARCH) {
            int bool_res = false;
            locdef_dp_update(DPID_SEARCH, DT_BOOL, 0, &bool_res); //TODO
        } else if (g_cur_play_tone == TONE_ALARM) {
            //run_param->dev_status = DEVICE_NORMAL;
        }
        locdef_dev_audio_stop();
    }

    TUYA_LOG_I(MOD_AUDIOE,"audio player timeout");
}


/**
 * @brief   音频模块初始化
 *
 * @param
 * @param
 *
 * @return
 */
int locdef_audio_init(void)
{
    return 0;
}

