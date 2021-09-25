#include "ty_uart.h"
#include "tuya_ble_api.h"




/*********************************************************************
 * LOCAL CONSTANT
 */
            
/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */
static volatile bool uart_is_init = false;
static volatile bool uart2_is_init = false;

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************************
 * VARIABLE
 */
__attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;

    uint8_t int_id = uart_reg->u3.iir.int_id;

    /* Receiver data available or Character time-out indication */
    if((int_id == 0x04) || (int_id == 0x0c)) {
        uint8_t ch = uart_reg->u1.data;
        
        //tuya_ble_modules_test_main(&ch);//TODO
        
    } else if(int_id == 0x06) {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

/*********************************************************************
 * VARIABLE
 */
__attribute__((section("ram_code"))) void uart1_isr_ram(void)
{
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART1_BASE;

    uint8_t int_id = uart_reg->u3.iir.int_id;

    /* Receiver data available or Character time-out indication */
    if((int_id == 0x04) || (int_id == 0x0c)) {
        uint8_t ch = uart_reg->u1.data;
        
        //TY_PRINTF("%02x ", ch);
        tuya_ble_common_uart_receive_data(&ch, 1);
        
    } else if(int_id == 0x06) {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

/*********************************************************************
 * LOCAL FUNCTION
 */
uint32_t ty_uart_init(void)
{
    system_set_port_pull(GPIO_PA0, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_UART0_TXD);
    uart_init(UART0, BAUD_RATE_115200);
    NVIC_EnableIRQ(UART0_IRQn);
    uart_is_init = true;
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t ty_uart2_init(void)
{
    system_set_port_pull(GPIO_PA2, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    uart_init(UART1, BAUD_RATE_115200);
    NVIC_EnableIRQ(UART1_IRQn);
    uart2_is_init = true;
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t ty_uart_send(const uint8_t* buf, uint32_t size)
{
    if(uart_is_init) {
        uart_write(UART0, buf, size);
    }
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t ty_uart2_send(const uint8_t* buf, uint32_t size)
{
    if(uart2_is_init) {
        uart_write(UART1,buf,size);
    }
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t ty_uart_uninit(void)
{
    system_set_port_pull(GPIO_PA0, false);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_A0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_A1);
    uart_is_init = false;
    return TUYA_BLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t ty_uart2_uninit(void)
{
    system_set_port_pull(GPIO_PA2, false);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_A2);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_A3);
    uart2_is_init = false;
    return TUYA_BLE_SUCCESS;
}






