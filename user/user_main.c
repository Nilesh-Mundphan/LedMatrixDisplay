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
#include "espconn.h"

#include "FreeMonoBold9pt7b.h" 

#define SAMPLE_PERIOD 1.5 // 200 ms
uint8_t pinsToTest[] = {0,2,4,5,12,13,14,15,16};
uint8_t pinsToTestLen = 9;

static  os_timer_t some_timer1;
static  os_timer_t some_timer;

char display_msg[128]="Embedded Makers";
char ndisplay_msg[128];

volatile uint8_t isNewMsg=0;

volatile int x=0,last_x=0;
volatile unsigned long previousMillis = 0;
volatile unsigned long interval = 1000;
volatile unsigned long currentMillis=0;

LOCAL struct espconn esp_conn;
LOCAL esp_tcp esptcp;
 
#define SERVER_LOCAL_PORT   8080

LOCAL void ICACHE_FLASH_ATTR
tcp_server_sent_cb(void *arg)
{
   //data sent successfully
 
    os_printf("tcp sent cb \r\n");
}
 
 
/******************************************************************************
 * FunctionName : tcp_server_recv_cb
 * Description  : receive callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
   //received some data from tcp connection
    
   struct espconn *pespconn = arg;
   os_printf("tcp recv : %s \r\n", pusrdata);
   pusrdata[length-1]='\0';
   strcpy(ndisplay_msg,pusrdata); 
   espconn_sent(pespconn, pusrdata, length);
   isNewMsg=1;
   /*os_timer_disarm(&some_timer);
   DisplayClear();
   setCursor(x--,14);
   print_line(display_msg);
   last_x=cursor_x;
   interval=1000;
   x=0;
   currentMillis=0;
    os_timer_arm(&some_timer, SAMPLE_PERIOD, 0);
*/
}
 
/******************************************************************************
 * FunctionName : tcp_server_discon_cb
 * Description  : disconnect callback.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_discon_cb(void *arg)
{
   //tcp disconnect successfully
    
    os_printf("tcp disconnect succeed !!! \r\n");
}
 
/******************************************************************************
 * FunctionName : tcp_server_recon_cb
 * Description  : reconnect callback, error occured in TCP connection.
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_recon_cb(void *arg, sint8 err)
{
   //error occured , tcp connection broke. 
    
    os_printf("reconnect callback, error code %d !!! \r\n",err);
}
 
LOCAL void tcp_server_multi_send(void)
{
   struct espconn *pesp_conn = &esp_conn;
 
   remot_info *premot = NULL;
   uint8 count = 0;
   sint8 value = ESPCONN_OK;
   if (espconn_get_connection_info(pesp_conn,&premot,0) == ESPCONN_OK){
      char *pbuf = "tcp_server_multi_send\n";
      for (count = 0; count < pesp_conn->link_cnt; count ++){
         pesp_conn->proto.tcp->remote_port = premot[count].remote_port;
          
         pesp_conn->proto.tcp->remote_ip[0] = premot[count].remote_ip[0];
         pesp_conn->proto.tcp->remote_ip[1] = premot[count].remote_ip[1];
         pesp_conn->proto.tcp->remote_ip[2] = premot[count].remote_ip[2];
         pesp_conn->proto.tcp->remote_ip[3] = premot[count].remote_ip[3];
 
         espconn_sent(pesp_conn, pbuf, os_strlen(pbuf));
      }
   }
}
 
 
/******************************************************************************
 * FunctionName : tcp_server_listen
 * Description  : TCP server listened a connection successfully
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
tcp_server_listen(void *arg)
{
    struct espconn *pesp_conn = arg;
    os_printf("tcp_server_listen !!! \r\n");
 
    espconn_regist_recvcb(pesp_conn, tcp_server_recv_cb);
    espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
    espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);
     
    espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);
   tcp_server_multi_send();
}
 
/******************************************************************************
 * FunctionName : user_tcpserver_init
 * Description  : parameter initialize as a TCP server
 * Parameters   : port -- server port
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_tcpserver_init(uint32 port)
{
    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&esp_conn, tcp_server_listen);
 
    sint8 ret = espconn_accept(&esp_conn);
     
    os_printf("espconn_accept [%d] !!! \r\n", ret);
 
}
LOCAL os_timer_t test_timer;

void ICACHE_FLASH_ATTR
user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;
 
   //disarm timer first
    os_timer_disarm(&test_timer);
 
   //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);
 
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
 
      os_printf("got ip !!! \r\n");
      user_tcpserver_init(SERVER_LOCAL_PORT);
 
    } else {
        
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL)) {
                 
         os_printf("connect fail !!! \r\n");
          
        } else {
         
           //re-arm timer to check ip
            os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
            os_timer_arm(&test_timer, 100, 0);
        }
    }
}
 
/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to 
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
   // Wifi configuration 
   char ssid[32] = "INTERNET"; 
   char password[64] = "internet321"; 
   struct station_config stationConf; 
 
   //need not mac address
   stationConf.bssid_set = 0; 
    
   //Set ap settings 
   os_memcpy(&stationConf.ssid, ssid, 32); 
   os_memcpy(&stationConf.password, password, 64); 
   wifi_station_set_config(&stationConf); 
 
   //set a timer to check whether got ip from router succeed or not.
   os_timer_disarm(&test_timer);
   os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
   os_timer_arm(&test_timer, 100, 0);
 
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
	if(currentMillis > interval)
	{
        	currentMillis=0;
        	setCursor(x,14);
        	DisplayClear();
        	print_line(display_msg);

        	if(x==0){
            		last_x=cursor_x;
			if(isNewMsg)
			{
				os_memcpy(display_msg,ndisplay_msg,strlen(ndisplay_msg));
                		setCursor(x,14);
				DisplayClear();
                		print_line(display_msg);
				os_printf("msg:%s %d\n",display_msg,strlen(display_msg));
    				os_delay_us(60000);
            		        last_x=cursor_x;
				isNewMsg=0;
			}
	
			os_printf("Msg:%s MsgLen:%d\n",display_msg,strlen(display_msg));
			//os_printf("X: %d Last:L:%d\n",x,last_x);
            		interval=1000;
        	}
        	else{
            		interval=25;
        	}

		x--;
        	if(-x > (last_x)){
            		x=0;
        	}
    }
}

void ICACHE_FLASH_ATTR
loop(void) {
    os_timer_disarm(&some_timer);
    update_disp();
    movemsg();
    os_timer_arm(&some_timer, SAMPLE_PERIOD, 0);
}

void user_init(void)
{
struct station_config stationConf;
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(60000);

    os_printf("\r\nSystem started ...\r\n");
    
   //Set  station mode 
   wifi_set_opmode(STATIONAP_MODE); 
 
   // ESP8266 connect to router.
   user_set_station_config();    

    init();
    setFont(&FreeMonoBold9pt7b);
    
    setCursor(0,14);
    print_line(display_msg);
    last_x=cursor_x;
    gpio_init();
    
    os_timer_disarm(&some_timer);
    os_timer_setfn(&some_timer, (os_timer_func_t *)loop, NULL);
    os_timer_arm(&some_timer, SAMPLE_PERIOD, 0);

    //WIFI_Connect("INTERNET", "internet321", wifiConnectCb);

}
