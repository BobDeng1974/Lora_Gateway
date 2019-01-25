#include "lora_network.h"
#include "radio.h"
#include "rtthread.h"
#include "stdio.h"
#include "debug.h"
#include "delay.h"

#if defined(USED_LORANETWORK_GATEWAY)

Lora_Node *Global_NodeList = NULL;								//ȫ�ֽڵ��豸ע���б�
LoraNode_Data Global_SensorData[LORA_NODE_MAX];		//ȫ�ֽڵ㴫����������
LoraGateway_Status Global_GatewayStatus;					//ȫ�ֽڵ���ѯ����

/**
 * lora gateway lora�շ�״̬����
 * @param   
 * @return 
 * @brief 
 **/
void LoraNetowrk_Gateway_Process(tRadioDriver *Radio)
{
	uint8_t lora_status;
	uint8_t lora_buffer[100];
	uint16_t lora_buffersize;
	LoraNetwork_Node_Pack rev_pack;
	
	// ��ʼ��ע������
	lora_status = LoraNode_Init(&Global_NodeList);
	if(lora_status != LORA_OK)
	{
		logging_error("Node_Init_ERROR!");
	}
	// ��������ע��֪ͨ
	LoraGateway_SendReregPack(Radio);
	while(1)
	{
		lora_status = Radio->Process();
		// ��lora�������ݳɹ����������ؽ���״̬
		if(lora_status == RF_TX_DONE)
		{
			logging_debug("TX DONE");
			Radio->StartRx();
		}
		// ��lora���յ�����
		if(lora_status == RF_RX_DONE)
		{
			//��ȡlora��������
			Radio->GetRxPacket(lora_buffer,&lora_buffersize);
			logging_debug("RX DONE,size=%d",sizeof(LoraNetwork_Node_Pack));
			if(lora_buffersize != sizeof(LoraNetwork_Node_Pack))
			{
				Radio->StartRx();
				continue;
			}
			else
			{
				//��ȡ�ڵ����ݰ�
				lora_memcpy(&rev_pack,lora_buffer,sizeof(LoraNetwork_Node_Pack));
			}
			//�ڵ����ݰ���������
			LoraNetowrk_NodePack_Resolver(Radio,&rev_pack);
		}
		rt_thread_delay(10);
	}
}

/**
 * �ڵ����ݰ�����
 * @param   
 * @return 
 * @brief 
 **/
void LoraNetowrk_NodePack_Resolver(tRadioDriver *Radio,LoraNetwork_Node_Pack* rev_pack)
{
	// �жϽ��յ������ݰ�����
	switch(rev_pack->pack_type)
	{
		// ���յ��ڵ�ע������
		case LORA_REG:
		{
			logging_debug("�ڵ�����ע��");
			LoraNode_Reg(Radio,rev_pack);
		}break;
		// ���յ��ڵ�һ����Ӧ
		case LOEA_ACK:
		{
			logging_debug("�յ��ڵ�һ����Ӧ");
			LoraNode_AckResole(rev_pack);
		}break;
		// ���յ��ڵ㴫�������ݰ�
		case LORA_NODE_UPLOAD:
		{
			logging_debug("�յ��ڵ㴫������Ϣ");
			//�������������ռ�����״̬�������ݰ����н���
			if(Global_GatewayStatus.gateway_status != GATEWAY_STATUS_REV)
				return;
			LoraNode_GetSensorData(rev_pack);
		}break;
	}
}

/**
 * ��������������ע��㲥��
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraGateway_SendReregPack(tRadioDriver *Radio)
{
	LoraNetwork_Gateway_Pack pack;
	
	//������ݰ�
	pack.node_id = 0;		//Ϊ�㲥��
	pack.pack_type = LORA_REREG;
	//�������ݰ�
	Radio->SetTxPacket(&pack,sizeof(pack));
}

/**
 * �����������ϴ���������㲥��
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraGateway_UploadBroadcast(tRadioDriver *Radio,Lora_Node *node_list)
{
	LoraNetwork_Gateway_Pack pack;
	Lora_Node *p;
	uint8_t i,node_num;		//��ǰ�ڵ�����
	uint8_t node_datalist[20]; 
	
	//���ɽڵ������ϴ�����
	p = node_list;	//ָ��ע������ͷָ��
	i = 0;
	do{
		//�����׽ڵ�
		if(p->node_mac == 0)
		{
			p = p->pNext;
			continue;
		}
		node_datalist[i] = p->node_mac;
		i++;
		p = p->pNext;
	}while(p == NULL);
	node_datalist[i] = 0;	//��0Ϊ��Ч������־
	//���Ƶ�������״̬��Ϣ��
	lora_memcpy(Global_GatewayStatus.node_list,node_datalist,i);	//���������ϴ������
	//������ݰ�
	pack.node_id = 0;										//Ϊ�㲥��
	pack.pack_type = LORA_NODE_UPLOAD;	//Ϊ�������ݹ㲥��
	pack.Load_length = i-1;	//��ǰҪ�����ݵĽڵ���
	lora_memcpy(pack.Load,node_datalist,i);	//���������ϴ������
	//���¼�����״̬Ϊ�ռ�����״̬
	Global_GatewayStatus.gateway_status = GATEWAY_STATUS_REV;
	//���µ�ǰ�ڵ���
	Global_GatewayStatus.node_num = i-1;
	//����ȡ���ݽڵ�������
	Global_GatewayStatus.upload_node_num = 0;
	//�������ݰ�
	Radio->SetTxPacket(&pack,sizeof(pack));
}

/**
 * lora ���մ��ڽ�������
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraOvertime_Event(void)
{
	uint8_t i,j,k;
	uint8_t unusualnode_list[10];
	
	//����ѽ������ݵĽڵ㲻���ڵ�ǰ�ڵ���
	if(Global_GatewayStatus.upload_node_num != Global_GatewayStatus.node_num)
	{
		Global_GatewayStatus.gateway_status = GATEWAY_STATUS_IDLE;
		return LORA_DATA_ERROR;
	}
	else
	{
		Global_GatewayStatus.gateway_status = GATEWAY_STATUS_IDLE;
		return LORA_OK;
	}
}

/**
 * lora �ڵ���ȡ���ݰ��д�������Ϣ
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_GetSensorData(LoraNetwork_Node_Pack* rev_pack)
{
	uint8_t node_id;
	LoraNode_Data* DataBuff;
	// �����Ч���س��Ȳ����ڴ��������ݽṹ���С,�������ݴ���
	if(rev_pack->Load_length != sizeof(LoraNode_Data))
		return LORA_DATA_ERROR;
	// ��������
	rt_memcpy(&Global_SensorData[rev_pack->node_id],rev_pack->Load,sizeof(LoraNode_Data));
	// ����ȡ�ڵ�����1
	Global_GatewayStatus.upload_node_num += 1;
	return LORA_OK;
}

/**
 * lora�ڵ�һ����Ӧ����
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_AckResole(LoraNetwork_Node_Pack* rev_pack)
{
	Lora_Node* node;
	
	node = LoraNode_Find(Global_NodeList,rev_pack->node_id);
	switch(node->node_status)
	{
		case NODE_STATUS_REG:
		{
			node->node_status = NODE_STATUS_ONLINE;
		}break;
	}
	
}

/**
 * lora�ڵ�ע�ᴦ��
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_Reg(tRadioDriver *Radio, LoraNetwork_Node_Pack* rev_pack)
{
	uint8_t status;
	Lora_Node* Node_New;
	LoraNetwork_Gateway_Pack Pack_Structure;
	
	// ���ҵ�ǰ�ڵ��Ƿ��Ѵ��ڴ˽ڵ�ע����Ϣ
	Node_New = LoraNode_Find(Global_NodeList,rev_pack->node_mac);
	if(Node_New != NULL) 
	{
		logging_warning("�ڵ��Ѿ�ע��");
		status = LORA_NODE_REGISTERED;
	}
	else
	{
		// �ڽڵ�ע���б������ڵ�
		Node_New = LoraNode_Append(Global_NodeList,rev_pack->node_mac);
		if(Node_New == NULL)  logging_warning("NEW NODE REG ERROR!");
		// ��ʼ���½ڵ�Ǽ���Ϣ
		// �ݲ�֧����ONENETƽ̨���Զ�ע��,��ΪĬ����ONENETע��������豸
		if(Node_New->node_mac == 0x80)
		{
			logging_warning("Node 0x80 sign in");
			sprintf(Node_New->device_id,"514914374");		//ONENET �ڵ�1�豸ID
			sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET �ڵ�1�豸��Կ
			sprintf(Node_New->node_name,"node1");				//ONENET �ڵ��豸����
			Node_New->node_status = NODE_STATUS_REG;	  //�豸״̬��Ϊע��״̬
		}
		else if(Node_New->node_mac == 0x81)
		{
			logging_warning("Node 0x81 sign in");
			sprintf(Node_New->device_id,"514914374");		//ONENET �ڵ�1�豸ID
			sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET �ڵ�1�豸��Կ
			sprintf(Node_New->node_name,"node2");				//ONENET �ڵ��豸����
			Node_New->node_status = NODE_STATUS_REG;	  //�豸״̬��Ϊע��״̬
		}
		else if(Node_New->node_mac == 0x82)
		{
			logging_warning("Node 0x82 sign in");
			sprintf(Node_New->device_id,"514914374");		//ONENET �ڵ�1�豸ID
			sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET �ڵ�1�豸��Կ
			sprintf(Node_New->node_name,"node3");				//ONENET �ڵ��豸����
			Node_New->node_status = NODE_STATUS_REG;	  //�豸״̬��Ϊע��״̬
		}
		status = LORA_OK;
	}
	// ����ע����Ӧ���ݰ�
	Pack_Structure.node_id = rev_pack->node_mac;  																			//Ŀ�Ľڵ����
	Pack_Structure.pack_type = LORA_REGACK;  																						//���ݰ�����Ϊע����Ӧ��
	Pack_Structure.Load_length = 6;														//���ز��ֵ���Ч�ֽ�Ϊ�豸���ƴ�С
	lora_memcpy(Pack_Structure.Load,Node_New->node_name,6);		//����Ϊ�豸����
	// ����ע����Ӧ���ݰ�
	Radio->SetTxPacket(&Pack_Structure,sizeof(LoraNetwork_Gateway_Pack));
	return status;
}

/**
 * ��ʼ��lora�ڵ���Ϣ����
 * @param   
 * @return 
 * @brief ͷ��㲻�����κ�һ�������ڵ㣬���������ע��ڵ�
 **/
Lora_Sataus LoraNode_Init(Lora_Node** pHead)
{
	// ���׽ڵ�����ڴ�ռ�
	*pHead = (Lora_Node*)rt_malloc(sizeof(Lora_Node));
	// ����ռ�ʧ���򷵻�ϵͳ����
	if(*pHead == NULL)
		return LORA_SYSTEM_ERROR;
	// ��ʼ��ͷ�ڵ���Ϣ
	(*pHead)->node_mac = 0;
	sprintf((*pHead)->node_name,"all");
	(*pHead)->node_status = 0;
	(*pHead)->pNext = NULL;
	
	return LORA_OK;
}

/**
 * �ڽڵ���Ϣ���������ڵ��豸
 * @param   pHead[�ڵ��豸��Ϣ����ͷָ��]
 * @return  Lora_Sataus[LORAִ�����]
 * @brief 
 **/
Lora_Node* LoraNode_Append(Lora_Node* pHead,uint8_t node_mac)
{
	Lora_Node* r = pHead;
	Lora_Node* pNew = (Lora_Node*)rt_malloc(sizeof(Lora_Node));
	if(pNew == NULL)
		return NULL;
	// ��βָ��ѭ������ǰ���һ���ڵ�
	while(r->pNext != NULL)
		r = r->pNext;
	// �������Ľڵ�ָ��ӵ�βָ�벢����ڵ����к�
	pNew->node_mac = node_mac;
	r->pNext = pNew;
	
	return pNew;
}

/**
 * ��ѯ����ȡע������ĳһ���
 * @param   Lora_Node*[ע���б�ͷָ��]  node_mac[�ڵ�ΨһӲ����ʶ]
 * @return  Lora_Node[���ҳɹ�����Ŀ�Ľڵ�ָ��,���򷵻�NULL]
 * @brief 
 **/
Lora_Node* LoraNode_Find(Lora_Node* pHead, uint8_t node_mac)
{
	Lora_Node *p,*r;
	if(pHead == NULL)
		return NULL;
	r = pHead;
	while(r->node_mac != node_mac && r->pNext != NULL)
	{
		r = r->pNext;
	}
	if(r->node_mac == node_mac)
		return r;
	else
		return NULL;	
}

/**
 * �ڽڵ���Ϣ����ɾ���ڵ��豸
 * @param   pHead[�ڵ��豸��Ϣ����ͷָ��]  node_id[�ڵ��豸���к�]
 * @return  Lora_Sataus[LORAִ�����]
 * @brief 
 **/
Lora_Sataus LoraNode_Delete(Lora_Node* pHead, uint8_t node_mac)
{
	Lora_Node *pb,*pf; 
	pb = pHead;
	if(pHead == NULL)
		return LORA_SYSTEM_ERROR;
	// ѭ������Ҫɾ���Ľ��
	while(pb->node_mac != node_mac && pb->pNext != NULL)
	{
		pf = pb;
		pb = pb->pNext;
	}
	// ����ҵ�Ҫɾ���Ľ��
	if(pb->node_mac == node_mac)
	{
		if(pb == pHead)
		{
			return LORA_SYSTEM_ERROR;
		}
		else
		{
			pf->pNext = pb->pNext;
			rt_free(pb);
		}
	}
	else
	{
		return LORA_SYSTEM_ERROR;
	}
	
	return LORA_OK;
}

#endif



