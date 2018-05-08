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
#include "ledmatrix.h"
#include <osapi.h>
#include <os_type.h>
#include <pwm.h>

#include "FreeMonoBold9pt7b.h" 

#define SAMPLE_PERIOD 1.5 // 200 ms
uint8_t pinsToTest[] = {0,2,4,5,12,13,14,15,16};
uint8_t pinsToTestLen = 9;

static  os_timer_t some_timer1;
static  os_timer_t some_timer;

char display_msg[128]="Embedded Makers";


volatile int x=0,last_x=0;
volatile unsigned long previousMillis = 0;
volatile unsigned long interval = 1000;
volatile unsigned long currentMillis=0;


static ETSTimer WiFiLinker;
WifiCallback wifiCb = NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;
static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
        struct ip_info ipConfig;

        os_timer_disarm(&WiFiLinker);
        wifi_get_ip_info(STATION_IF, &ipConfig);
        wifiStatus = wifi_station_get_connect_status();
        if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
        {

                os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
                os_timer_arm(&WiFiLinker, 2000, 0);


        }
        else
        {
                if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
                {

                        INFO("STATION_WRONG_PASSWORD\r\n");
                        wifi_station_connect();


                }
                else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
                {

                        INFO("STATION_NO_AP_FOUND\r\n");
                        wifi_station_connect();

                }
                else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
                {

                        INFO("STATION_CONNECT_FAIL\r\n");
                        wifi_station_connect();

                }
                else
                {
                        INFO("STATION_IDLE\r\n");
                }

                os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
                os_timer_arm(&WiFiLinker, 500, 0);
        }
        if(wifiStatus != lastWifiStatus){
                lastWifiStatus = wifiStatus;
                if(wifiCb)
                        wifiCb(wifiStatus);
        }
}

static void ICACHE_FLASH_ATTR
loop(void) {
  update_disp();
}

static void ICACHE_FLASH_ATTR
loop1(void) {
  static uint8_t f=0;
  update_disp();
  if(f){
	easygpio_outputSet(16, 0);
        f=0;
  }else{
        easygpio_outputSet(16, 1);
        f=1;
  }
}

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

static void ICACHE_FLASH_ATTR
movemsg(void){
	currentMillis++;

	if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        setCursor(x--,14);
        DisplayClear();
        print_line(display_msg);
        if(x==-1){
            last_x=cursor_x;
            interval=1000;
        }
        else{
            interval=40;
        }
        if(-x > (last_x)){
            x=0;
        }
    }
}

void user_init(void)
{
struct station_config stationConf;
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(60000);

    os_printf("\r\nSystem started ...\r\n");
        
        wifi_set_opmode_current(STATION_MODE);

        //wifi_station_set_auto_connect(FALSE);
        //wifiCb = cb;

        os_memset(&stationConf, 0, sizeof(struct station_config));

        os_sprintf(stationConf.ssid, "%s", "INTERNET");
        os_sprintf(stationConf.password, "%s", "internet321");

        wifi_station_set_config_current(&stationConf);

        os_timer_disarm(&WiFiLinker);
        os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
        os_timer_arm(&WiFiLinker, 1000, 0);

        //wifi_station_set_auto_connect(TRUE);
        wifi_station_connect();


    init();
    setFont(&FreeMonoBold9pt7b);
    
    setCursor(0,14);
    print_line(display_msg);
    last_x=cursor_x;
    gpio_init();
    
    os_timer_disarm(&some_timer);
    os_timer_setfn(&some_timer, (os_timer_func_t *)loop, NULL);
    os_timer_arm(&some_timer, SAMPLE_PERIOD, true);

    os_timer_disarm(&some_timer1);
    os_timer_setfn(&some_timer1, (os_timer_func_t *)movemsg, NULL);
    os_timer_arm(&some_timer1, 2, true);
    
   //WIFI_Connect("INTERNET", "internet321", wifiConnectCb);

}
