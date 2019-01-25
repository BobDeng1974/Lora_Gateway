#include "lora_network.h"
#include "radio.h"
#include "rtthread.h"
#include "stdio.h"
#include "debug.h"
#include "delay.h"

#if defined(USED_LORANETWORK_GATEWAY)

Lora_Node *Global_NodeList = NULL;								//全局节点设备注册列表
LoraNode_Data Global_SensorData[LORA_NODE_MAX];		//全局节点传感器缓冲区
LoraGateway_Status Global_GatewayStatus;					//全局节点轮询数组

/**
 * lora gateway lora收发状态处理
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
	
	// 初始化注册链表
	lora_status = LoraNode_Init(&Global_NodeList);
	if(lora_status != LORA_OK)
	{
		logging_error("Node_Init_ERROR!");
	}
	// 发送重新注册通知
	LoraGateway_SendReregPack(Radio);
	while(1)
	{
		lora_status = Radio->Process();
		// 当lora发送数据成功后立即返回接收状态
		if(lora_status == RF_TX_DONE)
		{
			logging_debug("TX DONE");
			Radio->StartRx();
		}
		// 当lora接收到数据
		if(lora_status == RF_RX_DONE)
		{
			//提取lora接收数据
			Radio->GetRxPacket(lora_buffer,&lora_buffersize);
			logging_debug("RX DONE,size=%d",sizeof(LoraNetwork_Node_Pack));
			if(lora_buffersize != sizeof(LoraNetwork_Node_Pack))
			{
				Radio->StartRx();
				continue;
			}
			else
			{
				//提取节点数据包
				lora_memcpy(&rev_pack,lora_buffer,sizeof(LoraNetwork_Node_Pack));
			}
			//节点数据包解析处理
			LoraNetowrk_NodePack_Resolver(Radio,&rev_pack);
		}
		rt_thread_delay(10);
	}
}

/**
 * 节点数据包解析
 * @param   
 * @return 
 * @brief 
 **/
void LoraNetowrk_NodePack_Resolver(tRadioDriver *Radio,LoraNetwork_Node_Pack* rev_pack)
{
	// 判断接收到的数据包类型
	switch(rev_pack->pack_type)
	{
		// 接收到节点注册请求
		case LORA_REG:
		{
			logging_debug("节点请求注册");
			LoraNode_Reg(Radio,rev_pack);
		}break;
		// 接收到节点一般响应
		case LOEA_ACK:
		{
			logging_debug("收到节点一般响应");
			LoraNode_AckResole(rev_pack);
		}break;
		// 接收到节点传感器数据包
		case LORA_NODE_UPLOAD:
		{
			logging_debug("收到节点传感器信息");
			//集中器不处于收集数据状态不对数据包进行解析
			if(Global_GatewayStatus.gateway_status != GATEWAY_STATUS_REV)
				return;
			LoraNode_GetSensorData(rev_pack);
		}break;
	}
}

/**
 * 集中器发送重新注册广播包
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraGateway_SendReregPack(tRadioDriver *Radio)
{
	LoraNetwork_Gateway_Pack pack;
	
	//打包数据包
	pack.node_id = 0;		//为广播包
	pack.pack_type = LORA_REREG;
	//发送数据包
	Radio->SetTxPacket(&pack,sizeof(pack));
}

/**
 * 集中器发送上传数据请求广播包
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraGateway_UploadBroadcast(tRadioDriver *Radio,Lora_Node *node_list)
{
	LoraNetwork_Gateway_Pack pack;
	Lora_Node *p;
	uint8_t i,node_num;		//当前节点数量
	uint8_t node_datalist[20]; 
	
	//生成节点数据上传队列
	p = node_list;	//指向注册链表头指针
	i = 0;
	do{
		//跳过首节点
		if(p->node_mac == 0)
		{
			p = p->pNext;
			continue;
		}
		node_datalist[i] = p->node_mac;
		i++;
		p = p->pNext;
	}while(p == NULL);
	node_datalist[i] = 0;	//以0为有效结束标志
	//复制到集中器状态信息中
	lora_memcpy(Global_GatewayStatus.node_list,node_datalist,i);	//复制数据上传排序表
	//打包数据包
	pack.node_id = 0;										//为广播包
	pack.pack_type = LORA_NODE_UPLOAD;	//为请求数据广播包
	pack.Load_length = i-1;	//当前要传数据的节点数
	lora_memcpy(pack.Load,node_datalist,i);	//复制数据上传排序表
	//更新集中器状态为收集数据状态
	Global_GatewayStatus.gateway_status = GATEWAY_STATUS_REV;
	//更新当前节点数
	Global_GatewayStatus.node_num = i-1;
	//已提取数据节点数清零
	Global_GatewayStatus.upload_node_num = 0;
	//发送数据包
	Radio->SetTxPacket(&pack,sizeof(pack));
}

/**
 * lora 接收窗口结束处理
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraOvertime_Event(void)
{
	uint8_t i,j,k;
	uint8_t unusualnode_list[10];
	
	//如果已接收数据的节点不等于当前节点数
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
 * lora 节点提取数据包中传感器信息
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_GetSensorData(LoraNetwork_Node_Pack* rev_pack)
{
	uint8_t node_id;
	LoraNode_Data* DataBuff;
	// 如果有效负载长度不等于传感器数据结构体大小,返回数据错误
	if(rev_pack->Load_length != sizeof(LoraNode_Data))
		return LORA_DATA_ERROR;
	// 拷贝数据
	rt_memcpy(&Global_SensorData[rev_pack->node_id],rev_pack->Load,sizeof(LoraNode_Data));
	// 已提取节点数加1
	Global_GatewayStatus.upload_node_num += 1;
	return LORA_OK;
}

/**
 * lora节点一般响应处理
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
 * lora节点注册处理
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_Reg(tRadioDriver *Radio, LoraNetwork_Node_Pack* rev_pack)
{
	uint8_t status;
	Lora_Node* Node_New;
	LoraNetwork_Gateway_Pack Pack_Structure;
	
	// 查找当前节点是否已存在此节点注册信息
	Node_New = LoraNode_Find(Global_NodeList,rev_pack->node_mac);
	if(Node_New != NULL) 
	{
		logging_warning("节点已经注册");
		status = LORA_NODE_REGISTERED;
	}
	else
	{
		// 在节点注册列表新增节点
		Node_New = LoraNode_Append(Global_NodeList,rev_pack->node_mac);
		if(Node_New == NULL)  logging_warning("NEW NODE REG ERROR!");
		// 初始化新节点登记信息
		// 暂不支持在ONENET平台的自动注册,改为默认在ONENET注册的三个设备
		if(Node_New->node_mac == 0x80)
		{
			logging_warning("Node 0x80 sign in");
			sprintf(Node_New->device_id,"514914374");		//ONENET 节点1设备ID
			sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET 节点1设备密钥
			sprintf(Node_New->node_name,"node1");				//ONENET 节点设备名称
			Node_New->node_status = NODE_STATUS_REG;	  //设备状态改为注册状态
		}
		else if(Node_New->node_mac == 0x81)
		{
			logging_warning("Node 0x81 sign in");
			sprintf(Node_New->device_id,"514914374");		//ONENET 节点1设备ID
			sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET 节点1设备密钥
			sprintf(Node_New->node_name,"node2");				//ONENET 节点设备名称
			Node_New->node_status = NODE_STATUS_REG;	  //设备状态改为注册状态
		}
		else if(Node_New->node_mac == 0x82)
		{
			logging_warning("Node 0x82 sign in");
			sprintf(Node_New->device_id,"514914374");		//ONENET 节点1设备ID
			sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET 节点1设备密钥
			sprintf(Node_New->node_name,"node3");				//ONENET 节点设备名称
			Node_New->node_status = NODE_STATUS_REG;	  //设备状态改为注册状态
		}
		status = LORA_OK;
	}
	// 生成注册响应数据包
	Pack_Structure.node_id = rev_pack->node_mac;  																			//目的节点接收
	Pack_Structure.pack_type = LORA_REGACK;  																						//数据包类型为注册响应类
	Pack_Structure.Load_length = 6;														//负载部分的有效字节为设备名称大小
	lora_memcpy(Pack_Structure.Load,Node_New->node_name,6);		//负载为设备名称
	// 发送注册响应数据包
	Radio->SetTxPacket(&Pack_Structure,sizeof(LoraNetwork_Gateway_Pack));
	return status;
}

/**
 * 初始化lora节点信息链表
 * @param   
 * @return 
 * @brief 头结点不代表任何一个独立节点，其代表所有注册节点
 **/
Lora_Sataus LoraNode_Init(Lora_Node** pHead)
{
	// 给首节点分配内存空间
	*pHead = (Lora_Node*)rt_malloc(sizeof(Lora_Node));
	// 分配空间失败则返回系统错误
	if(*pHead == NULL)
		return LORA_SYSTEM_ERROR;
	// 初始化头节点信息
	(*pHead)->node_mac = 0;
	sprintf((*pHead)->node_name,"all");
	(*pHead)->node_status = 0;
	(*pHead)->pNext = NULL;
	
	return LORA_OK;
}

/**
 * 在节点信息链表新增节点设备
 * @param   pHead[节点设备信息链表头指针]
 * @return  Lora_Sataus[LORA执行情况]
 * @brief 
 **/
Lora_Node* LoraNode_Append(Lora_Node* pHead,uint8_t node_mac)
{
	Lora_Node* r = pHead;
	Lora_Node* pNew = (Lora_Node*)rt_malloc(sizeof(Lora_Node));
	if(pNew == NULL)
		return NULL;
	// 让尾指针循环到当前最后一个节点
	while(r->pNext != NULL)
		r = r->pNext;
	// 将新增的节点指针接到尾指针并分配节点序列号
	pNew->node_mac = node_mac;
	r->pNext = pNew;
	
	return pNew;
}

/**
 * 查询并提取注册链表某一结点
 * @param   Lora_Node*[注册列表头指针]  node_mac[节点唯一硬件标识]
 * @return  Lora_Node[查找成功返回目的节点指针,否则返回NULL]
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
 * 在节点信息链表删除节点设备
 * @param   pHead[节点设备信息链表头指针]  node_id[节点设备序列号]
 * @return  Lora_Sataus[LORA执行情况]
 * @brief 
 **/
Lora_Sataus LoraNode_Delete(Lora_Node* pHead, uint8_t node_mac)
{
	Lora_Node *pb,*pf; 
	pb = pHead;
	if(pHead == NULL)
		return LORA_SYSTEM_ERROR;
	// 循环查找要删除的结点
	while(pb->node_mac != node_mac && pb->pNext != NULL)
	{
		pf = pb;
		pb = pb->pNext;
	}
	// 如果找到要删除的结点
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



