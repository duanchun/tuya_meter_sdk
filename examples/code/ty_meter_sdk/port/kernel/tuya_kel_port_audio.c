
#include "tuya_kel_audio.h"
#include "driver_plf.h"
#include "core_cm3.h"
#include "os_task.h"
#include "os_msg_q.h"
#include "driver_flash.h"
#include "co_printf.h"
#include <string.h>
#include "os_mem.h"
#include "adpcm_ms.h"
#include "adpcm_ima.h"
#include "decoder.h"
#include "driver_i2s.h"
#include "tuya_log.h"
#include "tuya_type.h"
#include "locdef_pin.h" //TODO

#define TY_AUDIO_BUFF_USE_HEAP   

#define TY_AUDIO_TONE_MAX    6

#if 1 //TODO
#define TY_PA_ENABLE    tuya_pin_write(EBIKE_PA_EN_GPIO, TUYA_PIN_HIGH);
#define TY_PA_DISABLE    tuya_pin_write(EBIKE_PA_EN_GPIO, TUYA_PIN_LOW);
#else
#define TY_PA_ENABLE    
#define TY_PA_DISABLE    
#endif


typedef struct {
    tuya_kel_audio_play_cb user_cbk;
	void *user_param;
	TY_AUDIO_STATUS_E status; 
	uint8_t audio_id;
	uint8_t loop;
	uint8_t stop;
}ty_audio_core_t;

typedef struct{
    uint32_t voice_id;       //单个音频ID     
    uint32_t voice_addr;    //单个音频地址
    uint16_t voice_len;     //单个音频长度
    uint16_t reserve;       //预留2个字节
}ty_voice_single_t;

typedef struct{
    uint32_t amount;     //音频个数
    uint32_t total_len;    //音频总长度 
    ty_voice_single_t single[TY_AUDIO_TONE_MAX];
}ty_voice_header_t;

/************************************** speaker { ************************************/
static void ty_speaker_start_hw(void);
static void ty_speaker_stop_hw(void);
static void ty_speaker_init(void);
/************************************** speaker } ************************************/

#define DEC_DBG FR_DBG_OFF
#define DEC_LOG FR_LOG(DEC_DBG)
#define ADPCM_SEL_MS
#define DECODER_STORE_TYPE_RAM      0
#define DECODER_STORE_TYPE_FLASH    1

#define TY_AUDIO_FLASH_BASE_ADDR    0x64000
#define TY_AUDIO_BUFF_SIZE    (4*1024)
enum decoder_event_t
{
    DECODER_EVENT_START,
    DECODER_EVENT_PREPARE,
    DECODER_EVENT_NEXT_FRAME,
    DECODER_EVENT_STOP,
    DECODER_EVENT_LOOP
};

static ty_audio_core_t s_audio_core;

static uint16_t ty_decoder_calc_adpcm_ms_frame_len(uint8_t **header_org);
static int ty_read_sbc_from_flash(uint8_t * s_sbc_buff, uint32_t read_len);
static void ty_decoder_start(uint32_t start, uint32_t end, uint32_t tot_data_len, 
	uint16_t frame_len, uint32_t start_offset, uint8_t type);
static void ty_decoder_start(uint32_t start, uint32_t end, uint32_t tot_data_len, 
    uint16_t frame_len, uint32_t start_offset, uint8_t type);
static void ty_decoder_stop(void);
static void ty_decoder_stop(void);
static void ty_decoder_update_tot_data_len(uint32_t len);
static void ty_decoder_half_processed(void);
static void ty_decoder_play_next_frame_handler(void *arg);
static void ty_decoder_end_func(void);
static int ty_speaker_from_flash(void);
int ty_audio_play_with_id(uint8_t id);


static sbc_store_info_t s_sbc_sotre_env = {0};
static uint8_t *s_sbc_buff = NULL;
static speaker_env_t s_speaker_env = {0};
static bool s_decoder_hold_flag = false;
static struct decoder_env_t s_decoder_env;
static uint8_t s_stop_flag = 0;
static uint8_t s_decodeTASKState = DECODER_STATE_IDLE;//函数decoder_play_next_frame_handler 中的状态
static uint8_t s_Flash_data_state = true;
static uint16_t s_audio_task_id;
static os_timer_t s_start_delay_timer = {0};//为了解决循环播放过程中，播放其他音频 播放不出来的问题
#ifdef TY_AUDIO_BUFF_USE_HEAP
static uint8_t s_audio_buff[TY_AUDIO_BUFF_SIZE] = {0};
#endif

static uint16_t ty_decoder_calc_adpcm_ms_frame_len(uint8_t **header_org)
{
    uint32_t len;
    uint8_t *header = *header_org;
    uint16_t frame_size;

    if(memcmp(header, "RIFF", 4) == 0)
    {
        header += 12;
        if(memcmp(header, "fmt ", 4) == 0)
        {
            header += 4;
            len = header[0];
            len |= (header[1] << 8);
            len |= (header[2] << 16);
            len |= (header[3] << 24);

            frame_size = header[16];
            frame_size |= (header[17] << 8);
            header += 4;
            header += len;
            while(memcmp(header, "data", 4) != 0)
            {
                header += 4;
                len = header[0];
                len |= (header[1] << 8);
                len |= (header[2] << 16);
                len |= (header[3] << 24);
                header += (len + 4);
            }
            header += 8;
            TUYA_LOG_I(MOD_AUDIO,"decoder_calc_adpcm_ms_frame_len: %08x, %08x.\r\n", header, *header_org);
            *header_org = header;
            return frame_size;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

static int ty_read_sbc_from_flash(uint8_t * s_sbc_buff, uint32_t read_len)
{
    uint32_t r_len = read_len;
    uint32_t pos = 0;
    while(r_len > 0)
    {
        if( s_speaker_env.last_read_page_idx < s_sbc_sotre_env.last_page_idx )
        {
            //每次读一页大小的数据
            if( (r_len + s_speaker_env.last_read_offset) >= FLASH_PAGE_SIZE )
            {
                flash_read(s_sbc_sotre_env.start_base + s_speaker_env.last_read_page_idx * FLASH_PAGE_SIZE + s_speaker_env.last_read_offset
                           ,FLASH_PAGE_SIZE - s_speaker_env.last_read_offset, s_sbc_buff + pos);
                r_len -= (FLASH_PAGE_SIZE - s_speaker_env.last_read_offset);
                pos += (FLASH_PAGE_SIZE - s_speaker_env.last_read_offset);
                s_speaker_env.last_read_offset = 0;
                s_speaker_env.last_read_page_idx++;
            }
            else
            {
                flash_read(s_sbc_sotre_env.start_base + s_speaker_env.last_read_page_idx * FLASH_PAGE_SIZE + s_speaker_env.last_read_offset
                           ,r_len, s_sbc_buff + pos);
                pos += r_len;
                s_speaker_env.last_read_offset += r_len;
                r_len = 0;
            }
        }
        else if( s_speaker_env.last_read_page_idx == s_sbc_sotre_env.last_page_idx )
        {
            if( s_speaker_env.last_read_offset >= s_sbc_sotre_env.last_offset)
            {
                return 0;
            }
            else
            {
                if( (r_len + s_speaker_env.last_read_offset) > s_sbc_sotre_env.last_offset  )
                {
                    flash_read(s_sbc_sotre_env.start_base + s_speaker_env.last_read_page_idx * FLASH_PAGE_SIZE + s_speaker_env.last_read_offset
                               ,s_sbc_sotre_env.last_offset - s_speaker_env.last_read_offset, s_sbc_buff + pos);
                    uint32_t no_read_len = ( r_len + s_speaker_env.last_read_offset - s_sbc_sotre_env.last_offset );
                    pos += r_len;
                    s_speaker_env.last_read_offset = s_sbc_sotre_env.last_offset;
                    r_len = 0;
                    return (read_len - no_read_len);
                }
                else
                {
                    flash_read(s_sbc_sotre_env.start_base + s_speaker_env.last_read_page_idx * FLASH_PAGE_SIZE + s_speaker_env.last_read_offset
                               ,r_len, s_sbc_buff + pos);
                    pos += r_len;
                    s_speaker_env.last_read_offset += r_len;
                    r_len = 0;
                }
            }
        }
        else
            return 0;
    }

    return read_len;
}

static void ty_decoder_start(uint32_t start, uint32_t end, uint32_t tot_data_len, 
    uint16_t frame_len, uint32_t start_offset, uint8_t type)
{
    struct decoder_prepare_t param;

    param.data_start = start;
    param.data_end = end;
    param.store_type = type;
    param.tot_data_len = tot_data_len;
    param.start_offset = start_offset;
    param.frame_len = frame_len;
    TUYA_LOG_I(MOD_AUDIO,"s:%x,e:%x\r\n",start,end);

	os_event_t audio_event;

	audio_event.event_id = DECODER_EVENT_PREPARE;
	audio_event.param = &param;
	audio_event.param_len = sizeof(param);
    os_msg_post(s_audio_task_id,&audio_event);//TODO

    ty_speaker_start_hw();
    s_decoder_hold_flag = false;
}

static void ty_decoder_play_next_frame(void)
{
   

	os_event_t audio_event;

	audio_event.event_id = DECODER_EVENT_NEXT_FRAME;
	audio_event.param = NULL;
	audio_event.param_len = 0;
	if(false == s_decoder_hold_flag)
	{
		TUYA_LOG_I(MOD_AUDIO,"decoder_play_next_frame\r\n");
	}else{
		audio_event.event_id = DECODER_EVENT_STOP;
		TUYA_LOG_I(MOD_AUDIO,"decoder_play_next_frame have stopped\r\n");
	}
	os_msg_post(s_audio_task_id,&audio_event);
}

static void ty_decoder_stop(void)
{
    TUYA_LOG_I(MOD_AUDIO,"decoder_stop\r\n");


	os_event_t audio_event;

	audio_event.event_id = DECODER_EVENT_STOP;
	audio_event.param = NULL;
	audio_event.param_len = 0;
   os_msg_post(s_audio_task_id,&audio_event);
}


static void ty_end_speaker(void)
{
    TUYA_LOG_I(MOD_AUDIO,"ty_end_speaker\r\n");
	s_audio_core.stop = true;
	s_decoder_hold_flag = true;
    ty_decoder_stop();
    //ty_speaker_stop_hw();//TODO
}

static void ty_decoder_update_tot_data_len(uint32_t len)
{
    s_decoder_env.tot_data_len = len;
}

static void ty_decoder_half_processed(void)
{
    if(s_speaker_env.end_flag)
        goto _Exit;
    else
    {
        uint32_t pos = (s_speaker_env.store_data_len % s_speaker_env.sbc_data_tot_len);
        //fputc('z',0);
        uint32_t read_len = ty_read_sbc_from_flash(s_sbc_buff + pos,s_speaker_env.sbc_data_tot_len>>1);
        if(read_len > 0)
        {
            s_speaker_env.store_data_len += read_len;
            ty_decoder_update_tot_data_len( s_speaker_env.store_data_len );
        }
        else
            s_speaker_env.end_flag = 1;
    }
_Exit:
    ;
}

static void  ty_decoder_play_next_frame_handler(void *arg)
{
    uint8_t *buffer;
    uint32_t pcm_len;
    uint32_t streamlen;
    struct decoder_pcm_t *pcm_frame;
	uint8_t *Task_state;
	Task_state = arg;
    // CPU_SR cpu_sr;

    switch(*Task_state)
    {
        case DECODER_STATE_IDLE:
            break;
        case DECODER_STATE_BUFFERING:
        case DECODER_STATE_PLAYING:
#if 1

            if(s_decoder_env.store_type == DECODER_STORE_TYPE_RAM)
            {
                buffer = (uint8_t *)s_decoder_env.current_pos;
            }
            else
            {
                // TBD
            }

            #ifdef ADPCM_SEL_IMA
            pcm_frame = (struct decoder_pcm_t *)os_zalloc(sizeof(struct decoder_pcm_t) + 2*(s_decoder_env.frame_len+16)*2);
            #endif
            #ifdef ADPCM_SEL_MS
            pcm_frame = (struct decoder_pcm_t *)os_zalloc(sizeof(struct decoder_pcm_t) + 2*(s_decoder_env.frame_len+8)*2);
            #endif
            if(pcm_frame == NULL)
            {
               // return KE_MSG_SAVED;
            }

            pcm_len = 2*(s_decoder_env.frame_len+8)*2;
            streamlen = s_decoder_env.frame_len;
            TUYA_LOG_I(MOD_AUDIO,"playing:%d\r\n",streamlen);
            //printf("p:%d\r\n",streamlen);
            TUYA_LOG_I(MOD_AUDIO,"s_sbc_buff[0] = %02x, %02x.\r\n", buffer[0], buffer[1]);

            if(s_decoder_hold_flag == false)
            {
                #ifdef ADPCM_SEL_IMA
                adpcm_decode_frame(s_decoder_env.decoder_context, (short *)&pcm_frame->pcm_data[0], (int *)&pcm_len, buffer, s_decoder_env.frame_len);
                pcm_len = adpcm_decode_block ((void *)&pcm_frame->pcm_data[0], (const void *)buffer, s_decoder_env.frame_len, 1);
                pcm_len *= 2;
                #endif
                #ifdef ADPCM_SEL_MS
                adpcm_decode_frame(s_decoder_env.decoder_context, (short *)&pcm_frame->pcm_data[0], (int *)&pcm_len, buffer, s_decoder_env.frame_len);
                #endif
            }
            else
                memset((uint8_t *)&pcm_frame->pcm_data[0],0x0,pcm_len);

            if( false == s_decoder_hold_flag)
            {
                pcm_frame->pcm_size = pcm_len >> 1;
                pcm_frame->pcm_offset = 0;
                GLOBAL_INT_DISABLE();
                co_list_push_back(&s_decoder_env.pcm_buffer_list,&pcm_frame->list);
                GLOBAL_INT_RESTORE();
                TUYA_LOG_I(MOD_AUDIO,"pcmlen=%d,%d\r\n",pcm_len,streamlen);
                s_decoder_env.pcm_buffer_counter++;

                if((*Task_state )== DECODER_STATE_BUFFERING)
                {
					if (s_decoder_env.pcm_buffer_counter > 2)
                    {
                 		s_decodeTASKState = DECODER_STATE_PLAYING;
                        NVIC_EnableIRQ(I2S_IRQn);//
						
                    }
                    else
                    {
						ty_decoder_play_next_frame();
                    }
                }

                if(s_decoder_hold_flag == false)
                {
                    s_decoder_env.current_pos += s_decoder_env.frame_len;
                    s_decoder_env.data_processed_len += s_decoder_env.frame_len;
                    if( (s_decoder_env.tot_data_len - s_decoder_env.data_processed_len) < (1024) )
                    {
						ty_decoder_half_processed();
                    }

                    if( (s_decoder_env.tot_data_len - s_decoder_env.data_processed_len)  < s_decoder_env.frame_len )
                    {
                        if(s_decoder_hold_flag == false)
                        {                 
                        	s_decodeTASKState = DECODER_STATE_WAITING_END;                          
                        }
                    }
                    if(s_decoder_env.current_pos >= s_decoder_env.data_end)
                    {
                       s_decoder_env.current_pos = s_decoder_env.data_start;
                    }
                }

                if(s_decoder_env.store_type == DECODER_STORE_TYPE_FLASH)
                {
                    // TBD, free buffer
                }
            }
            else
            {
                os_free(pcm_frame);
                NVIC_EnableIRQ(I2S_IRQn);
                ty_decoder_stop();
            }
#endif
            break;
        case DECODER_STATE_WAITING_END:
            TUYA_LOG_I(MOD_AUDIO,"STATE_WAITING_END\r\n");
            //if(s_decoder_env.pcm_buffer_list.first == NULL)
            if(s_decoder_env.pcm_buffer_counter == 0)
            {
                if(s_stop_flag == 0)
                {
                    ty_decoder_stop();
                    s_stop_flag = 1;
                }
            }
            break;
        default:
            break;
    }

}

static void ty_decoder_end_func(void)
{
    TUYA_LOG_I(MOD_AUDIO,"audio play finished audioID==%d\r\n",s_audio_core.audio_id);
	#ifdef TY_AUDIO_BUFF_USE_HEAP
    if(s_sbc_buff!= NULL)
    {
        os_free(s_sbc_buff);
        s_sbc_buff = NULL;
    }
	#endif

	if(NULL != s_audio_core.user_cbk)
	{
	    s_audio_core.user_cbk(s_audio_core.user_param, TY_KEL_AUDIO_PLAY_FINISH);
	}
	if(true == s_audio_core.loop && false == s_audio_core.stop)
	{
	    ty_audio_play_loop();
	}else{
        memset(&s_audio_core,0,sizeof(s_audio_core));
	}
}

static int ty_speaker_from_flash(void)
{
    #ifdef TY_AUDIO_BUFF_USE_HEAP
    if( s_sbc_buff != NULL)
        goto _Exit;
    #endif
	
    TUYA_LOG_I(MOD_AUDIO,"speaker_flash_start\r\n");
    TUYA_LOG_I(MOD_AUDIO,"%x,%d,%d\r\n",s_sbc_sotre_env.start_base,s_sbc_sotre_env.last_page_idx,s_sbc_sotre_env.last_offset);
    if(s_sbc_sotre_env.start_base == 0xffffffff)//异常
    {
        memset((void *)&s_sbc_sotre_env,0x00,sizeof(s_sbc_sotre_env));
		s_Flash_data_state = false;//flash没有音频数据
        goto _Exit;
    }
	s_Flash_data_state = true;//flash中有音频数据
    ty_speaker_init();//speaker 初始化

    #ifdef TY_AUDIO_BUFF_USE_HEAP 
    if(NULL == s_sbc_buff) 
    {
        s_sbc_buff = (uint8_t *)os_zalloc(TY_AUDIO_BUFF_SIZE);//申请buffer 内存空间
    }
	#else
	if(NULL == s_sbc_buff) 
	{
	    s_sbc_buff = s_audio_buff;
	}
	#endif

	if(NULL == s_sbc_buff)
	{
	    TUYA_LOG_I(MOD_AUDIO,"ty_speaker_from_flash malloc failed\r\n");
		goto _Exit;
	}

    uint8_t *tmp_buf;
    memset(s_sbc_buff,0,TY_AUDIO_BUFF_SIZE);
    flash_read(s_sbc_sotre_env.start_base, 512, s_sbc_buff);//读取512个字节的数据

    memset((void *)&s_speaker_env, 0, sizeof(s_speaker_env));
    tmp_buf = s_sbc_buff;
    s_speaker_env.sbc_frame_len = ty_decoder_calc_adpcm_ms_frame_len(&tmp_buf);//获取sbc_frame_len
    //s_sbc_sotre_env.start_base += (tmp_buf - s_sbc_buff);
    s_speaker_env.last_read_offset = (tmp_buf - s_sbc_buff);
    s_speaker_env.last_read_page_idx = 0;
    if(s_speaker_env.sbc_data_tot_len == 0)
    {
        //sbc_frame_len 采样帧大小。该数值为:声道数×位数/8。播放软件需要一次处理多个该值大小的字节数据,用该数值调整缓冲区。
        //s_speaker_env.sbc_data_tot_len下面的计算设置s_speaker_env.sbc_data_tot_len为s_speaker_env.sbc_frame_len的倍数
    	s_speaker_env.sbc_data_tot_len = (TY_AUDIO_BUFF_SIZE - TY_AUDIO_BUFF_SIZE%s_speaker_env.sbc_frame_len)&(~0x1);
    }
    s_speaker_env.store_data_len += ty_read_sbc_from_flash(s_sbc_buff,s_speaker_env.sbc_data_tot_len>>1);

	TUYA_LOG_I(MOD_AUDIO,"sbc_frame_len==%d,last_read_offset==%d,sbc_data_tot_len==%d\r\n",
		s_speaker_env.sbc_frame_len,s_speaker_env.last_read_offset,s_speaker_env.sbc_data_tot_len);
	
    ty_decoder_start((uint32_t)s_sbc_buff, (uint32_t)s_sbc_buff + s_speaker_env.sbc_data_tot_len
                  , s_speaker_env.store_data_len, s_speaker_env.sbc_frame_len, 0, DECODER_STORE_TYPE_RAM);

    return 0;
_Exit:
    return -1;
}


int ty_audio_play_with_id(uint8_t id)
{
	ty_voice_header_t header_info = {0};
	ty_voice_single_t *audio_info = NULL;

	memset(&header_info,0,sizeof(header_info));
    flash_read(TY_AUDIO_FLASH_BASE_ADDR, sizeof(ty_voice_header_t), (uint8_t *)&header_info);//读取Flash存储的audio信息

	TUYA_LOG_I(MOD_AUDIO,"audio amount == %d\r\n",header_info.amount);
	TUYA_LOG_I(MOD_AUDIO,"audio total len == %x\r\n",header_info.total_len);

	if(header_info.amount <= 0)
	{
		TUYA_LOG_E(MOD_AUDIO,"not audio data in flash\r\n",header_info.amount);
		return -1;
	}

	audio_info = &header_info.single[id - 1];
	TUYA_LOG_I(MOD_AUDIO,"audio start address == %x,len==%d\r\n",audio_info->voice_addr,audio_info->voice_len);
	memset((void *)&s_sbc_sotre_env, 0, sizeof(s_sbc_sotre_env));
	s_sbc_sotre_env.start_base = audio_info->voice_addr;
	s_sbc_sotre_env.last_page_idx = audio_info->voice_len/FLASH_PAGE_SIZE;
	s_sbc_sotre_env.last_offset = audio_info->voice_len%FLASH_PAGE_SIZE;
	//s_sbc_sotre_env.tot_data_len = audio_info->voice_len;
	
	return ty_speaker_from_flash();
}

/*************************************** speaker { *****************************************/

static volatile struct i2s_reg_t *i2s_reg = (struct i2s_reg_t *)I2S_BASE;

static void ty_speaker_start_hw(void)
{
	pmu_codec_power_enable();               
	i2s_start();                            
    codec_enable_dac();                     
	TUYA_LOG_I(MOD_AUDIO,"speaker_start_hw\r\n");
	TY_PA_ENABLE;								
}

static void ty_speaker_stop_hw(void)
{
	pmu_codec_power_disable();    
	TY_PA_DISABLE;						
	codec_disable_dac();				
    i2s_stop();							
	TUYA_LOG_I(MOD_AUDIO,"speaker_stop_hw\r\n");
}

static void ty_speaker_init(void)
{
	pmu_codec_power_enable();
	audio_speaker_codec_init();	

	i2s_init(I2S_DIR_TX,8000,1);	
	NVIC_SetPriority(I2S_IRQn, 2);	//Setting the I2S interrupt priority
	TUYA_LOG_I(MOD_AUDIO,"speaker_init\r\n");
	//PA_init_pins();					//Initialize PA enable pin
}

__attribute__((section("ram_code"))) void i2s_isr_ram(void)
{
	uint8_t i;
	//M:xiaojian 
	if(0){
    
	}else{
		uint32_t last = 0;
	    if((i2s_reg->status.tx_half_empty)&&(i2s_reg->mask.tx_half_empty))//codec_DAC
	    {
	        
#define I2S_FIFO_DEPTH      64
	        struct co_list_hdr *element;
	        struct decoder_pcm_t *pcm;
	        uint16_t *tx_data;
		        if(co_list_is_empty(&s_decoder_env.pcm_buffer_list))
		        {
		            TUYA_LOG_I(MOD_AUDIO,"F\r\n");
		            for(i=0; i<(I2S_FIFO_DEPTH/2); i++)
		            {
						i2s_reg->data = 0;

		            }
		        }
		        else
		        {
		            element = s_decoder_env.pcm_buffer_list.first;

		            pcm = (struct decoder_pcm_t *)element;
		            tx_data = (uint16_t *)&pcm->pcm_data[pcm->pcm_offset];
		            last = pcm->pcm_size - pcm->pcm_offset;
		            if(last > (I2S_FIFO_DEPTH/2))
		            {
		              //  co_printf("X");
		                for (i=0; i<(I2S_FIFO_DEPTH/2); i++)
		                {
		                    uint32_t tmp_data = *tx_data++;
		                    tmp_data &= 0xFFFF;

							i2s_reg->data = tmp_data;
		                }

		                pcm->pcm_offset += (I2S_FIFO_DEPTH/2);
		            }
		            else
		            {
		               
		                for (i=0; i<last; i++)
		                {
		                    uint32_t tmp_data = *tx_data++;
		                    tmp_data &= 0xFFFF;
							i2s_reg->data = tmp_data;
		                }
		                co_list_pop_front(&s_decoder_env.pcm_buffer_list);
		                os_free((void *)pcm);
		                s_decoder_env.pcm_buffer_counter--;
		                ty_decoder_play_next_frame();

		                while(!co_list_is_empty(&s_decoder_env.pcm_buffer_list))
		                {
		                    element = s_decoder_env.pcm_buffer_list.first;

		                    pcm = (struct decoder_pcm_t *)element;
		                    tx_data = (uint16_t *)&pcm->pcm_data[0];
		                    last = pcm->pcm_size - pcm->pcm_offset;
		                    if((last + i) > (I2S_FIFO_DEPTH/2))
		                    {
		                        pcm->pcm_offset = (I2S_FIFO_DEPTH/2) - i;
		                       for(; i<(I2S_FIFO_DEPTH/2); i++)
		                        {
		                            uint32_t tmp_data = *tx_data++;
		                            tmp_data &= 0xFFFF;                      
									i2s_reg->data = tmp_data;
		                        }
		                        break;
		                    }
		                    else
		                    {
		                        last += i;
		                        for(; i<last; i++)
		                        {
		                            uint32_t tmp_data = *tx_data++;
		                            tmp_data &= 0xFFFF;
		                            //REG_PL_WR(I2S_REG_DATA, tmp_data);
									i2s_reg->data = tmp_data;
		                        }
		                        co_list_pop_front(&s_decoder_env.pcm_buffer_list);
		                        os_free((void *)pcm);
		                        s_decoder_env.pcm_buffer_counter--;
		                        ty_decoder_play_next_frame();
		                    }
		                }

		            }
		        }
			
	    }
	}
}


/*************************************** speaker } *****************************************/

static int ty_audio_play_loop(void)
{
	os_event_t audio_event = {0};

    memset(&audio_event,0,sizeof(audio_event));
	audio_event.event_id = DECODER_EVENT_LOOP;
    os_msg_post(s_audio_task_id,&audio_event);
    return 0;
}


static int ty_audio_task_func(os_event_t *param)
{
	struct decoder_prepare_t *decoder_param = NULL;
	ADPCMContext *context = NULL;
	switch (param->event_id)
	{
	    case DECODER_EVENT_START:
			do
			{
				ty_audio_core_t *audio_info = (ty_audio_core_t *)(param->param);

				if(TY_AUDIO_STATUS_IDLE != s_audio_core.status)
				{
				    TUYA_LOG_I(MOD_AUDIO,"[warring]audio is busy!\r\n");
					if(audio_info->user_cbk != NULL)
					{
					    audio_info->user_cbk(audio_info->user_param,TY_KEL_AUDIO_PLAY_ERROR);
					}
					break;
				}

				memset(&s_audio_core,0,sizeof(s_audio_core));
				s_audio_core.status = TY_AUDIO_STATUS_BUSY;
				s_audio_core.user_cbk = audio_info->user_cbk;
				s_audio_core.user_param = audio_info->user_param;
				s_audio_core.audio_id = audio_info->audio_id;
				s_audio_core.loop = audio_info->loop;
			    TUYA_LOG_I(MOD_AUDIO,"audio ID==%d",s_audio_core.audio_id);
				if(0 != ty_audio_play_with_id(s_audio_core.audio_id))
				{
				    TUYA_LOG_I(MOD_AUDIO,"audio play start failed!\r\n");
					if(audio_info->user_cbk != NULL)
					{
					    audio_info->user_cbk(audio_info->user_param,TY_KEL_AUDIO_PLAY_ERROR);
					}
				}
			}while(0);
			break;
		
		case DECODER_EVENT_LOOP:
			
			if(0 != ty_audio_play_with_id(s_audio_core.audio_id))
			{
			    TUYA_LOG_I(MOD_AUDIO,"audio play start failed!\r\n");
				if(s_audio_core.user_cbk != NULL)
				{
				    s_audio_core.user_cbk(s_audio_core.user_param,TY_KEL_AUDIO_PLAY_ERROR);
				}
			}
			break;
		case DECODER_EVENT_PREPARE:		

		    //ty_speaker_init();//speaker 初始化
		    //ty_speaker_start_hw();
			
			decoder_param = (struct decoder_prepare_t *)(param->param);
			s_decoder_env.decoder_context = os_zalloc(sizeof(ADPCMContext));
            context = (ADPCMContext *)s_decoder_env.decoder_context;
            context->channel = 1;
            context->block_align = decoder_param->frame_len;
            s_decoder_env.data_start = decoder_param->data_start;
            s_decoder_env.data_end = decoder_param->data_end;
            s_decoder_env.current_pos = decoder_param->data_start + decoder_param->start_offset;
            s_decoder_env.tot_data_len = decoder_param->tot_data_len;
            s_decoder_env.store_type = decoder_param->store_type;
            s_decoder_env.data_processed_len = 0;
            s_decoder_env.frame_len = decoder_param->frame_len;
            s_stop_flag = 0;

            TUYA_LOG_I(MOD_AUDIO,"preparing,fram_len:%d\r\n",s_decoder_env.frame_len);
            co_list_init(&s_decoder_env.pcm_buffer_list);
            s_decoder_env.pcm_buffer_counter = 0;
            ty_decoder_play_next_frame();
			s_decodeTASKState = DECODER_STATE_BUFFERING;
			
			break;
		//音频解码下一帧数据处理
		 case DECODER_EVENT_NEXT_FRAME:	
		 	//co_printf("DECODER_EVENT_NEXT_FRAME\r\n");
			ty_decoder_play_next_frame_handler(&s_decodeTASKState);
		 	break;
		 
		 //音频解码停止
		case DECODER_EVENT_STOP:
			TUYA_LOG_I(MOD_AUDIO,"DECODER_EVENT_STOP\r\n");
		    NVIC_DisableIRQ(I2S_IRQn);

		    while(1)
		    {
		        struct co_list_hdr *element = co_list_pop_front(&s_decoder_env.pcm_buffer_list);
		        if(element == NULL)
		            break;
		        os_free((void *)element);
		    }

		    if(s_decoder_env.decoder_context != NULL)
		    {
		        os_free((void *)s_decoder_env.decoder_context);
		        s_decoder_env.decoder_context = NULL;
		    }
			s_decodeTASKState = DECODER_STATE_IDLE;
			s_decoder_hold_flag = false;
		    ty_speaker_stop_hw();
		    ty_decoder_end_func();
			TUYA_LOG_I(MOD_AUDIO,"system heap free:%u", os_get_free_heap_size());
			break;
	}
    return EVT_CONSUMED;
}

static ty_audio_core_t s_audio_temp_buff = {0};
static void ty_play_start_delay_timeout_cb(void* param)
{
    os_event_t audio_event = {0};

	audio_event.event_id = DECODER_EVENT_START;
	audio_event.param = &s_audio_temp_buff;
	audio_event.param_len = sizeof(s_audio_temp_buff);
    os_msg_post(s_audio_task_id,&audio_event);
}


TUYA_RET_E tuya_kel_audio_play_start(char* file_path,TUYA_KEL_AUDIO_FORMAT_E format, tuya_kel_audio_play_cb cbk, uint8_t loop,void *user_param)
{
#if 0
    ty_audio_core_t audio_info = {0};
	os_event_t audio_event = {0};


	audio_info.user_cbk = cbk;
	audio_info.user_param = user_param;
	audio_info.audio_id = atoi(file_path);
	audio_info.loop = loop;

	

	audio_event.event_id = DECODER_EVENT_START;
	audio_event.param = &audio_info;
	audio_event.param_len = sizeof(audio_info);
    os_msg_post(s_audio_task_id,&audio_event);
    return 0;
#else
	//TODO
    s_audio_temp_buff.user_cbk = cbk;
	s_audio_temp_buff.user_param = user_param;
	s_audio_temp_buff.audio_id = atoi(file_path);
	s_audio_temp_buff.loop = loop;
	
    os_timer_stop(&s_start_delay_timer);
    os_timer_start(&s_start_delay_timer, 100, false);
	return TUYA_OK;
#endif
}

TUYA_RET_E tuya_kel_audio_play_stop(void)
{
    TUYA_LOG_I(MOD_AUDIO,"ty_audio_play_stop\r\n");
    ty_end_speaker();
	return TUYA_OK;
}

void tuya_kel_audio_init(void)
{
    //音频功放使能控制
    TUYA_GPIO_INIT(EBIKE_PA_EN,GPIO_DIR_OUT,TUYA_PIN_LOW); 
	s_audio_task_id = os_task_create(ty_audio_task_func);//创建音频任务
	memset(&s_audio_core,0,sizeof(0));

	os_timer_init(&s_start_delay_timer, ty_play_start_delay_timeout_cb, NULL);
}


