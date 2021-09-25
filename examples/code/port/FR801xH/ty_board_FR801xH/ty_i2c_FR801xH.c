#include "ty_i2c.h"
#include "driver_iic.h"
#include "driver_system.h"
#include "driver_iomux.h"
#include "driver_gpio.h"
#include "driver_plf.h"
#include "driver_pmu.h"
#include "sys_utils.h"
#include "ty_oled.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */
uint8_t fr_salve_address = 0x35;

/*********************************************************************
 * LOCAL FUNCTION
 */
 
 
 

/*********************************************************
FN: 
*/
static void ty_i2c_scan_address(void)
{
    uint8_t  err_code;
    uint8_t address;
    uint8_t sample_data;
    uint8_t savle_flag  = 0;
    bool detected_device = false;

    TY_PRINTF("TWI scanner started.");
 
    for (address = 1; address <= 127; address++) {
        if(address== fr_salve_address) {
            address = address-1;
            savle_flag = 1;
        }
        
        err_code = iic_read_byte(IIC_CHANNEL_1, ((address<<1) | I2C_READ), 0,&sample_data);
        
        if(savle_flag) {
            address = address+1;
        }
        
        TY_PRINTF("address:0x%02x",address);
        
        if (err_code == true) {
            detected_device = true;
            TY_PRINTF("TWI-i2c device detected at address 0x%x.", address);
        }
    }
 
    if (!detected_device) {
        TY_PRINTF("No device was found.");
    }
 
    TY_PRINTF("TWI device scan ended.");
}

/*********************************************************
FN: 
*/
uint32_t ty_i2c_init(void)
{
    uint8_t errcode;
    uint8_t sample_data;
    
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_I2C1_CLK);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_I2C1_DAT);
    system_set_port_pull((GPIO_PD6|GPIO_PD7), true);
    
    iic_init(IIC_CHANNEL_1, 400, fr_salve_address);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_i2c_start(void)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_i2c_stop(void)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_i2c_control(uint8_t cmd, void* arg)
{
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_i2c_uninit(void)
{
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_D6);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_D7);
    return 0;
}

/*********************************************************
FN: 
*/
uint32_t ty_i2c_send(const uint8_t addr, const uint8_t* buf, uint32_t size)
{
    iic_write_bytes(IIC_CHANNEL_1, addr, buf[0], (void*)&buf[1], size-1);
    return 0;
}




/*********************************************************************
 * LOCAL CONSTANT
 */
#define DELAY_CNT 1
enum {
	GPIO_INPUT_MODE,
	GPIO_OUTPUT_MODE,
};

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
void i2c_delay(unsigned long tim_1us)
{
    co_delay_10us(tim_1us);
}

static void i2c_sda_pin_mode_set(uint8_t mode, uint8_t level)
{
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_D7);
    if(mode == GPIO_INPUT_MODE){
        system_set_port_pull(GPIO_PD7,true);
        gpio_set_dir(GPIO_PORT_D, GPIO_BIT_7,GPIO_DIR_IN);
	} else if(mode == GPIO_OUTPUT_MODE){
        gpio_set_dir(GPIO_PORT_D,GPIO_BIT_7,GPIO_DIR_OUT);
	}
}

static void i2c_sda_pin_set(uint8_t level)
{
    gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_7,level);
}

static void i2c_scl_pin_set(uint8_t level)
{
    gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_6,level);
}

static uint8_t i2c_sda_pin_status_get(void)
{
    return gpio_get_pin_value(GPIO_PORT_D,GPIO_BIT_7)?1:0;
}

/**
 * @description: i2c ack func
 * @param {type} none
 * @return: none
 */
static void i2c_ack(void)
{
    i2c_scl_pin_set(0);
    i2c_delay(DELAY_CNT);

    i2c_sda_pin_mode_set( GPIO_OUTPUT_MODE, 0 );
    i2c_sda_pin_set( 0 );
    i2c_delay(DELAY_CNT);
    
    i2c_scl_pin_set( 1 );
    i2c_delay(DELAY_CNT);
    i2c_scl_pin_set( 0 );
    i2c_delay(DELAY_CNT);
}

/**
 * @description: i2c none ack func
 * @param {type} none
 * @return: none
 */
static void i2c_noack(void)
{
    i2c_sda_pin_mode_set( GPIO_OUTPUT_MODE, 1 );
    i2c_sda_pin_set( 1 );

    i2c_delay(DELAY_CNT);
    i2c_scl_pin_set( 1 );
    i2c_delay(DELAY_CNT);
    i2c_scl_pin_set( 0 );
    i2c_delay(DELAY_CNT); 
}

/**
 * @description: i2c wait ack
 * @param {type} none
 * @return: rev ack return true else return false
 */
static uint8_t i2c_wait_ack(void)
{
    uint8_t cnt = 50;

    i2c_sda_pin_mode_set( GPIO_INPUT_MODE, 1 );/* set input and release SDA */
    i2c_sda_pin_set( 1 );
    i2c_delay(DELAY_CNT);

    i2c_scl_pin_set( 0 );       /* put down SCL ready to cheack SCA status */
    i2c_delay(DELAY_CNT);
    
    i2c_scl_pin_set( 1 );
    i2c_delay(DELAY_CNT);
    
    while( i2c_sda_pin_status_get() ) /* get ack */
    {
        cnt--;
        if( cnt==0 )
        {
            i2c_scl_pin_set( 0 );
            return false;
        }
        i2c_delay(DELAY_CNT);
    }
    
    i2c_scl_pin_set( 0 );
    i2c_delay(DELAY_CNT);
    return true;
}

/**
 * @description: i2c start signal
 * @param {type} none
 * @return: none
 */
void i2c_start(void)
{
    i2c_sda_pin_mode_set( GPIO_OUTPUT_MODE, 1 );    //SDA output mode

    i2c_scl_pin_set( 1 );
    i2c_sda_pin_set( 1 );
    i2c_delay(DELAY_CNT);

    i2c_sda_pin_set( 0 );
    i2c_delay(DELAY_CNT);
  
    i2c_scl_pin_set( 0 );
    i2c_delay(DELAY_CNT);
}

/**
 * @description: i2c stop signal
 * @param {type} none
 * @return: none
 */
void i2c_stop(void)
{
   i2c_sda_pin_mode_set( GPIO_OUTPUT_MODE, 0 );     //SDA input mode

    i2c_scl_pin_set( 0 );
    i2c_sda_pin_set( 0 );
    i2c_delay(DELAY_CNT);

    i2c_scl_pin_set( 1 );
    i2c_delay(DELAY_CNT);

    i2c_sda_pin_set( 1 );
    i2c_delay(DELAY_CNT);
}

/**
 * @description: send one byte to i2c bus
 * @param {uint8_t} data send to i2c
 * @return: none
 */
void i2c_send_byte(uint8_t data)
{
     uint8_t idx = 0;
    i2c_scl_pin_set( 0 );
    i2c_sda_pin_mode_set( GPIO_OUTPUT_MODE, 1 );
    
    for( idx=0; idx<8; idx++ ) {
        if( data & 0x80 ) {
            i2c_sda_pin_set( 1 );
        } else {
            i2c_sda_pin_set( 0 );
        }
        i2c_delay(DELAY_CNT);
        
        i2c_scl_pin_set( 1 );
        i2c_delay(DELAY_CNT);
        
        i2c_scl_pin_set( 0 );
        i2c_delay(DELAY_CNT);
        
        data <<= 1;
    }
}

/**
 * @description: send bytes to i2c bus
 * @param {type} none
 * @return: none
 */
void i2c_send_bytes(uint8_t adderss_cmd, uint8_t *buff, uint8_t len)
{
    uint8_t idx;
    i2c_send_byte( adderss_cmd );
    i2c_wait_ack();

    for( idx=0; idx<len; idx++ ) {
        i2c_send_byte( buff[idx] );
        i2c_wait_ack();
    }
}

/**
 * @description: recive one byte from i2c bus
 * @param {type} none
 * @return: none
 */
void i2c_rcv_byte(uint8_t *data)
{
    uint8_t idx;
    i2c_sda_pin_mode_set( GPIO_INPUT_MODE, 1 );
    i2c_delay(25);
    
    for( idx=0; idx<8; idx++ ) {
        i2c_scl_pin_set( 0 );
        i2c_delay(DELAY_CNT);

        i2c_scl_pin_set( 1 );
        *data = *data << 1;
        if( i2c_sda_pin_status_get() ) {
            *data |= 1;
        }
        i2c_delay(DELAY_CNT);
    }
    
    i2c_scl_pin_set( 0 );
}

/**
 * @description: recive bytes from i2c bus,last byte none ack
 * @param {type} none
 * @return: none
 */
void i2c_rcv_bytes(uint8_t adderss_cmd, uint8_t *buff, uint8_t len)
{
    uint8_t idx;
    i2c_send_byte( adderss_cmd );
    i2c_wait_ack();
    
    for( idx=0; idx<len; idx++ ) {
        i2c_rcv_byte( &buff[idx] );
        
        if( idx<len-1 ) {
            i2c_ack();
        } else {
            i2c_noack();
        }
    }
}

void i2c_soft_cfg(uint8_t adderss_cmd,uint8_t reg_addr,uint8_t data)
{
    i2c_start();
    i2c_send_byte(adderss_cmd);
    i2c_wait_ack();
    i2c_send_byte(reg_addr);
    i2c_wait_ack();
    i2c_send_byte(data);
    i2c_wait_ack();		
    i2c_stop();
}

void i2c_write_reg(uint8_t address_cmd,uint8_t register_addr)
{
	i2c_start();
    i2c_send_bytes(address_cmd,&register_addr ,1);
    i2c_stop();
}

void i2c_soft_gpio_init(void)
{
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_D7);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_D6);
	gpio_set_dir(GPIO_PORT_D,GPIO_BIT_6,GPIO_DIR_OUT);
    gpio_set_dir(GPIO_PORT_D,GPIO_BIT_7,GPIO_DIR_OUT);
}
