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
 * 初始化配置ESP8266
 * @param   
 * @return 
 * @brief 
 **/
void ESP8266_Init(void)
{
	ESP_SendCMD(AT, "OK", 1000);		//模块有效性检查
	ESP_SendCMD(CWMODE, "OK", 1000);	//模块工作模式
	ESP_SendCMD(CIFSR, "OK", 1000);		//查询网络信息
	ESP_SendCMD(CWJAP, "OK", 2000);		//配置需要连接的WIFI热点SSID和密码
}

/**
 * 给ESP8266发送命令
 * @param  cmd[命令]  result[预期结果] timeout[超时计数]
 * @return 0[成功] 1[失败]
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
 * 配置ESP8266建立TCP连接
 * @param   cmd[TCP连接指令]
 * @return 	0[连接成功] 1[连接失败]
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
 * 查询当前ESP8266网络连接状态
 * @param   
 * @return 
 * @brief 
 **/
u8 Check_NETSTATUS(void)
{
	//清空当前串口数据缓存
	USART2_Clear();
	//发送查询命令
	printf("AT+CIPSTATUS\r\n");
	delay_ms(100);
	//已连接到AP
	if(rt_strstr(USART_RX_BUF,"STATUS:2") != NULL)
	{
		return 2;
	}
	//已建立连接
	else if(rt_strstr(USART_RX_BUF,"STATUS:3") != NULL)
	{
		return 3;
	}
	//已断开网络连接
	else if(rt_strstr(USART_RX_BUF,"STATUS:4") != NULL)
	{
		return 4;
	}
	//未连接到AP
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
 * ESP退出透传
 * @param   
 * @return 
 * @brief 
 **/
u8 ESP_ENDSEND(void)
{
	USART2_Clear();
	do{
		//发送结束透传命令
		printf("+++");
		//延时1s
		delay_ms(1000);
		//确认是否退出透传
		printf("AT\r\n");
		if(rt_strstr(USART_RX_BUF,"OK") != NULL)
		{
			return 0;
		}
		delay_ms(300);
	}while(1);
}

