/**
  * @file    esp8266.h
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-6
  * @brief	
  **/

#include "sys.h"

#define SSID        "Liang123"          //修改为自己的WIFI热点名
#define PASSWD      "12345678"              //修改为自己的WIFI密码
#define HTTP_SERVER_ADDR    "183.230.40.33"
#define EDP_SERVER_ADDR     "183.230.40.39"
#define MQTT_SERVER_ADDR    "183.230.40.39"
#define TCP_SERVER_ADDR			"192.168.137.1"

#define AT          "AT\r\n"	
#define CWMODE      "AT+CWMODE=3\r\n"		//STA+AP模式
#define RST         "AT+RST\r\n"
#define CIFSR       "AT+CIFSR\r\n"
#define CWJAP       "AT+CWJAP=\""SSID"\",\""PASSWD"\"\r\n"	
#define EDP_CIPSTART    "AT+CIPSTART=\"TCP\",\""EDP_SERVER_ADDR"\",876\r\n"	//EDP服务器 183.230.40.39/876
#define MQTT_CIPSTART   "AT+CIPSTART=\"TCP\",\""EDP_SERVER_ADDR"\",6002\r\n"	//MQTT服务器 183.230.40.39/876
#define HTTP_CIPSTART   "AT+CIPSTART=\"TCP\",\""HTTP_SERVER_ADDR"\",80\r\n"		//HTTP
#define TEST_CIPSTART		"AT+CIPSTART=\"TCP\",\""TCP_SERVER_ADDR"\",8000\r\n"
#define CIPCLOSE        "AT+CIPCLOSE\r\n"

#define CIPMODE0    "AT+CIPMODE=0\r\n"		//非透传模式
#define CIPMODE1    "AT+CIPMODE=1\r\n"		//透传模式
#define CIPSEND     "AT+CIPSEND\r\n"
#define CIPSTATUS   "AT+CIPSTATUS\r\n"		//网络状态查询


void ESP8266_Init(void);
u8 ESP_SendCMD(char* cmd, char* result,int timeout);
u8 ESP_TCP_CONNECT(char* cmd);
u8 Check_NETSTATUS(void);
u8 ESP_ENDSEND(void);




