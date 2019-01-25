/**
  * @file    app.c
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-9
  * @brief	
  **/

#include "app.h"

#include "onenet_hal.h"
#include "esp8266.h"
#include "led.h"
#include "radio.h"
#include "stdio.h"
#include "delay.h"
#include "../../radio/sx1276-Hal.h"
#include "lora_network.h"
#include "debug.h"

tRadioDriver *Radio = NULL;

const uint8_t MY_TEST_Msg[] = "SX1278_TEST";

void Main_APP()
{
	//创建线程 1
  Updata_thread = rt_thread_create("t1", //线程1的名称是t1 
							Updata_thread_entry, RT_NULL, //入口是thread1_entry，参数是RT_NULL 
							2048, //线程堆栈大小
							3, //线程优先级
							20);//时间片tick
	//创建线程 2
  Lora_thread = rt_thread_create("t2", //线程1的名称是t1 
							Lora_thread_entry, RT_NULL, //入口是thread1_entry，参数是RT_NULL 
							4096, //线程堆栈大小
							1, //线程优先级
							50);//时间片tick
	
	//创建周期定时器
	loraupload_broadcast_timer = rt_timer_create("timer1", Loraupload_broadcast_entry,
																								RT_NULL, 5000,
																								RT_TIMER_FLAG_PERIODIC);
//	if(loraupload_broadcast_timer != RT_NULL)
//			rt_timer_start(loraupload_broadcast_timer);

	if (Lora_thread != RT_NULL) //如果获得线程控制块，启动这个线程
			rt_thread_startup(Lora_thread);	

//	if (Updata_thread != RT_NULL) //如果获得线程控制块，启动这个线程
//			rt_thread_startup(Updata_thread);	
}

void Lora_thread_entry(void* parameter)
{
	Radio = RadioDriverInit();
	Radio->Init();
	Radio->SetTxPacket(MY_TEST_Msg,11);
	while(1)
	{
		if(Radio->Process( ) == RF_TX_DONE)
		{
			LEDG = 0;
			rt_thread_delay(500);
			LEDG = 1;
			rt_thread_delay(800);
			Radio->SetTxPacket(MY_TEST_Msg,11);
		}
	}
	LoraNetowrk_Gateway_Process(Radio);
}

void Loraupload_broadcast_entry(void* parameter)
{
	rt_kprintf("timeout\r\n");
}

void Updata_thread_entry(void* parameter)
{
	u8 Network_status;
	u8 i;
	Lora_Node *p;
	
	while (1)
	{
		LEDB = 0;
		//获取当前网络状态
		Network_status = Check_NETSTATUS();
		//如果未连接到AP
		if(Network_status == 5)
		{ ESP_SendCMD(CWJAP,"OK",2000); }
		//如果未建立与服务器连接
		else if((Network_status == 2)||(Network_status == 4))
		{ ESP_TCP_CONNECT(HTTP_CIPSTART); }
		//如果未知错误
		else if(Network_status == 1)
		{ continue; }
		//配置8266以开始传送数据
		//配置透传模式
		ESP_SendCMD(CIPMODE1,"OK",500);
		//发送传送命令
		ESP_SendCMD(CIPSEND,">",500);
		//上传各节点的数据
		for(i=0;i<Global_GatewayStatus.upload_node_num;i++)
		{
			p = LoraNode_Find(Global_NodeList,Global_SensorData[i].node_mac);
			//如果查找节点信息失败,直接跳过此节点
			if(p == NULL)
			{
				logging_debug("查询节点:node_mac=%d信息失败",Global_SensorData[i].node_mac);
				continue;
			}
			//发送HTTP请求上传数据点
			TCP_Upload_TR(p->device_id,p->device_key,Global_SensorData[i].temp,Global_SensorData[i].hum);
			rt_thread_delay(100);
			TCP_Upload_PT(p->device_id,p->device_key,56,78);
			rt_thread_delay(100);
		}
		//结束透传
		ESP_ENDSEND();
		LEDB = 1;
		rt_thread_delay(30000);
	}
}