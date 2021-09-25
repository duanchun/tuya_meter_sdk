/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>
#include "co_printf.h"
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"

#include "simple_gatt_service.h"
#include "driver_uart.h"
#include "sys_utils.h"
#include "os_timer.h"
#include "os_mem.h"
#include "tuya_ble_api.h"
#include "ty_ble.h"

/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */
#define BLE_CONNECTION_MAX (20)
static uint16_t ntf_enable_flag[BLE_CONNECTION_MAX] = {0};

// Simple GATT Profile Service UUID: 0xFFF0
const uint8_t sp_svc_uuid[] = UUID16_ARR(SP_SVC_UUID);

/******************************* Characteristic 1 defination *******************************/
// Characteristic 1 UUID: 0xFFF1
// Characteristic 1 data 
#define SP_CHAR1_VALUE_LEN  251
uint8_t sp_char1_value[SP_CHAR1_VALUE_LEN] = {0};


/******************************* Characteristic 2 defination *******************************/
// Characteristic 2 UUID: 0xFFF2
// Characteristic 2 data 
#define SP_CHAR2_VALUE_LEN  251
uint8_t sp_char2_value[SP_CHAR2_VALUE_LEN] = {0};
// Characteristic 2 User Description
#define SP_CHAR2_CCC_LEN   2
uint8_t sp_char2_ccc[SP_CHAR2_CCC_LEN] = {0};


/******************************* Characteristic 3 defination *******************************/
// Characteristic 3 UUID: 0xFFF3
// Characteristic 3 data 
#define SP_CHAR3_VALUE_LEN  62
uint8_t sp_char3_value[SP_CHAR3_VALUE_LEN] = {0};


/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */
struct uart_recv_t
{
  uint8_t start_flag;
  uint8_t time_count;
  uint8_t indx;
  uint8_t length;
  uint8_t recv[251];  
};
struct uart_recv_t uart_recv = 
{
  .start_flag = 0,
  .time_count = 0,
  .indx = 0,
  .length = 0,
  .recv = {0},  
};

uint8_t sp_svc_id = 0;
uint8_t nty_enable_flag = 0;
uint8_t user_conn_idx = 0;
os_timer_t uart_recv_timeout_id;
/*
 * LOCAL VARIABLES (本地变量)
 */
static gatt_service_t simple_profile_svc;

#define  sp_char1_write_uuid     {0xD0, 0x07, 0x9B, 0x5F, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00}
#define  sp_char2_notify_uuid    {0xD0, 0x07, 0x9B, 0x5F, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00}
#define  sp_char3_read_uuid      {0xD0, 0x07, 0x9B, 0x5F, 0x80, 0x00, 0x01, 0x80, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00}

/*********************************************************************
 * Profile Attributes - Table
 * 每一项都是一个attribute的定义。
 * 第一个attribute为Service 的的定义。
 * 每一个特征值(characteristic)的定义，都至少包含三个attribute的定义；
 * 1. 特征值声明(Characteristic Declaration)
 * 2. 特征值的值(Characteristic value)
 * 3. 特征值描述符(Characteristic description)
 * 如果有notification 或者indication 的功能，则会包含四个attribute的定义，除了前面定义的三个，还会有一个特征值客户端配置(client characteristic configuration)。
 *
 */

const gatt_attribute_t simple_profile_att_table[SP_IDX_NB] =
{
    // Simple gatt Service Declaration
    [SP_IDX_SERVICE]                        =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID) },     /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    UUID_SIZE_2,                                                /* Max size of the value */     /* Service UUID size in service declaration */
                                                    (uint8_t*)sp_svc_uuid,                                      /* Value of the attribute */    /* Service UUID value in service declaration */
                                                },

//        // Characteristic 1 Declaration           
        [SP_IDX_CHAR1_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 1 Value                  
        [SP_IDX_CHAR1_VALUE]                =   {
                                                    { UUID_SIZE_16, sp_char1_write_uuid},                      /* UUID */
                                                     GATT_PROP_WRITE | GATT_PROP_WRITE_CMD,                                          /* Permissions */
                                                    SP_CHAR1_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },             
//        // Characteristic 1 User Description
//        [SP_IDX_CHAR1_USER_DESCRIPTION]     =   {
//                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },      /* UUID */
//                                                    GATT_PROP_READ,                                             /* Permissions */
//                                                    SP_CHAR1_DESC_LEN,                                          /* Max size of the value */
//                                                    (uint8_t *)sp_char1_desc,                                   /* Value of the attribute */
//                                                },


//        // Characteristic 2 Declaration
        [SP_IDX_CHAR2_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 2 Value   
        [SP_IDX_CHAR2_VALUE]                =   {
                                                    { UUID_SIZE_16, sp_char2_notify_uuid },                 /* UUID */
                                                     GATT_PROP_NOTI,                                             /* Permissions */
                                                    SP_CHAR2_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */	/* Can assign a buffer here, or can be assigned in the application by user */
                                                },   
//        // Characteristic 2 User Description
//        [SP_IDX_CHAR2_USER_DESCRIPTION]     =   {
//                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHAR_USER_DESC_UUID) },       /* UUID */
//                                                    GATT_PROP_READ,                                             /* Permissions */
//                                                    SP_CHAR2_DESC_LEN,                                          /* Max size of the value */
//                                                    (uint8_t *)sp_char2_desc,                                   /* Value of the attribute */
//                                                },


        // Characteristic 2 client characteristic configuration
        [SP_IDX_CHAR2_CFG]                  =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },     /* UUID */
                                                    GATT_PROP_READ | GATT_PROP_WRITE,                                            /* Permissions */
                                                    SP_CHAR2_CCC_LEN,                                           /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },
        
                                                    
        [SP_IDX_CHAR3_DECLARATION]          =   {
                                                    { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },           /* UUID */
                                                    GATT_PROP_READ,                                             /* Permissions */
                                                    0,                                                          /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */
                                                },
        // Characteristic 3 Value                  
        [SP_IDX_CHAR3_VALUE]                =   {
                                                    { UUID_SIZE_16, sp_char3_read_uuid},                      /* UUID */
                                                    GATT_PROP_READ,                                          /* Permissions */
                                                    SP_CHAR3_VALUE_LEN,                                         /* Max size of the value */
                                                    NULL,                                                       /* Value of the attribute */    /* Can assign a buffer here, or can be assigned in the application by user */
                                                },       
        
};

/*********************************************************************
 * @fn      sp_gatt_read_cb
 *
 * @brief   Simple Profile user application handles read request in this callback.
 *			应用层在这个回调函数里面处理读的请求。
 *
 * @param   p_read  - the pointer to read buffer. NOTE: It's just a pointer from lower layer, please create the buffer in application layer.
 *					  指向读缓冲区的指针。 请注意这只是一个指针，请在应用程序中分配缓冲区. 为输出函数, 因此为指针的指针.
 *          len     - the pointer to the length of read buffer. Application to assign it.
 *                    读缓冲区的长度，用户应用程序去给它赋值.
 *          att_idx - index of the attribute value in it's attribute table.
 *					  Attribute的偏移量.
 *
 * @return  读请求的长度.
 */
static void sp_gatt_read_cb(uint8_t *p_read, uint16_t *len, uint16_t att_idx)
{
    switch (att_idx)
    {
        case SP_IDX_CHAR1_VALUE:
            for (int i = 0; i < SP_CHAR1_VALUE_LEN; i++)
                sp_char1_value[i] = sp_char1_value[0] + i + 1;
            memcpy(p_read, sp_char1_value, SP_CHAR1_VALUE_LEN);
            *len = SP_CHAR1_VALUE_LEN;
        break;

        case SP_IDX_CHAR2_VALUE:
            for (int i = 0; i < SP_CHAR2_VALUE_LEN; i++)
                sp_char2_value[i] = sp_char2_value[0] + i + 1;
            memcpy(p_read, sp_char2_value, SP_CHAR2_VALUE_LEN);
            *len = SP_CHAR2_VALUE_LEN;
       break;
        
        case SP_IDX_CHAR2_CFG:
            *len = 2;
            memcpy(p_read, sp_char2_ccc, 2);
        break;
        case SP_IDX_CHAR3_VALUE:
            memcpy(p_read, sp_char3_value, SP_CHAR3_VALUE_LEN);
            *len = SP_CHAR3_VALUE_LEN;
       break;
        default:
        break;
    }
    
	TY_PRINTF("Read request: len: %d  att_idx: %d", *len, att_idx);
    //TUYA_APP_LOG_HEXDUMP_INFO("att read:", p_read, SP_CHAR3_VALUE_LEN);
}

void sp_gatt_set_read_data(uint8_t *read_buf, uint16_t len)
{
    memcpy(sp_char3_value, read_buf,SP_CHAR3_VALUE_LEN);
}

/*********************************************************************
 * @fn      sp_gatt_write_cb
 *
 * @brief   Simple Profile user application handles write request in this callback.
 *			应用层在这个回调函数里面处理写的请求。
 *
 * @param   write_buf   - the buffer for write
 *			              写操作的数据.
 *					  
 *          len         - the length of write buffer.
 *                        写缓冲区的长度.
 *          att_idx     - index of the attribute value in it's attribute table.
 *					      Attribute的偏移量.
 *
 * @return  写请求的长度.
 */
static void sp_gatt_write_cb(uint8_t *write_buf, uint16_t len, uint16_t att_idx)
{
    if(att_idx == SP_IDX_CHAR1_VALUE) {
        ty_ble_receive_data_handler(write_buf, len);
    }
}
/*********************************************************************
 * @fn      sp_gatt_msg_handler
 *
 * @brief   Simple Profile callback funtion for GATT messages. GATT read/write
 *			operations are handeled here.
 *
 * @param   p_msg       - GATT messages from GATT layer.
 *
 * @return  uint16_t    - Length of handled message.
 */
static uint16_t sp_gatt_msg_handler(gatt_msg_t *p_msg)
{
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_READ_REQ:
            sp_gatt_read_cb((uint8_t *)(p_msg->param.msg.p_msg_data), &(p_msg->param.msg.msg_len), p_msg->att_idx);
            break;
        
        case GATTC_MSG_WRITE_REQ:
            sp_gatt_write_cb((uint8_t*)(p_msg->param.msg.p_msg_data), (p_msg->param.msg.msg_len), p_msg->att_idx);
            user_conn_idx = p_msg->conn_idx;
            break;
            
        default:
            break;
    }
    return p_msg->param.msg.msg_len;
}

/*********************************************************************
 * @fn      sp_gatt_add_service
 *
 * @brief   Simple Profile add GATT service function.
 *			添加GATT service到ATT的数据库里面。
 *
 * @param   None. 
 *        
 *
 * @return  None.
 */
void sp_gatt_add_service(void)
{
    nty_enable_flag = 0;
	simple_profile_svc.p_att_tb = simple_profile_att_table;
	simple_profile_svc.att_nb = SP_IDX_NB;
	simple_profile_svc.gatt_msg_handler = sp_gatt_msg_handler;
	
	sp_svc_id = gatt_add_service(&simple_profile_svc);
}








