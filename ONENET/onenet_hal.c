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
 * ����HTTP���ݰ�
 * @param   
 * @return 
 * @brief 
 **/
void Send_HttpPkt(char* phead,char* pbody)
{
	char sendBuf0[20];
	char sendBuf1[500];

	//�ϳ�HTTP���ݰ�
	sprintf(sendBuf1, "%s%d\r\n\r\n%s", phead, (int)rt_strlen(pbody), pbody);
	sprintf(sendBuf0, "AT+CIPSEND=%d\r\n", strlen(sendBuf1));
	//��ջ�������
	USART2_Clear();
	//��ʼ����
	ESP_SendCMD(sendBuf0,">",500);
	/* EDP�豸���Ӱ������� */
	printf("%s%d",phead, (int)rt_strlen(pbody));
	printf("\r\n\r\n");
	printf("%s",pbody);
}



/**
 * ��͸��ģʽ����EDP����
 * @param   
 * @return 
 * @brief 
 **/
void sendPkt(char *p, int len)
{
    char sendBuf[30] = {0};

    /* ��͸��ģʽ�ȷ���AT+CIPSEND=X */
    sprintf(sendBuf, "AT+CIPSEND=%d\r\n", len);
    ESP_SendCMD(sendBuf, ">", 500);
    //printf("%s",sendBuf);
		USART2_Clear();

    /* EDP�豸���Ӱ������� */
    USART2_Write(p, len);    //���ڷ���
}

/**
  * @brief     �������ݽ���
  * @param     buffer�����ڽ������ݻ���
  * @param     plen�� ���ڱ��������EDP���ݵĳ���
  * @attention ���ڴ������ݲ�����8266ģ����ϱ���Ϣ����Ҫ���������ݽ��н���
  *            ����ģ���ֲᣬ���յ� +IPD, ������յ��˷��������·�����
  * @retval    ����������EDP���ݵ���ָ�룬��û�������򷵻�NULL
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
        pnum = p + 5;       //����ͷ���� "+IPD,"��ָ�򳤶��ֶε��׵�ַ
        p = rt_strstr(p, ":"); //ָ�򳤶��ֶ�ĩβ
        *(p++) = '\0';      //�����ֶ�ĩβ��ӽ�������pָ��������·��ĵ�һ���ֽ�
        len = atoi(pnum);
        *plen = len;
        return p;
    }
    return NULL;
}

/**
 * �ϴ���ʪ�����ݵ㵽������
 * @param   device_id[�豸ID] device_key[�豸��Կ] Temp_Val[�¶���ֵ] RH_Val[ʪ����ֵ]
 * @return 
 * @brief 
 **/
u8 TCP_Upload_TR(char* device_id, char* device_key, int Temp_Val, int RH_Val)
{
	char Send_Buff[3][100];
	
	//����JSON�ն���
	cJSON *json = cJSON_CreateObject();
	//�������
	cJSON *array = NULL;
	cJSON_AddItemToObject(json,"datastreams",array=cJSON_CreateArray());
	//������������¶ȶ���
	cJSON *obj1 = NULL;
	cJSON_AddItemToArray(array,obj1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1,"id",cJSON_CreateString("Lora_Temperature"));
	//���������
	cJSON *array1_1 = NULL;
	cJSON_AddItemToObject(obj1,"datapoints",array1_1=cJSON_CreateArray());
	//������������Ӷ���
	cJSON *obj1_1 = NULL;
	cJSON_AddItemToArray(array1_1,obj1_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1_1,"value",cJSON_CreateNumber(Temp_Val));
	//�����������ʪ�ȶ���
	cJSON *obj2 = NULL;
	cJSON_AddItemToArray(array,obj2=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2,"id",cJSON_CreateString("Lora_RH"));
	//���������
	cJSON *array2_1 = NULL;
	array2_1=cJSON_CreateArray();
	if(!array2_1)
		rt_kprintf("ERROR\r\n");
	cJSON_AddItemToObject(obj2,"datapoints",array2_1);
	//������������Ӷ���
	cJSON *obj2_1 = NULL;
	cJSON_AddItemToArray(array2_1,obj2_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2_1,"value",cJSON_CreateNumber(RH_Val));
	//ת��ΪJSON��ӡ��ʽ
	char *buf = cJSON_PrintUnformatted(json);
	//��������
	printf("POST http://api.heclouds.com/devices/%s/datapoints HTTP/1.1\r\n",device_id);
	printf("Host: api.heclouds.com\r\n");
	printf("api-key: %s\r\n",device_key);
	printf("content-length: %d\r\n\r\n",(int)rt_strlen(buf));
	printf("%s",buf);	
	//�ͷ��ڴ�
	cJSON_Delete(json);
	rt_free(buf);
}

/**
 * �ϴ���ѹ/TVOC���ݵ㵽������
 * @param   device_id[�豸ID] device_key[�豸��Կ] Pressure_Val[��ѹ��ֵ] TVOC_Val[TVOC��ֵ]
 * @return 
 * @brief 
 **/
u8 TCP_Upload_PT(char* device_id, char* device_key, int Pressure_Val, int TVOC_Val)
{
	char Send_Buff[3][100];
	
	//����JSON�ն���
	cJSON *json = cJSON_CreateObject();
	//�������
	cJSON *array = NULL;
	cJSON_AddItemToObject(json,"datastreams",array=cJSON_CreateArray());
	//����������Ӵ���ѹǿ����
	cJSON *obj1 = NULL;
	cJSON_AddItemToArray(array,obj1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1,"id",cJSON_CreateString("Lora_AirPressure"));
	//���������
	cJSON *array1_1 = NULL;
	cJSON_AddItemToObject(obj1,"datapoints",array1_1=cJSON_CreateArray());
	//������������Ӷ���
	cJSON *obj1_1 = NULL;
	cJSON_AddItemToArray(array1_1,obj1_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj1_1,"value",cJSON_CreateNumber(Pressure_Val));
	//�����������TVOC����
	cJSON *obj2 = NULL;
	cJSON_AddItemToArray(array,obj2=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2,"id",cJSON_CreateString("Lora_TVOC"));
	//���������
	cJSON *array2_1 = NULL;
	array2_1=cJSON_CreateArray();
	if(!array2_1)
		rt_kprintf("ERROR\r\n");
	cJSON_AddItemToObject(obj2,"datapoints",array2_1);
	//������������Ӷ���
	cJSON *obj2_1 = NULL;
	cJSON_AddItemToArray(array2_1,obj2_1=cJSON_CreateObject());
	cJSON_AddItemToObject(obj2_1,"value",cJSON_CreateNumber(TVOC_Val));
	//ת��ΪJSON��ӡ��ʽ
	char *buf = cJSON_PrintUnformatted(json);
	//��������
	printf("POST http://api.heclouds.com/devices/%s/datapoints HTTP/1.1\r\n",device_id);
	printf("Host: api.heclouds.com\r\n");
	printf("api-key: %s\r\n",device_key);
	printf("content-length: %d\r\n\r\n",(int)rt_strlen(buf));
	printf("%s",buf);	
	//�ͷ��ڴ�
	cJSON_Delete(json);
	rt_free(buf);
}


