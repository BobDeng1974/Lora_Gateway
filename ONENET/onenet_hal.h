/**
  * @file    onenet_hal.h
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-6
  * @brief	
  **/
#include "sys.h"

#define PROD_ID     "173303"             //修改为自己的产品ID
#define SN          "44415968"           //修改为自己的设备唯一序列号
#define REG_CODE    "wDGNAKmW2T2aEby1"   //修改为自己的产品注册码
#define API_ADDR    "api.heclouds.com"


#define DEVICE_NAME     "text_dev"SN

#define REG_PKT_HEAD    "POST http://"API_ADDR"/register_de?register_code="REG_CODE" HTTP/1.1\r\n"\
                        "Host: "API_ADDR"\r\n"\
                        "Content-Length: "
                    
#define REG_PKT_BODY    "{\"title\":\"123\",\"sn\":\"123\"}"

#define REG_PKT_HEADER  "GET http://api.heclouds.com/devices/514176240 HTTP/1.1\r\n" \
												"api-key: RD1GwmGn165d4BzP3SIwrFAT65E=\r\n" \
												"Host: api.heclouds.com\r\n\r\n"
												

void sendPkt(char *p, int len);
void Send_HttpPkt(char* phead,char* pbody);
char *uartDataParse(char *buffer, int32_t *plen);
u8 TCP_Upload_TR(char* device_id, char* device_key, int Temp_Val, int RH_Val);
u8 TCP_Upload_PT(char* device_id, char* device_key, int Pressure_Val, int TVOC_Val);

