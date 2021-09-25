#include "tuya_ble_ota.h"
#include "tuya_ble_log.h"
#include "tuya_ble_api.h"
#include "tuya_ble_utils.h"
#include "ty_ble.h"
#include "ty_flash.h"
#include "driver_flash.h"
#include "jump_table.h"
#include "flash_usage_config.h"
#include "ty_ble_config.h"




/*********************************************************************
 * LOCAL CONSTANTS
 */
#define TUYA_BLE_OTA_STATE_UNKNOWN   (-1)

#define TUYA_BLE_OTA_VERSION         (3)

#define TUYA_BLE_OTA_PKG_LEN         (256)

#define TUYA_BLE_OTA_FILE_MD5_LEN    (16)

#define TUYA_BLE_OTA_START_ADDR       ty_ota_start_addr
#define TUYA_BLE_OTA_END_ADDR         ty_ota_end_addr
#define TUYA_BLE_OTA_FILE_MAX_LEN     ty_ot_file_max_len
#define TUYA_BLE_OTA_INFO_ADDR        BOARD_FLASH_OTA_INFO_ADDR

/*********************************************************************
 * LOCAL STRUCT
 */
#pragma pack(1)
typedef struct{
	uint8_t  flag;
	uint8_t  ota_version;
    uint8_t  type;
    uint32_t version;
    uint16_t package_maxlen;
} tuya_ble_ota_req_rsp_t;

typedef struct{
	uint8_t  type;
	uint8_t  pid[8];
    uint32_t version;
    uint8_t  md5[TUYA_BLE_OTA_FILE_MD5_LEN];
    uint32_t file_len;
    uint32_t crc32;
} tuya_ble_ota_file_info_t;

typedef struct{
	uint8_t  type;
	uint8_t  state;
    uint32_t old_file_len;
    uint32_t old_crc32;
    uint8_t  old_md5[TUYA_BLE_OTA_FILE_MD5_LEN];
} tuya_ble_ota_file_info_rsp_t;

typedef struct{
	uint8_t  type;
    uint32_t offset;
} tuya_ble_ota_file_offset_t;

typedef struct{
	uint8_t  type;
    uint32_t offset;
} tuya_ble_ota_file_offset_rsp_t;

typedef struct{
	uint8_t  type;
    uint16_t pkg_id;
    uint16_t len;
    uint16_t crc16;
    uint8_t  data[];
} tuya_ble_app_ota_data_t;

typedef struct{
	uint8_t type;
    uint8_t state;
} tuya_ble_ota_data_rsp_t;

typedef struct{
	uint8_t  type;
    uint8_t state;
} tuya_ble_ota_end_rsp_t;

typedef struct{
	uint32_t len;
    uint32_t crc32;
    uint8_t  md5[TUYA_BLE_OTA_FILE_MD5_LEN];
    uint8_t  first_page[256];
} tuya_ble_ota_file_info_storage_t;
#pragma pack()

/*********************************************************************
 * LOCAL VARIABLES
 */
static volatile int8_t  s_ota_state     = TUYA_BLE_OTA_STATE_UNKNOWN;
static volatile int32_t s_pkg_id;
static volatile bool    s_ota_success;
static volatile uint8_t ota_state = 0;

static uint32_t         s_data_len;
static uint32_t         s_data_crc;

static uint32_t         s_file_len;
static uint32_t         s_file_crc;
static uint8_t          s_file_md5[TUYA_BLE_OTA_FILE_MD5_LEN];

//file info
static tuya_ble_ota_file_info_storage_t s_old_file;

/*********************************************************************
 * LOCAL FUNCTION
 */
extern void enable_cache(uint8_t invalid_ram);
extern void disable_cache(void);

/*********************************************************************
 * VARIABLES
 */
uint32_t ty_ota_start_addr = 0;
uint32_t ty_ota_end_addr = 0;
uint32_t ty_ot_file_max_len = 0;




/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_get_curr_firmwave_version(void)
{
    struct jump_table_t *jump_table_a = (void*)0x01000000;
    struct jump_table_t *jump_table_b = (void*)(0x01000000 + jump_table_a->image_size);
    
    if(system_regs->remap_length != 0) { //partB
        return jump_table_b->firmware_version;
    } else {
        return jump_table_a->firmware_version;
    }
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_get_storage_address(void)
{
    struct jump_table_t* jump_table_tmp = (void*)0x01000000;
    
    if(system_regs->remap_length != 0) { //partB, then return partA flash Addr
        return 0;
    } else {
        return jump_table_tmp->image_size;
    }
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_get_image_size(void)
{
    struct jump_table_t *jump_table_tmp = (void*)0x01000000;
    return jump_table_tmp->image_size;
}

/*********************************************************
FN: 
*/
__attribute__((section("ram_code"))) uint8_t app_get_ota_state(void)
{
    return ota_state;
}

/*********************************************************
FN: 
*/
__attribute__((section("ram_code"))) void app_set_ota_state(uint8_t state_flag)
{
    if(state_flag) {
        ota_state = 1;
    } else {
        ota_state = 0;
    }
}

/*********************************************************
FN: 
*/
/*********************************ota_TuYa*********************************************/
__attribute__((section("ram_code")))  uint8_t tuya_ble_ota_save_data(uint32_t dest, uint8_t *src, uint32_t len)
{
    uint32_t current_remap_address, remap_size;
    current_remap_address = system_regs->remap_virtual_addr;
    remap_size = system_regs->remap_length;

    GLOBAL_INT_DISABLE();

    system_regs->remap_virtual_addr = 0;
    system_regs->remap_length = 0;

    app_set_ota_state(1);
    flash_write(dest, len, src);
    app_set_ota_state(0);

    system_regs->remap_virtual_addr = current_remap_address;
    system_regs->remap_length = remap_size;

    GLOBAL_INT_RESTORE();
    
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_info_save(void)
{
    tuya_ble_nv_erase(TUYA_BLE_OTA_INFO_ADDR, 0x1000);
    tuya_ble_nv_write(TUYA_BLE_OTA_INFO_ADDR, (void*)&s_old_file, sizeof(tuya_ble_ota_file_info_storage_t));
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_info_load(void)
{
    tuya_ble_nv_read(TUYA_BLE_OTA_INFO_ADDR, (void*)&s_old_file, sizeof(tuya_ble_ota_file_info_storage_t));
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_flash_page_erase(uint32_t addr)
{
#ifdef FLASH_PROTECT
    flash_protect_disable(0);
#endif
    
    flash_page_erase(addr);
    
#ifdef FLASH_PROTECT
    flash_protect_enable(0);
#endif
    
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_enter(void)
{
    s_pkg_id = -1;
    s_ota_success = false;
    
    s_data_len = 0;
    s_data_crc = 0;
    
    s_file_len = 0;
    s_file_crc = 0;
    memset(s_file_md5, 0, TUYA_BLE_OTA_FILE_MD5_LEN);
    
    ty_ble_set_conn_param(15, 15, 0, 6000);
    ty_ble_set_dle();
    gatt_mtu_exchange_req(0);
    
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_exit(void)
{
    if(!s_ota_success) {
        //#1
    }
    
    s_ota_state = TUYA_BLE_OTA_STATE_UNKNOWN;
    tuya_ble_disconnect_and_reset_timer_start(); //TODO
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_get_crc32_in_flash(uint32_t len)   
{
    static uint8_t buf[TUYA_BLE_OTA_PKG_LEN];
    
    if(len == 0) {
        return 0;
    }
    
    uint32_t crc_temp = 0;
    uint32_t read_addr = TUYA_BLE_OTA_START_ADDR;
    uint32_t cnt = len/TUYA_BLE_OTA_PKG_LEN;
    uint32_t remainder = len%TUYA_BLE_OTA_PKG_LEN;
    
    for(uint32_t idx=0; idx<cnt; idx++) {
        uint32_t current_remap_address, remap_size;
        current_remap_address = system_regs->remap_virtual_addr;
        remap_size = system_regs->remap_length;
        
        if(idx == 0) {
            crc_temp =  tuya_ble_crc32_compute(s_old_file.first_page, TUYA_BLE_OTA_PKG_LEN, &crc_temp);
        } else {
            GLOBAL_INT_DISABLE();
            system_regs->remap_virtual_addr = 0;
            system_regs->remap_length = 0;
            disable_cache();
            tuya_ble_nv_read(read_addr, buf, TUYA_BLE_OTA_PKG_LEN);
            enable_cache(true);
            system_regs->remap_virtual_addr = current_remap_address;
            system_regs->remap_length = remap_size;
            GLOBAL_INT_RESTORE();
            crc_temp = tuya_ble_crc32_compute(buf, TUYA_BLE_OTA_PKG_LEN, &crc_temp);   
        }
        
        read_addr += TUYA_BLE_OTA_PKG_LEN;
    }

    if(remainder > 0) {
        uint32_t current_remap_address, remap_size;
        current_remap_address = system_regs->remap_virtual_addr;
        remap_size = system_regs->remap_length;
        GLOBAL_INT_DISABLE();
        system_regs->remap_virtual_addr = 0;
        system_regs->remap_length = 0;
        disable_cache();
        tuya_ble_nv_read(read_addr, buf, TUYA_BLE_OTA_PKG_LEN);
        enable_cache(true);
        system_regs->remap_virtual_addr = current_remap_address;
        system_regs->remap_length = remap_size;
        GLOBAL_INT_RESTORE();
        crc_temp = tuya_ble_crc32_compute(buf, remainder, &crc_temp);
        read_addr += remainder;
    }
    
    return crc_temp;
}
/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_rsp(tuya_ble_ota_response_t* rsp, void* rsp_data, uint16_t data_size)
{
    if(rsp->type != TUYA_BLE_OTA_DATA) {
        TUYA_APP_LOG_HEXDUMP_INFO("ota_rsp_data", rsp_data, data_size);
    }
    
    rsp->p_data = rsp_data;
    rsp->data_len = data_size;
    return tuya_ble_ota_response(rsp);
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_req_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_STATE_UNKNOWN) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_REQ- s_ota_state error");
        //rsp
        tuya_ble_ota_req_rsp_t req_rsp;
        memset(&req_rsp, 0x00, sizeof(tuya_ble_ota_req_rsp_t));
        req_rsp.flag = 0x01; //refuse ota
        
        tuya_ble_ota_rsp(rsp, &req_rsp, sizeof(tuya_ble_ota_req_rsp_t));
        tuya_ble_ota_exit();
        return 1;
    }
    
    s_ota_state = TUYA_BLE_OTA_REQ;
    
    //param check
    if((cmd_size != 0x0001) || (*cmd != 0x00)) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_REQ- param error");
        //rsp
        tuya_ble_ota_req_rsp_t req_rsp;
        memset(&req_rsp, 0x00, sizeof(tuya_ble_ota_req_rsp_t));
        req_rsp.flag = 0x01; //refuse ota
        
        tuya_ble_ota_rsp(rsp, &req_rsp, sizeof(tuya_ble_ota_req_rsp_t));
        tuya_ble_ota_exit();
        return 1;
    }
    
    {
        tuya_ble_ota_enter();
        ty_ota_start_addr  = tuya_ble_ota_get_storage_address();
        ty_ot_file_max_len = tuya_ble_ota_get_image_size();
        ty_ota_end_addr =   ty_ota_start_addr+ ty_ot_file_max_len;
        

        //rsp
        tuya_ble_ota_req_rsp_t req_rsp;
        memset(&req_rsp, 0x00, sizeof(tuya_ble_ota_req_rsp_t));
        req_rsp.flag = 0x00; //accept ota
        req_rsp.ota_version = TUYA_BLE_OTA_VERSION;
        req_rsp.type = 0x00; //firmware info
        
        req_rsp.version = TY_DEVICE_FVER_NUM;
        tuya_ble_inverted_array((void*)&req_rsp.version, sizeof(uint32_t));
        
        req_rsp.package_maxlen = TUYA_BLE_OTA_PKG_LEN;
        tuya_ble_inverted_array((void*)&req_rsp.package_maxlen, sizeof(uint16_t));
        
        tuya_ble_ota_rsp(rsp, &req_rsp, sizeof(tuya_ble_ota_req_rsp_t));
        s_ota_state = TUYA_BLE_OTA_FILE_INFO;
    }
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_file_info_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_FILE_INFO) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_FILE_INFO- s_ota_state error");
        //rsp none
        tuya_ble_ota_exit();
        return 1;
    }

    //param check
    tuya_ble_ota_file_info_t* file_info = (void*)cmd;
    if(file_info->type != 0x00) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_FILE_INFO- file_info->type error");
        //rsp none
        tuya_ble_ota_exit();
        return 1;
    }
    
    {
        //file info
        tuya_ble_inverted_array((void*)&file_info->version, sizeof(uint32_t));
        tuya_ble_inverted_array((void*)&file_info->file_len, sizeof(uint32_t));
        tuya_ble_inverted_array((void*)&file_info->crc32, sizeof(uint32_t));
        
        s_file_len = file_info->file_len;
        s_file_crc = file_info->crc32;
        memcpy(s_file_md5, file_info->md5, TUYA_BLE_OTA_FILE_MD5_LEN);
        
        //rsp
        tuya_ble_ota_file_info_rsp_t file_info_rsp;
        memset(&file_info_rsp, 0x00, sizeof(tuya_ble_ota_file_info_rsp_t));
        file_info_rsp.type = 0x00; //firmware info
        if(file_info->version <= TY_DEVICE_FVER_NUM) {
            file_info_rsp.state = 0x02; //version error
        }
        else if(file_info->file_len > TUYA_BLE_OTA_FILE_MAX_LEN) {
            TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_FILE_MAX_LEN: %d,file_len: %d ", TUYA_BLE_OTA_FILE_MAX_LEN,file_info->file_len);
            file_info_rsp.state = 0x03; //size error
        } else {
            file_info_rsp.state = 0x00;
            s_ota_state = TUYA_BLE_OTA_FILE_OFFSET_REQ;
        }
        
        file_info_rsp.old_file_len = s_old_file.len;
        tuya_ble_inverted_array((void*)&file_info_rsp.old_file_len, sizeof(uint32_t));
        
        file_info_rsp.old_crc32 = s_old_file.crc32;
        tuya_ble_inverted_array((void*)&file_info_rsp.old_crc32, sizeof(uint32_t));
        
        memset(file_info_rsp.old_md5, 0x00, TUYA_BLE_OTA_FILE_MD5_LEN);
        tuya_ble_ota_rsp(rsp, &file_info_rsp, sizeof(tuya_ble_ota_file_info_rsp_t));
        
        if(file_info_rsp.state != 0x00) {
            TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_FILE_INFO- errorid: %d", file_info_rsp.state);
            tuya_ble_ota_exit();
        }
    }
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_file_offset_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_FILE_OFFSET_REQ) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_FILE_OFFSET_REQ- s_ota_state error");
        //rsp none
        tuya_ble_ota_exit();
        return 1;
    }

    //param check
    tuya_ble_ota_file_offset_t* file_offset = (void*)cmd;
    if(file_offset->type != 0x00) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_FILE_OFFSET_REQ- file_offset->type error");
        //rsp none
        tuya_ble_ota_exit();
        return 1;
    }
    
    {
        tuya_ble_inverted_array((void*)&file_offset->offset, sizeof(uint32_t));
        
        //rsp
        tuya_ble_ota_file_offset_rsp_t file_offset_rsp;
        memset(&file_offset_rsp, 0x00, sizeof(tuya_ble_ota_file_offset_rsp_t));
        file_offset_rsp.type = 0x00;
        {
            if(file_offset->offset > 0)
            {
                if((memcmp(s_old_file.md5, s_file_md5, TUYA_BLE_OTA_FILE_MD5_LEN) == 0) 
                            && (tuya_ble_ota_get_crc32_in_flash(s_old_file.len) == s_old_file.crc32) 
                            && (file_offset->offset >= s_old_file.len)) {
                    file_offset_rsp.offset = s_old_file.len;
                    s_data_len = s_old_file.len;
                    s_data_crc = s_old_file.crc32;
                } else {
                    file_offset_rsp.offset = 0;
                    s_data_len = 0;
                    s_data_crc = 0;
                }
            }
            
            memcpy(s_old_file.md5, s_file_md5, TUYA_BLE_OTA_FILE_MD5_LEN);
            tuya_ble_ota_info_save();
        }
        
        tuya_ble_inverted_array((void*)&file_offset_rsp.offset, sizeof(uint32_t));
        
        tuya_ble_ota_rsp(rsp, &file_offset_rsp, sizeof(tuya_ble_ota_file_offset_rsp_t));
        s_ota_state = TUYA_BLE_OTA_DATA;
    }
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_data_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    uint8_t first_flag = 0;
    //param check
    if(s_ota_state != TUYA_BLE_OTA_DATA) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_DATA- s_ota_state error");
        //rsp
        tuya_ble_ota_data_rsp_t ota_data_rsp;
        memset(&ota_data_rsp, 0x00, sizeof(tuya_ble_ota_data_rsp_t));
        ota_data_rsp.state = 0x04; //unknow error
        
        tuya_ble_ota_rsp(rsp, &ota_data_rsp, sizeof(tuya_ble_ota_data_rsp_t));
        tuya_ble_ota_exit();
        return 1;
    }

    //param check
    tuya_ble_app_ota_data_t* ota_data = (void*)cmd;
    if(ota_data->type != 0x00) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_DATA- ota_data->type error");
        //rsp
        tuya_ble_ota_data_rsp_t ota_data_rsp;
        memset(&ota_data_rsp, 0x00, sizeof(tuya_ble_ota_data_rsp_t));
        ota_data_rsp.state = 0x04; //unknow error
        
        tuya_ble_ota_rsp(rsp, &ota_data_rsp, sizeof(tuya_ble_ota_data_rsp_t));
        tuya_ble_ota_exit();
        return 1;
    }
    
    {
        tuya_ble_inverted_array((void*)&ota_data->pkg_id, sizeof(uint16_t));
        tuya_ble_inverted_array((void*)&ota_data->len, sizeof(uint16_t));
        tuya_ble_inverted_array((void*)&ota_data->crc16, sizeof(uint16_t));
        
        //rsp
        tuya_ble_ota_data_rsp_t ota_data_rsp;
        memset(&ota_data_rsp, 0x00, sizeof(tuya_ble_ota_data_rsp_t));
        ota_data_rsp.type = 0x00;
        
        if(s_pkg_id+1 != ota_data->pkg_id) {
            ota_data_rsp.state = 0x01; //package id error
        }
        else if(cmd_size-7 != ota_data->len) {
            ota_data_rsp.state = 0x02; //size error
        }
        else if(tuya_ble_crc16_compute(ota_data->data, ota_data->len, NULL) != ota_data->crc16) {
            ota_data_rsp.state = 0x03; //crc error
        } else {
            ota_data_rsp.state = 0x00;
            
            //erase
            bool flag_4k = false;
            if((s_data_len == 0) || ((s_data_len + ota_data->len) >= (((s_data_len/TUYA_NV_ERASE_MIN_SIZE) + 1)*TUYA_NV_ERASE_MIN_SIZE)))
            {
                if(s_data_len == 0) {
                    for(uint16_t offset = 256; offset < 4096; offset += 256) {
                        tuya_ble_ota_flash_page_erase(offset + TUYA_BLE_OTA_START_ADDR);
                    }
                    first_flag = 1;
                } else {
                    uint32_t erase_addr = TUYA_BLE_OTA_START_ADDR + (((s_data_len/TUYA_NV_ERASE_MIN_SIZE) + 1)*TUYA_NV_ERASE_MIN_SIZE);
                    app_set_ota_state(1);
                    tuya_ble_nv_erase(erase_addr, TUYA_NV_ERASE_MIN_SIZE);
                    app_set_ota_state(0);
                }
                flag_4k = true;     
            }
            
            if(!first_flag) {
                if(0 != tuya_ble_ota_save_data(TUYA_BLE_OTA_START_ADDR + s_data_len, ota_data->data, ota_data->len)) {
                    ota_data_rsp.state = 0x04; //write error
                } else {
                    s_data_len += ota_data->len;
                    
                    if(s_data_len < s_file_len) {
                        s_ota_state = TUYA_BLE_OTA_DATA;
                    } else if(s_data_len == s_file_len) {
                        s_ota_state = TUYA_BLE_OTA_END;
                    } else {
                        ota_data_rsp.state = 0x04;
                    }
                    
                    s_pkg_id++;
                    TUYA_APP_LOG_INFO("s_pkg_id: %d", s_pkg_id);
                    
                    s_data_crc = tuya_ble_crc32_compute(ota_data->data, ota_data->len, &s_data_crc);
                    
                    if(flag_4k) {
                        s_old_file.len = s_data_len;
                        s_old_file.crc32 = s_data_crc;
                        tuya_ble_ota_info_save();
                    }
                }
            } else {
                memcpy(s_old_file.first_page, ota_data->data, ota_data->len);
                tuya_ble_ota_info_save();
                
                TUYA_APP_LOG_DEBUG("first pkt flash_write:0x%x\r\n",s_data_len);
                s_data_len += ota_data->len;
                
                s_pkg_id++;
                TUYA_APP_LOG_INFO("s_pkg_id: %d", s_pkg_id);
                
                s_data_crc = tuya_ble_crc32_compute(ota_data->data, ota_data->len, &s_data_crc);
            }
        }
        
        tuya_ble_ota_rsp(rsp, &ota_data_rsp, sizeof(tuya_ble_ota_data_rsp_t));
        
        if(ota_data_rsp.state != 0x00) {
            TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_DATA- errorid: %d", ota_data_rsp.state);
            tuya_ble_ota_exit();
        }
    }
    return 0;
}

/*********************************************************
FN: 
*/
static uint32_t tuya_ble_ota_end_handler(uint8_t* cmd, uint16_t cmd_size, tuya_ble_ota_response_t* rsp)
{
    //param check
    if(s_ota_state != TUYA_BLE_OTA_END) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_END- s_ota_state error");
        //rsp
        tuya_ble_ota_end_rsp_t end_rsp;
        memset(&end_rsp, 0x00, sizeof(tuya_ble_ota_end_rsp_t));
        end_rsp.state = 0x03; //unknow error
        
        tuya_ble_ota_rsp(rsp, &end_rsp, sizeof(tuya_ble_ota_end_rsp_t));
        tuya_ble_ota_exit();
        return 1;
    }

    //param check
    if((cmd_size != 0x0001) || (*cmd != 0x00)) {
        TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_END- type error");
        //rsp
        tuya_ble_ota_end_rsp_t end_rsp;
        memset(&end_rsp, 0x00, sizeof(tuya_ble_ota_end_rsp_t));
        end_rsp.state = 0x03; //unknow error
        
        tuya_ble_ota_rsp(rsp, &end_rsp, sizeof(tuya_ble_ota_end_rsp_t));
        tuya_ble_ota_exit();
        return 1;
    }
    
    {
        //rsp
        tuya_ble_ota_end_rsp_t end_rsp;
        memset(&end_rsp, 0x00, sizeof(tuya_ble_ota_end_rsp_t));
        end_rsp.type = 0x00;
        
        if(s_data_len != s_file_len) {
            end_rsp.state = 0x01; //total size error
        } else if(s_file_crc != tuya_ble_ota_get_crc32_in_flash(s_data_len)) {
            end_rsp.state = 0x02; //crc error
        } else {
            end_rsp.state = 0x00;
            s_ota_success = true;
            TUYA_APP_LOG_INFO("ota success");
            
            tuya_ble_ota_flash_page_erase(TUYA_BLE_OTA_START_ADDR);
            
            uint32_t* first_page_p = (uint32_t *)(s_old_file.first_page+0x18);
            *first_page_p = tuya_ble_ota_get_curr_firmwave_version() + 1;
            
            tuya_ble_ota_save_data(TUYA_BLE_OTA_START_ADDR, s_old_file.first_page, 256);
            
            memset((void*)&s_old_file, 0, sizeof(tuya_ble_ota_file_info_storage_t));
            tuya_ble_ota_info_save();
            
            tuya_ble_ota_exit();
        }
        
        tuya_ble_ota_rsp(rsp, &end_rsp, sizeof(tuya_ble_ota_end_rsp_t));
        
        if(end_rsp.state != 0x00) {
            TUYA_APP_LOG_ERROR("TUYA_BLE_OTA_END- errorid: %d", end_rsp.state);
            tuya_ble_ota_exit();
        }
    }
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t tuya_ble_ota_init(void)
{
    tuya_ble_ota_info_load();
    s_ota_state = TUYA_BLE_OTA_STATE_UNKNOWN;
    return 0;
}

/*********************************************************
FN: 
*/
void tuya_ble_ota_handler(tuya_ble_ota_data_t* ota)
{
    tuya_ble_ota_response_t rsp;
    rsp.type = ota->type;
    
    if(ota->type != TUYA_BLE_OTA_DATA) {
        TUYA_APP_LOG_INFO("ota_cmd_type: %d", ota->type);
        TUYA_APP_LOG_HEXDUMP_INFO("ota_cmd_data", ota->p_data, ota->data_len);
    }
    
    switch(ota->type)
    {
        case TUYA_BLE_OTA_REQ: {
            tuya_ble_ota_req_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_FILE_INFO: {
            tuya_ble_ota_file_info_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_FILE_OFFSET_REQ: {
            tuya_ble_ota_file_offset_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_DATA: {
            tuya_ble_ota_data_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_END: {
            tuya_ble_ota_end_handler(ota->p_data, ota->data_len, &rsp);
        } break;
        
        case TUYA_BLE_OTA_UNKONWN: {
        } break;
        
        default: {
        } break;
    }
}

/*********************************************************
FN: 
*/
uint32_t tuya_ble_ota_get_state(void)
{
    return s_ota_state;
}

/*********************************************************
FN: 
*/
uint32_t tuya_ble_ota_disconn_handler(void)
{
    if(s_ota_state >= TUYA_BLE_OTA_REQ) {
        return tuya_ble_ota_exit();
    } else {
        return 0;
    }
}






















