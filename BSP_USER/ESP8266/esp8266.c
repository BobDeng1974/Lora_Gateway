/**
  * @file    esp8266.c
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-6
  * @brief	
  **/
	
#include "esp8266.h"
#include "usart2.h"
#include "sys.h"
#include "stdio.h"
#include "delay.h"
#include "rtthread.h"

/**
 * ��ʼ������ESP8266
 * @param   
 * @return 
 * @brief 
 **/
void ESP8266_Init(void)
{
	ESP_SendCMD(AT, "OK", 1000);		//ģ����Ч�Լ��
	ESP_SendCMD(CWMODE, "OK", 1000);	//ģ�鹤��ģʽ
	ESP_SendCMD(CIFSR, "OK", 1000);		//��ѯ������Ϣ
	ESP_SendCMD(CWJAP, "OK", 2000);		//������Ҫ���ӵ�WIFI�ȵ�SSID������
}

/**
 * ��ESP8266��������
 * @param  cmd[����]  result[Ԥ�ڽ��] timeout[��ʱ����]
 * @return 0[�ɹ�] 1[ʧ��]
 * @brief 
 **/
u8 ESP_SendCMD(char* cmd, char* result,int timeout)
{
	int i;
	
	while(1)
	{
		USART2_Clear();
		printf("%s",cmd);
		delay_ms(timeout);
		if(rt_strstr(USART_RX_BUF,result) != NULL)
		{
			return 0;
		}
		else
		{
			delay_ms(100);
		}
		i++;
		if(i >= 100)
			return 1;
	}
}

/**
 * ����ESP8266����TCP����
 * @param   cmd[TCP����ָ��]
 * @return 	0[���ӳɹ�] 1[����ʧ��]
 * @brief 
 **/
u8 ESP_TCP_CONNECT(char* cmd)
{
	int i;
	
	while(1)
	{
		USART2_Clear();
		printf("%s",cmd);
		delay_ms(500);
		if(rt_strstr(USART_RX_BUF,"OK") != NULL)
		{
			return 0;
		}
		else if(rt_strstr(USART_RX_BUF,"ALREADY CONNECT") != NULL)
		{
			return 0;
		}
		else
		{
			delay_ms(500);
		}
		i++;
		if(i >= 100)
			return 1;
	}	
}

/**
 * ��ѯ��ǰESP8266��������״̬
 * @param   
 * @return 
 * @brief 
 **/
u8 Check_NETSTATUS(void)
{
	//��յ�ǰ�������ݻ���
	USART2_Clear();
	//���Ͳ�ѯ����
	printf("AT+CIPSTATUS\r\n");
	delay_ms(100);
	//�����ӵ�AP
	if(rt_strstr(USART_RX_BUF,"STATUS:2") != NULL)
	{
		return 2;
	}
	//�ѽ�������
	else if(rt_strstr(USART_RX_BUF,"STATUS:3") != NULL)
	{
		return 3;
	}
	//�ѶϿ���������
	else if(rt_strstr(USART_RX_BUF,"STATUS:4") != NULL)
	{
		return 4;
	}
	//δ���ӵ�AP
	else if(rt_strstr(USART_RX_BUF,"STATUS:5") != NULL)
	{
		return 5;
	}
	else
	{
		return 1;
	}
}

/**
 * ESP�˳�͸��
 * @param   
 * @return 
 * @brief 
 **/
u8 ESP_ENDSEND(void)
{
	USART2_Clear();
	do{
		//���ͽ���͸������
		printf("+++");
		//��ʱ1s
		delay_ms(1000);
		//ȷ���Ƿ��˳�͸��
		printf("AT\r\n");
		if(rt_strstr(USART_RX_BUF,"OK") != NULL)
		{
			return 0;
		}
		delay_ms(300);
	}while(1);
}

