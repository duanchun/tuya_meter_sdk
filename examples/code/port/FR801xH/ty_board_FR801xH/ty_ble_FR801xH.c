#include "ty_ble.h"
#include "tuya_ble_api.h"
#include "tuya_ble_ota.h"
#include "tuya_ble_utils.h"
#include "tuya_ble_log.h"
#include "simple_gatt_service.h"
#include "ty_ble_config.h"

/*********************************************************************
 * LOCAL CONSTANT
 */
#define     DEFAULT_ADV_DATA    \
            {                   \
                3,              \
                {               \
                    0x02,       \
                    0x01,       \
                    0x06,       \
                },              \
            }
            
#define     DEFAULT_SCAN_RSP    \
            {                   \
                13,              \
                {               \
                    0x0C,       \
                    0x09,       \
                    't', 'y', '_', 'b', 'l', 'e', '_', 'd', 'e', 'm', 'o', \
                },              \
            }

#define     DEFAULT_ADV_PARAM               \
            {                               \
                .adv_interval_min = TY_ADV_INTERVAL,    \
                .adv_interval_max = TY_ADV_INTERVAL,    \
                .adv_type         = 0x01,   \
                .adv_power        = 0x00,   \
                .adv_channal_map  = 0x07,   \
            }

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
ty_ble_data_t        s_adv_data  = DEFAULT_ADV_DATA;
ty_ble_data_t        s_scan_rsp  = DEFAULT_SCAN_RSP;
ty_ble_adv_param_t   s_adv_param = DEFAULT_ADV_PARAM;

static volatile bool s_is_advertising = false;
volatile bool g_adv_restart_glag = false;


/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
uint32_t ty_ble_start_adv(void)
{
    gap_adv_param_t adv_param = {0};
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.disc_mode = GAP_ADV_DISC_MODE_GEN_DISC;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = ((s_adv_param.adv_interval_min*8)/5);
    adv_param.adv_intv_max = ((s_adv_param.adv_interval_max*8)/5);
    gap_set_advertising_param(&adv_param);
    
    ty_ble_set_advdata_and_scanrsp(NULL, NULL);
    
    gap_start_advertising(0);
    s_is_advertising = 1;
    TY_PRINTF("start adv");
	#if 0
	{
		char adv_name[128] = {0};
		int len = 0;
		memset(adv_name,0,sizeof(adv_name));
		len = gap_get_dev_name((uint8_t *)adv_name);
		TY_PRINTF("adv name==%s,len==%d\r\n",adv_name,len);
	}
	#endif
	//TUYA_APP_LOG_HEXDUMP_INFO("adv     data:",s_adv_data.value,s_adv_data.len);
    //TUYA_APP_LOG_HEXDUMP_INFO("scanrsp data:",s_scan_rsp.value,s_scan_rsp.len);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_restart_adv(void)
{
    ty_ble_start_adv();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_stop_adv(void)
{
    gap_stop_advertising();
    s_is_advertising = 0;
    TY_PRINTF("stop adv");
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_advdata_and_scanrsp(const ty_ble_data_t* p_adv_data, const ty_ble_data_t* p_scan_rsp)
{
    gap_set_advertising_data(s_adv_data.value+3, s_adv_data.len-3);
    gap_set_advertising_rsp_data(s_scan_rsp.value, s_scan_rsp.len);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_adv_param(const ty_ble_adv_param_t* p_param)
{
    s_adv_param.adv_interval_min = p_param->adv_interval_min;
    s_adv_param.adv_interval_max = p_param->adv_interval_max;
    
    if(s_is_advertising) {
        ty_ble_stop_adv();
        
        g_adv_restart_glag = true;
    }
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_connect(const ty_ble_mac_t* p_mac)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_reconnect(const ty_ble_mac_t* p_mac)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_disconnect(void)
{
    gap_disconnect_req(0);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_conn_param(uint16_t cMin, uint16_t cMax, uint16_t latency, uint16_t timeout)
{
    gap_conn_param_update(0, (cMin*4/5), (cMax*4/5), latency, (timeout/10));
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_mac(const ty_ble_mac_t* p_mac)
{
    mac_addr_t addr;
    memcpy(addr.addr, p_mac->addr, 6);
    gap_address_set(&addr);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_get_mac(ty_ble_mac_t* p_mac)
{
    mac_addr_t addr;
    gap_address_get(&addr);
    memcpy(p_mac->addr, addr.addr, 6);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_connect_handler(void)
{
    s_is_advertising = false;
    tuya_ble_connected_handler();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_disconnect_handler(void)
{
    tuya_ble_disconnected_handler();
    tuya_ble_ota_disconn_handler();
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_receive_data_handler(const uint8_t* buf, uint32_t size)
{
//    TY_PRINTF("ble rx size: %d", size);
    tuya_ble_gatt_receive_data((void*)buf, size);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_send_data(const uint8_t* buf, uint32_t size)
{
    gatt_ntf_t ntf_att_tuya;
    ntf_att_tuya.att_idx = SP_IDX_CHAR2_VALUE;
    ntf_att_tuya.conidx = 0;
    ntf_att_tuya.svc_id = sp_svc_id;
    ntf_att_tuya.data_len = MIN(size, gatt_get_mtu(0) - 3);
    ntf_att_tuya.p_data = (uint8_t *)buf;
    gatt_notification(ntf_att_tuya);
    return 0;
}

/*********************************************************
FN: 
*/
static uint8_t rssi_get_flag = 1;
static uint8_t ty_rssi;
__attribute__((section("ram_code"))) void gap_rssi_ind(int8_t rssi, uint8_t conidx)
{
   rssi_get_flag = 0;
   ty_rssi = rssi; 
}
uint32_t ty_ble_get_rssi(int8_t* p_rssi)
{
    rssi_get_flag = 1;
    ty_rssi = 0;
    
    gap_set_link_rssi_report(true);
    while(rssi_get_flag);
    
    *p_rssi = ty_rssi;
    gap_set_link_rssi_report(false);
    
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_tx_power(int8_t tx_power)
{
    system_set_tx_power((enum rf_tx_power_t )tx_power);  //0:-16dbm£¬7:0dbm
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_device_name(const uint8_t* buf, uint16_t size)
{
    gap_set_dev_name((uint8_t *)buf,size);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_get_device_name(uint8_t* buf, uint16_t* p_size)
{
    gap_get_dev_name(buf);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_dle(void)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_ble_set_read_data(uint8_t* buf, uint16_t len)
{
    sp_gatt_set_read_data(buf, len);
    return 0;
}


