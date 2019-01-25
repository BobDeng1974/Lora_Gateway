#include "sys.h"
#include "radio.h"
#include "lora_network.h"
#include "stdio.h"
#include "delay.h"

#if defined(USED_LORANETWORK_NODE)

LoraNode_Data Global_SensorDataBuff;	//ȫ�ִ�����������


/**
 * lora ���ؽڵ�  ע����������
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
	
	// ������������
	LoraNode_SendRegPack(Radio,node_info);
	// �ȴ�ע����Ӧ��Ϣ
	do{
		if(Radio->Process()==RF_TX_DONE)
			Radio->StartRx();
		if(Radio->Process()==RF_RX_DONE)
		{
			Radio->GetRxPacket(Buffer, (uint16_t*)&buffer_size);
			//������յ������ݰ������ϼ��������ݰ���С
			if(buffer_size != sizeof(LoraNetwork_Gateway_Pack))
				continue;
			else
			{
				//��ԭ���������ݰ�
				lora_memcpy(&rev_pack,Buffer,sizeof(LoraNetwork_Gateway_Pack));
				//���Էǹ㲥���ݰ�
				if(rev_pack.node_id != 0)
					continue;
				//���Բ���ע����Ӧ�����ݰ�
				if(rev_pack.pack_type != LORA_REGACK)
					continue;
				//��ȡע����Ϣ,������ȷ��־
				if(LoraNode_RegAck(&rev_pack,node_info) == LORA_OK)
					return LORA_OK;
		}
		delay_ms(100);
	}while(timeout<100);
	//��ʱ���ش���״̬
	return LORA_SYSTEM_ERROR;
}


/**
 * lora ���ؽڵ�  ����ע����������
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendRegPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// ���������Ч��
	if(Radio==NULL || node_info->LocalNode_id != 0)
		return LORA_SYSTEM_ERROR;
	// ��װע�����������
	send_pack.node_id = LORA_BROADCAST_ID;	//�㲥����
	send_pack.pack_type = LORA_REG;					//ע������
	send_pack.node_mac = node_info->LocalNode_Mac;
	// �������ݰ�
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;
}


/**
 * lora ���ؽڵ�  ����ע����Ӧ
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_RegAck(LoraNetwork_Gateway_Pack *rev_pack,LocalNode_INFO *node_info)
{
	uint8_t temp[2];
	
	// ����Ѿ�ע������ֱ�ӷ���
	if(node_info->LocalNode_id != 0)
		return LORA_OK;
	// ��ȡ������Ϣ
	temp[0] = rev_pack->Load[0];	//��ȡĿ�Ľڵ�MAC
	temp[1] = rev_pack->Load[1];  //��ȡ���������node_id
	// �ж��Ƿ�Ϊ���ڵ�����node_id
	if(temp[0] != node_info->LocalNode_Mac)
		return LORA_DATA_ERROR;
	node_info->LocalNode_id = temp[1];
	
	return LORA_OK;
}

/**
 * lora ���ؽڵ�  ����һ����Ӧ
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendAckPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// ���������Ч��
	if(Radio==NULL || node_info->LocalNode_id == 0)
		return LORA_NODE_OFFLINE;
	// ��װһ����Ӧ��
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LOEA_ACK;
	send_pack.node_mac = node_info->LocalNode_Mac;
	// �������ݰ�
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;	
}

Lora_Sataus LoraNode_SendDataPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// ���������Ч��
	if(Radio==NULL || node_info->LocalNode_id == 0)
		return LORA_NODE_OFFLINE;
	// ��װ�������ݰ�
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LORA_NODE_UPLOAD;
	send_pack.node_mac = node_info->LocalNode_Mac;
	send_pack.rev_delay_time = 0;
	send_pack.rev_Rssi = 0;
	send_pack.Load_length = sizeof(Global_SensorDataBuff);
	rt_memcpy(send_pack.Load,&Global_SensorDataBuff,sizeof(Global_SensorDataBuff));
	// �������ݰ�
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;
}

#endif