#include "sys.h"
#include "radio.h"
#include "lora_network.h"
#include "stdio.h"
#include "delay.h"

#if defined(USED_LORANETWORK_NODE)

LoraNode_Data Global_SensorDataBuff;	//全局传感器缓存区


/**
 * lora 本地节点  注册入网过程
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode__Registe(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	uint8_t status;
	uint16_t timeout;
	uint16_t buffer_size;
	uint8_t Buffer[100];
	LoraNetwork_Gateway_Pack rev_pack;
	
	// 发送入网请求
	LoraNode_SendRegPack(Radio,node_info);
	// 等待注册响应信息
	do{
		if(Radio->Process()==RF_TX_DONE)
			Radio->StartRx();
		if(Radio->Process()==RF_RX_DONE)
		{
			Radio->GetRxPacket(Buffer, (uint16_t*)&buffer_size);
			//如果接收到的数据包不符合集中器数据包大小
			if(buffer_size != sizeof(LoraNetwork_Gateway_Pack))
				continue;
			else
			{
				//还原集中器数据包
				lora_memcpy(&rev_pack,Buffer,sizeof(LoraNetwork_Gateway_Pack));
				//忽略非广播数据包
				if(rev_pack.node_id != 0)
					continue;
				//忽略不是注册响应的数据包
				if(rev_pack.pack_type != LORA_REGACK)
					continue;
				//提取注册信息,返回正确标志
				if(LoraNode_RegAck(&rev_pack,node_info) == LORA_OK)
					return LORA_OK;
		}
		delay_ms(100);
	}while(timeout<100);
	//超时返回错误状态
	return LORA_SYSTEM_ERROR;
}


/**
 * lora 本地节点  发送注册入网请求
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendRegPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// 检查组网有效性
	if(Radio==NULL || node_info->LocalNode_id != 0)
		return LORA_SYSTEM_ERROR;
	// 封装注册入网请求包
	send_pack.node_id = LORA_BROADCAST_ID;	//广播发送
	send_pack.pack_type = LORA_REG;					//注册请求
	send_pack.node_mac = node_info->LocalNode_Mac;
	// 发送数据包
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;
}


/**
 * lora 本地节点  处理注册响应
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_RegAck(LoraNetwork_Gateway_Pack *rev_pack,LocalNode_INFO *node_info)
{
	uint8_t temp[2];
	
	// 如果已经注册了则直接返回
	if(node_info->LocalNode_id != 0)
		return LORA_OK;
	// 提取负载信息
	temp[0] = rev_pack->Load[0];	//提取目的节点MAC
	temp[1] = rev_pack->Load[1];  //提取即将分配的node_id
	// 判断是否为本节点分配的node_id
	if(temp[0] != node_info->LocalNode_Mac)
		return LORA_DATA_ERROR;
	node_info->LocalNode_id = temp[1];
	
	return LORA_OK;
}

/**
 * lora 本地节点  发送一般响应
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendAckPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// 检查组网有效性
	if(Radio==NULL || node_info->LocalNode_id == 0)
		return LORA_NODE_OFFLINE;
	// 封装一般响应包
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LOEA_ACK;
	send_pack.node_mac = node_info->LocalNode_Mac;
	// 发送数据包
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;	
}

Lora_Sataus LoraNode_SendDataPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// 检查组网有效性
	if(Radio==NULL || node_info->LocalNode_id == 0)
		return LORA_NODE_OFFLINE;
	// 封装发送数据包
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LORA_NODE_UPLOAD;
	send_pack.node_mac = node_info->LocalNode_Mac;
	send_pack.rev_delay_time = 0;
	send_pack.rev_Rssi = 0;
	send_pack.Load_length = sizeof(Global_SensorDataBuff);
	rt_memcpy(send_pack.Load,&Global_SensorDataBuff,sizeof(Global_SensorDataBuff));
	// 发送数据包
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;
}

#endif