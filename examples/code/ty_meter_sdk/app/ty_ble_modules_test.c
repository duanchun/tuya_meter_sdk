
#include "tuya_ble_log.h"

void tuya_ble_modules_test_main(uint8_t *param)
{
    uint8_t cmd = *param;

	switch(cmd)
	{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
			ty_audio_play_with_id(cmd - '0');
			break;
		case 'a':
			ty_audio_play_stop();
	        ty_audio_play_start("3",0,NULL,true,NULL);
			break;
	    case 'b':
			locdef_dev_audio_play(1,1,5*1000);
			break;
	    case 'c':
			TY_PRINTF("MS==%u\r\n",system_get_curr_time());
			break;
	    case 'd':
			ty_audio_play_stop();
	        ty_audio_play_start("2",0,NULL,false,NULL);
			break;
		default:
			ty_audio_play_stop();
			break;
	}
}


