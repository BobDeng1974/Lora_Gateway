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
	//�����߳� 1
  Updata_thread = rt_thread_create("t1", //�߳�1��������t1 
							Updata_thread_entry, RT_NULL, //�����thread1_entry��������RT_NULL 
							2048, //�̶߳�ջ��С
							3, //�߳����ȼ�
							20);//ʱ��Ƭtick
	//�����߳� 2
  Lora_thread = rt_thread_create("t2", //�߳�1��������t1 
							Lora_thread_entry, RT_NULL, //�����thread1_entry��������RT_NULL 
							4096, //�̶߳�ջ��С
							1, //�߳����ȼ�
							50);//ʱ��Ƭtick
	
	//�������ڶ�ʱ��
	loraupload_broadcast_timer = rt_timer_create("timer1", Loraupload_broadcast_entry,
																								RT_NULL, 5000,
																								RT_TIMER_FLAG_PERIODIC);
//	if(loraupload_broadcast_timer != RT_NULL)
//			rt_timer_start(loraupload_broadcast_timer);

	if (Lora_thread != RT_NULL) //�������߳̿��ƿ飬��������߳�
			rt_thread_startup(Lora_thread);	

//	if (Updata_thread != RT_NULL) //�������߳̿��ƿ飬��������߳�
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
		//��ȡ��ǰ����״̬
		Network_status = Check_NETSTATUS();
		//���δ���ӵ�AP
		if(Network_status == 5)
		{ ESP_SendCMD(CWJAP,"OK",2000); }
		//���δ���������������
		else if((Network_status == 2)||(Network_status == 4))
		{ ESP_TCP_CONNECT(HTTP_CIPSTART); }
		//���δ֪����
		else if(Network_status == 1)
		{ continue; }
		//����8266�Կ�ʼ��������
		//����͸��ģʽ
		ESP_SendCMD(CIPMODE1,"OK",500);
		//���ʹ�������
		ESP_SendCMD(CIPSEND,">",500);
		//�ϴ����ڵ������
		for(i=0;i<Global_GatewayStatus.upload_node_num;i++)
		{
			p = LoraNode_Find(Global_NodeList,Global_SensorData[i].node_mac);
			//������ҽڵ���Ϣʧ��,ֱ�������˽ڵ�
			if(p == NULL)
			{
				logging_debug("��ѯ�ڵ�:node_mac=%d��Ϣʧ��",Global_SensorData[i].node_mac);
				continue;
			}
			//����HTTP�����ϴ����ݵ�
			TCP_Upload_TR(p->device_id,p->device_key,Global_SensorData[i].temp,Global_SensorData[i].hum);
			rt_thread_delay(100);
			TCP_Upload_PT(p->device_id,p->device_key,56,78);
			rt_thread_delay(100);
		}
		//����͸��
		ESP_ENDSEND();
		LEDB = 1;
		rt_thread_delay(30000);
	}
}