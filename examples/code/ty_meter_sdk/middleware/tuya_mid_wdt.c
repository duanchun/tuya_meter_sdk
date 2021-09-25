
#include "ty_ble_wdt.h"
#include "driver_wdt.h"

__attribute__((section("ram_code"))) void wdt_isr_ram(unsigned int* hardfault_args)
{
    co_printf("wdt_rest\r\n\r\n");
    co_printf("PC    = 0x%08X\r\n",hardfault_args[6]);
    co_printf("LR    = 0x%08X\r\n",hardfault_args[5]);
    co_printf("R0    = 0x%08X\r\n",hardfault_args[0]);
    co_printf("R1    = 0x%08X\r\n",hardfault_args[1]);
    co_printf("R2    = 0x%08X\r\n",hardfault_args[2]);
    co_printf("R3    = 0x%08X\r\n",hardfault_args[3]);
    co_printf("R12   = 0x%08X\r\n",hardfault_args[4]);
    /* reset the system */
    wdt_init(WDT_ACT_RST_CHIP,1);
    ool_write(PMU_REG_WTD_CTRL, ool_read(PMU_REG_WTD_CTRL) | PMU_WTD_EN );
    while(1);
}

void ty_ble_wdt_init(uint8_t delay_s)
{
    wdt_init(WDT_ACT_RST_CHIP,delay_s);
}

void ty_ble_wdt_feed(void)
{
    wdt_feed();
}

void ty_ble_wdt_start(void)
{
    wdt_start();
}

void ty_ble_wdt_stop(void)
{
    wdt_stop();
}


