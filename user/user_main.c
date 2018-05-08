#include "ets_sys.h"
#include "driver/uart.h"
#include "osapi.h"
#include "mqtt.h"
#include "wifi.h"
#include "config.h"
#include "debug.h"
#include "gpio.h"
#include "user_interface.h"
#include "mem.h"
#include "sntp.h"
#include "easygpio.h"
//#include "mygpio.h"


uint8_t pinsToTest[] = {0,2,4,5,12,13,14,15,16};
uint8_t pinsToTestLen = 9;

uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


void user_init(void)
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(60000);

    os_printf("\r\nSystem started ...\r\n");

    uint8_t i=0;
    for (i=0; i<pinsToTestLen; i++) {
       os_printf("Setting gpio%d as output\n", pinsToTest[i]);
       easygpio_pinMode(pinsToTest[i], EASYGPIO_NOPULL, EASYGPIO_OUTPUT);
    }

    easygpio_outputSet(pinsToTest[0], 1);
    while(1){
       os_printf("\r\nHello Sytem ...\r\n");
       os_delay_us(1000000);
    }
  //WIFI_Connect("INTERNET", "internet321", wifiConnectCb);

}
