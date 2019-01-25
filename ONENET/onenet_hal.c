/**
  * @file    onenet_hal.c
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-6
  * @brief	
  **/
	
#include "onenet_hal.h"
#include "rtthread.h"
#include "esp8266.h"
#include "usart2.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cJSON.h"
#include "delay.h"

/**
 * 发送HTTP数据包
 * @param   
 * @return 
 * @brief 
 **/
void Send_HttpPkt(char* phead,char* pbody)
{
	char sendBuf0[20];
	char sendBuf1[500];

	//合成HTTP数据包
	sprintf(sendBuf1, "%s%d\r\n\r\n%s", phead, (int)rt_strlen(pbody), pbody);
	sprintf(sendBuf0, "AT+CIPSEND=%d\r\n", strlen(sendBuf1));
	//清空缓存数组
	USART2_Clear();
	//开始发送
	ESP_SendCMD(sendBuf0,">",500);
	/* EDP设备连接包，发送 */
	printf("%s%d",phead, (int)rt_strlen(pbody));
	printf("\r\n\r\n");
	printf("%s",pbody);
}



/**
 * 非透传模式发送EDP报文
 * @param   
 * @return 
 * @brief 
 **/
void sendPkt(char *p, int len)
{
    char sendBuf[30] = {0};

    /* 非透传模式先发送AT+CIPSEND=X */
    sprintf(sendBuf, "AT+CIPSEND=%d\r\n", len);
    ESP_SendCMD(sendBuf, ">", 500);
    //printf("%s",sendBuf);
		USART2_Clear();

    /* EDP设备连接包，发送 */
    USART2_Write(p, len);    //串口发送
}

/**
  * @brief     串口数据解析
  * @param     buffer，串口接收数据缓存
  * @param     plen， 用于保存解析后EDP数据的长度
  * @attention 由于串口数据参杂着8266模块的上报信息，需要将串口数据进行解析
  *            根据模块手册，若收到 +IPD, 则表明收到了服务器的下发数据
  * @retval    串口数据中EDP数据的首指针，若没有数据则返回NULL
  */
char *uartDataParse(char *buffer, int32_t *plen)
{
    char *p;
    char *pnum;
    int32_t len;
	
    if((p = rt_strstr(buffer, "CLOSED")) != NULL)
    {
        rt_kprintf("tcp connection closed\r\n");
    }
    if((p = rt_strstr(buffer, "WIFI DISCONNECT")) != NULL)
    {
        rt_kprintf("wifi disconnected\r\n");
    }
    if((p = rt_strstr(buffer, "WIFI CONNECTED")) != NULL)
    {
        rt_kprintf("wifi connected\r\n");
    }
    if((p = rt_strstr(buffer, "+IPD")) != NULL)
    {
        pnum = p + 5;       //跳过头部的 "+IPD,"，指向长度字段的首地址
        p = rt_strstr(p, ":"); //指向长度字段末尾
        *(p++) = '\0';      //长度字段末尾添加结束符，p指向服务器下发的第一个字节
        len = atoi(pnum);
        *plen = len;
        return p;
    }
    return NULL;
}

/**
 * 上传温湿度数据点到服务器
 * @param   device_id[设备ID] device_key[设备密钥] Temp_Val[温度数值] RH_Val[湿度数值]
 * @return 
 * @brief 
 **/
u8 TCP_Upload_TR(char* device_id, char* device_key, int Temp_Val, int RH_Val)
{
	char Send_Buff[3][100];
	
	//创建JSON空对象
	cJSON *json = cJSON_CreateObject();
	//添加数组
	cJSON *array = NULL;
	cJSON_AddItemToObject(json,"datastreams",array=cJSON_CreateArray());
	//在数组上添加温度对象
	cJSON *obj1 = NULL;
	cJSON_AddItemToArray(array,obj1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1,"id",cJSON_CreateString("Lora_Temperature"));
	//添加子数组
	cJSON *array1_1 = NULL;
	cJSON_AddItemToObject(obj1,"datapoints",array1_1=cJSON_CreateArray());
	//在子数组上添加对象
	cJSON *obj1_1 = NULL;
	cJSON_AddItemToArray(array1_1,obj1_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1_1,"value",cJSON_CreateNumber(Temp_Val));
	//在数组上添加湿度对象
	cJSON *obj2 = NULL;
	cJSON_AddItemToArray(array,obj2=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2,"id",cJSON_CreateString("Lora_RH"));
	//添加子数组
	cJSON *array2_1 = NULL;
	array2_1=cJSON_CreateArray();
	if(!array2_1)
		rt_kprintf("ERROR\r\n");
	cJSON_AddItemToObject(obj2,"datapoints",array2_1);
	//在子数组上添加对象
	cJSON *obj2_1 = NULL;
	cJSON_AddItemToArray(array2_1,obj2_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2_1,"value",cJSON_CreateNumber(RH_Val));
	//转换为JSON打印格式
	char *buf = cJSON_PrintUnformatted(json);
	//发送数据
	printf("POST http://api.heclouds.com/devices/%s/datapoints HTTP/1.1\r\n",device_id);
	printf("Host: api.heclouds.com\r\n");
	printf("api-key: %s\r\n",device_key);
	printf("content-length: %d\r\n\r\n",(int)rt_strlen(buf));
	printf("%s",buf);	
	//释放内存
	cJSON_Delete(json);
	rt_free(buf);
}

/**
 * 上传气压/TVOC数据点到服务器
 * @param   device_id[设备ID] device_key[设备密钥] Pressure_Val[气压数值] TVOC_Val[TVOC数值]
 * @return 
 * @brief 
 **/
u8 TCP_Upload_PT(char* device_id, char* device_key, int Pressure_Val, int TVOC_Val)
{
	char Send_Buff[3][100];
	
	//创建JSON空对象
	cJSON *json = cJSON_CreateObject();
	//添加数组
	cJSON *array = NULL;
	cJSON_AddItemToObject(json,"datastreams",array=cJSON_CreateArray());
	//在数组上添加大气压强对象
	cJSON *obj1 = NULL;
	cJSON_AddItemToArray(array,obj1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1,"id",cJSON_CreateString("Lora_AirPressure"));
	//添加子数组
	cJSON *array1_1 = NULL;
	cJSON_AddItemToObject(obj1,"datapoints",array1_1=cJSON_CreateArray());
	//在子数组上添加对象
	cJSON *obj1_1 = NULL;
	cJSON_AddItemToArray(array1_1,obj1_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1_1,"value",cJSON_CreateNumber(Pressure_Val));
	//在数组上添加TVOC对象
	cJSON *obj2 = NULL;
	cJSON_AddItemToArray(array,obj2=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2,"id",cJSON_CreateString("Lora_TVOC"));
	//添加子数组
	cJSON *array2_1 = NULL;
	array2_1=cJSON_CreateArray();
	if(!array2_1)
		rt_kprintf("ERROR\r\n");
	cJSON_AddItemToObject(obj2,"datapoints",array2_1);
	//在子数组上添加对象
	cJSON *obj2_1 = NULL;
	cJSON_AddItemToArray(array2_1,obj2_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2_1,"value",cJSON_CreateNumber(TVOC_Val));
	//转换为JSON打印格式
	char *buf = cJSON_PrintUnformatted(json);
	//发送数据
	printf("POST http://api.heclouds.com/devices/%s/datapoints HTTP/1.1\r\n",device_id);
	printf("Host: api.heclouds.com\r\n");
	printf("api-key: %s\r\n",device_key);
	printf("content-length: %d\r\n\r\n",(int)rt_strlen(buf));
	printf("%s",buf);	
	//释放内存
	cJSON_Delete(json);
	rt_free(buf);
}


