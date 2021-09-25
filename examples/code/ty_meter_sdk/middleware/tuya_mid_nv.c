
#include "tuya_mid_nv.h"
#include "tuya_log.h"
#include "tuya_kel_flash.h"

#define TUYA_INVALID_NV_OFFSET    0XFFFFFFFF

typedef struct {
	uint8_t nv_item_lid;
	uint8_t total;
	uint16_t size;
	const char *verno;
	const char *item_name;
}tuya_core_nv_item_t;

static tuya_core_nv_item_t s_tuya_core_nv_items[] = {
	{
		TUYA_NVRAM_DEV_CONFIG_LID,
		TUYA_NVRAM_DEV_CONFIG_LID_TOTAL,
		TUYA_NVRAM_DEV_CONFIG_LID_SIZE,
		TUYA_NVRAM_DEV_CONFIG_LID_VERNO,
		"dev_config"
	},
	{
		TUYA_NVRAM_RCU433_LID,
		TUYA_NVRAM_RCU433_LID_TOTAL,
		TUYA_NVRAM_RCU433_LID_SIZE,
		TUYA_NVRAM_RCU433_LID_VERNO,
		"rcu_addr"
	}
};

static uint32_t tuya_core_all_nv_size_get(void)
{
	int nv_max = TUYA_NVRAM_MAX_LID;
	int index = 0;
	uint32_t nv_size = 0;

	for(index = 0;index < nv_max;index++)
	{
		nv_size += s_tuya_core_nv_items[index].size*s_tuya_core_nv_items[index].total;
	}
	TUYA_LOG_I(MOD_NV,"All NV size==%u",nv_size);
	return nv_size;
}

static uint32_t tuya_core_nv_item_offet_get(TUYA_NVRAM_LID_E nLID, uint16_t nRecordId)
{
	int nv_max = TUYA_NVRAM_MAX_LID;
	int index = 0;
	uint32_t offset = 0;

	for(index = 0;index < nv_max;index++)
	{
		if(nLID == s_tuya_core_nv_items[index].nv_item_lid)
		{
			offset += s_tuya_core_nv_items[index].size*(nRecordId -1);
			break;
		}
		offset += s_tuya_core_nv_items[index].size*s_tuya_core_nv_items[index].total;
	}

	if(index == nv_max)
	{
		TUYA_LOG_E(MOD_NV,"invalid nLID input");
		return TUYA_INVALID_NV_OFFSET;
	}
	TUYA_LOG_I(MOD_NV,"offset==%u",offset);
	return offset;
}

static void tuya_core_nv_init(void)
{
	int index = 0;
	char *temp_buf = NULL;
	int ret = 0;
	int nv_max = TUYA_NVRAM_MAX_LID;

	for(index = 0;index < nv_max;index++)
	{
		TUYA_LOG_I(MOD_NV,"NV itme name==%s",s_tuya_core_nv_items[index].item_name);
	}

	TUYA_LOG_I(MOD_NV,"NV ram init Successed");
}

static tuya_core_nv_item_t * tuya_core_nv_get_item_info_with_lid(TUYA_NVRAM_LID_E nLID)
{
	int index = 0;
	int nv_max = TUYA_NVRAM_MAX_LID;

	for(index = 0;index < nv_max;index++)
	{
		if(s_tuya_core_nv_items[index].nv_item_lid == nLID)
		{
			return &s_tuya_core_nv_items[index];
		}
	}
	TUYA_LOG_W(MOD_NV,"map nv failed");
	return NULL;
	
}

TUYA_RET_E tuya_mid_nv_read(TUYA_NVRAM_LID_E nLID, uint16_t nRecordId, void *pBuffer, uint16_t nBufferSize, int *pError)
{
    TUYA_RET_E ret = TUYA_OK;
	tuya_core_nv_item_t *pItem= NULL;

	pItem = tuya_core_nv_get_item_info_with_lid(nLID);

	if(NULL == pItem)
	{
		TUYA_LOG_W(MOD_NV,"Invalid nv lid");
		return TUYA_INVALID_PARAMS;
	}

	if(nBufferSize < pItem->size)
	{
		TUYA_LOG_W(MOD_NV,"Buff size not enought");
		return TUYA_INVALID_PARAMS;
	}

	ret = tuya_kel_flash_read(TUYA_NVRAM_FLASH_START_ADDR,pBuffer,nBufferSize);

	if(TUYA_OK != ret)
	{
		TUYA_LOG_E(MOD_NV,"read failed");
		return TUYA_COMMON_ERROR;
	}

    return ret;
}


TUYA_RET_E tuya_mid_nv_write(TUYA_NVRAM_LID_E nLID, uint16_t nRecordId, void *pBuffer, uint16_t nBufferSize, int *pError)
{
    TUYA_RET_E ret = TUYA_OK;
	tuya_core_nv_item_t *pItem= NULL;
	uint8_t* temp = NULL;
	uint32_t nv_size = 0;
	uint32_t offset = 0;

	pItem = tuya_core_nv_get_item_info_with_lid(nLID);

	if(NULL == pItem)
	{
		TUYA_LOG_W(MOD_NV,"Invalid nv lid");
		return TUYA_INVALID_PARAMS;
	}

	if(nBufferSize > pItem->size)
	{
		TUYA_LOG_W(MOD_NV,"Invalid nv nv size");
		return TUYA_INVALID_PARAMS;
	}

	nv_size = tuya_core_all_nv_size_get();
	temp = (uint8_t*)tuya_kel_malloc(nv_size);
	if(NULL == temp)
	{
		TUYA_LOG_E(MOD_NV,"malloc failed,size==%u",nv_size);
		return TUYA_COMMON_ERROR;
	}

	ret = tuya_kel_flash_read(TUYA_NVRAM_FLASH_START_ADDR,(void*)temp,nv_size);

	if(TUYA_OK != ret)
	{
		TUYA_LOG_E(MOD_NV,"read failed");
		goto END;
	}

	offset = tuya_core_nv_item_offet_get(nLID,nRecordId);
	if(TUYA_INVALID_NV_OFFSET == offset)
	{
		goto END;
	}

	ret = ty_kel_flash_erase(TUYA_NVRAM_FLASH_START_ADDR,1);
	if(TUYA_OK != ret)
	{
		TUYA_LOG_E(MOD_NV,"erash flash failed");
		goto END;
	}

	memcpy(temp + offset,pBuffer,nBufferSize);

	ret = tuya_kel_flash_write(TUYA_NVRAM_FLASH_START_ADDR,temp,nv_size);
	if(TUYA_OK != ret)
	{
		TUYA_LOG_E(MOD_NV,"write flash failed");
		goto END;
	}
	
END:
	tuya_kel_free(temp);
    return ret;
}


void tuya_mid_nv_init(void)
{
	tuya_core_nv_init();
}



