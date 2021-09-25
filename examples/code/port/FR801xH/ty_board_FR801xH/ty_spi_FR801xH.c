#include "ty_spi.h"
#include "driver_ssp.h"
#include "driver_system.h"
#include "driver_gpio.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;

/*********************************************************************
 * LOCAL STRUCT
 */
struct ssp_cr0 {
    u32 dss:4;  /* data size select : = DATASIZE - 1*/

    u32 frf:2;  /* frame format */

    u32 spo:1;  /* sclk polarity */
    u32 sph:1;  /* sclk phase */
    u32 scr:8;  /* serial clock rate */
    u32 unused:16;
};

struct ssp_cr1 {
    u32 rie:1;
    u32 tie:1;
    u32 rorie:1;

    u32 lbm:1;  /* loop back mode */
    u32 sse:1;  /* synchronous serial port enable*/

    u32 ms:1;   /* master mode or slave mode */
    u32 sod:1;  /* output disable in slave mode */

    u32 unused:25;
};

struct ssp_dr {
    u32 data;
};

struct ssp_sr {
    u32 tfe:1;  /* transmit fifo empty */
    u32 tnf:1;  /* transmit fifo not full */
    u32 rne:1;  /* receive fifo not empty */
    u32 rff:1;  /* receive fifo full */
    u32 bsy:1;  /* ssp busy flag */
    u32 unused:27;
};

struct ssp_cpsr {
    u32 cpsdvsr:8;  /* clock prescale divisor 2-254 */
    u32 unused:24;
};

struct ssp_iir {
    uint32_t ris:1;
    uint32_t tis:1;
    uint32_t roris:1;
    uint32_t reserved:29;
};

struct ssp {
    struct ssp_cr0 ctrl0;
    struct ssp_cr1 ctrl1; /*is also error clear register*/
    struct ssp_dr data;
    struct ssp_sr status;
    struct ssp_cpsr clock_prescale;
    struct ssp_iir iir;
};

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
void ty_spi_init(void)
{
}

/*********************************************************
FN: 
*/
void ty_spi_csn_set(bool pinState)
{
   gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_5,pinState);
}

/*********************************************************
FN: 
*/
void ty_spi_readWriteData(uint8_t *pWriteData, uint8_t *pReadData, uint8_t writeDataLen)
{
	volatile uint8_t temp = 0,block;
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    if(!writeDataLen) {
       return; 
    }
    
    block = writeDataLen/SSP_FIFO_SIZE;
   
    ty_spi_csn_set(0);
    ssp->ctrl1.sse = 1;
    while(block)
    {
        for(uint8_t i=0; i<SSP_FIFO_SIZE; i++) {
            ssp->data.data = *pWriteData++;
        }
        
        while(ssp->status.bsy){}
        
        for(uint8_t i=0; i<SSP_FIFO_SIZE; i++) {
            *pReadData++ = ssp->data.data;
        }
        
        block--;
        writeDataLen -= SSP_FIFO_SIZE;
    }
    
    while(writeDataLen)
    {
        for(uint8_t i = 0; i < writeDataLen;i++) {
            ssp->data.data = *pWriteData++;
            writeDataLen--;
        }
        
        while(ssp->status.bsy){}
        while(ssp->status.rne == 0){}
        
        while(ssp->status.rne) {
            *pReadData++ = ssp->data.data;
        }
    }
    
    ssp->ctrl1.sse = 0;
    ty_spi_csn_set(1);
}

/*********************************************************
FN: Turn on SPI, which is different from initialization: CS pin is not initialized
*/
void ty_spi_enable(void)
{
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_A5);
    gpio_set_dir(GPIO_PORT_A, GPIO_BIT_5, GPIO_DIR_OUT);
	system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSP0_CLK);
//    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_SSP0_CSN);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_SSP0_DOUT);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_SSP0_DIN);

    ssp_init_(8, SSP_FRAME_MOTO, SSP_MASTER_MODE, 1000000, 2, NULL);
}

/*********************************************************
FN: 
*/
void ty_spi_disable(void)
{
	system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_A4);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_A5);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_A6);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_A7);
}

















