#ifndef _LORA_NETWORK_H
#define _LORA_NETWORK_H

#include "sys.h"
#include "radio.h"

//#define USED_LORANETWORK_NODE
#define USED_LORANETWORK_GATEWAY

#define lora_memcpy rt_memcpy
#define LORA_NODE_MAX 10	//当前支持入网的最大节点数

/* ------------------LORA节点状态码------------------- */
#define NODE_STATUS_OFFLINE 0			//离线状态
#define NODE_STATUS_REG			1			//注册状态
#define NODE_STATUS_ONLINE	2			//在线状态
#define NODE_STATUS_UPDATA  3			//已更新数据状态
/* ------------------LORA网关状态码------------------- */
#define GATEWAY_STATUS_IDLE		0			//空闲状态
#define GATEWAY_STATUS_REV		1			//处于收集节点上传的数据状态
#define GATEWAY_STATUS_UPLOAD 2			//处于上传数据到云服务器状态
#if defined(USED_LORANETWORK_NODE)
/* ------------------节点端------------------- */
#define LORA_LOCAL_MAC	0x01			//本节点唯一硬件标识码
typedef struct
{
	uint8_t LocalNode_id;				//集中器分配的组网序列号
	uint8_t LocalNode_Mac;			//本设备自带的硬件识别码
	char    LocalNode_Name[20];	//集中器分配的组网设备名称
}LocalNode_INFO;
#endif




#define LOAD_LENGTH 50	//数据负载长度
#define LORA_BROADCAST_ID	0	//广播id
#define LOEA_ACK						0x00    //lora 一般响应
#define LORA_REG		        0x01		//lora node 注册请求
#define LORA_REGACK         0x02		//lora node 注册应答
#define LORA_NODE_REMOVE    0x03		//lora node 注销请求
#define LORA_NODE_REMOVEACK 0x04	  //lora node 注销确认
#define LORA_NODE_UPLOAD    0x05		//lora node 上传数据
#define LORA_NODE_LOWPOWER  0x06		//lora node 低电量报警
#define LORA_REREG					0x07		//lora gateway重新注册命令
#define LORA_UPDATA					0x08		//lora gateway更新数据请求广播


typedef enum
{
	LORA_OK = 0,
	LORA_SYSTEM_ERROR,
	LORA_DATA_ERROR,
	LORA_NODE_OFFLINE,
	LORA_NODE_REGISTERED
}Lora_Sataus;

/* 
 * 集中器数据包 
 */
typedef struct
{
	uint8_t  node_id;	    			//节点id,用于区分不同的节点
	uint8_t	 pack_type;	  			//数据包类型
	uint8_t  rev_delay_time;		//接收延迟时间
	uint32_t rev_Rssi;					//上一次接收数据包的信号质量情况
	uint8_t  Load_length;				//负载长度
	uint8_t  Load[20]; 					//数据负载	
}LoraNetwork_Gateway_Pack;

/*
 * 节点数据包
 */
typedef struct
{
	uint8_t  node_id;	    			//节点id,用于区分不同的节点
	uint8_t	 pack_type;	  			//数据包类型
	uint8_t  node_mac;					//节点唯一硬件地址
	uint8_t  rev_delay_time;		//接收延迟时间
	uint32_t rev_Rssi;					//上一次接收数据包的信号质量情况
	uint8_t  Load_length;				//负载长度
	uint8_t  Load[LOAD_LENGTH]; //数据负载	
}LoraNetwork_Node_Pack;



/*
 * lora 节点数据结构体
 */
typedef struct
{
	uint8_t  node_mac;	//数据所属节点标识
	uint16_t LX;				//光照度
	uint16_t TVOC;		  //TVOC浓度
	int32_t  temp;			//温度
	int32_t  hum;				//湿度
	int32_t  pressure;	//大气压强
}LoraNode_Data;

/*
 * lora gateway 状态信息结构体
 */
typedef struct
{
	uint8_t gateway_status;		//集中器状态
	uint8_t node_num;		  		//当前节点数量
	uint16_t event_time;			//事件时间记录
	uint8_t upload_node_num;	//当前已上传数据的节点数
	uint8_t node_list[20];		//当前节点轮询状态数组
}LoraGateway_Status;

/*
 * 节点信息链表结点结构体
 */
typedef struct lora_node
{
	uint8_t node_mac;				//节点唯一硬件地址
	char    node_name[20];	//节点设备名称
	uint8_t node_status;		//节点设备状态
	uint16_t node_rssi;			//最近一次设备的信号状况
	char device_id[50];			//云服务器分配的设备id
	char device_key[50];		//云服务器分配的设备密钥
	
	struct lora_node *pNext;	//指向下一个节点信息
	
}Lora_Node;

/* -------------------本地节点端相关---------------- */
#if defined(USED_LORANETWORK_NODE)
Lora_Sataus LoraNode_SendDataPack(tRadioDriver *Radio,LocalNode_INFO *node_info);
Lora_Sataus LoraNode_SendRegPack(tRadioDriver *Radio,LocalNode_INFO *node_info);
Lora_Sataus LoraNode_RegAck(LoraNetwork_Gateway_Pack *rev_pack,LocalNode_INFO *node_info);
#endif

#if defined(USED_LORANETWORK_GATEWAY)
/* ------------------集中器端------------------- */
/* ------------------集中器端全局变量------------------- */
extern Lora_Node *Global_NodeList;								//全局节点设备注册列表
extern LoraNode_Data Global_SensorData[LORA_NODE_MAX];		//全局节点传感器缓冲区
extern LoraGateway_Status Global_GatewayStatus;					//全局节点轮询数组
/* -------------------节点注册列表相关---------------- */
Lora_Sataus LoraNode_Init(Lora_Node** pHead);
Lora_Node* LoraNode_Append(Lora_Node* pHead,uint8_t node_mac);
Lora_Sataus LoraNode_Delete(Lora_Node* pHead, uint8_t node_id);
Lora_Node* LoraNode_Find(Lora_Node* pHead, uint8_t node_mac);
/* -------------------节点数据包处理相关---------------- */
Lora_Sataus LoraNode_Reg(tRadioDriver *Radio, LoraNetwork_Node_Pack* rev_pack);
Lora_Sataus LoraNode_AckResole(LoraNetwork_Node_Pack* rev_pack);
Lora_Sataus LoraNode_GetSensorData(LoraNetwork_Node_Pack* rev_pack);
Lora_Sataus LoraGateway_SendReregPack(tRadioDriver *Radio);
/* -------------------节点收发管理相关---------------- */
void LoraNetowrk_NodePack_Resolver(tRadioDriver *Radio,LoraNetwork_Node_Pack* rev_pack);
void LoraNetowrk_Gateway_Process(tRadioDriver *Radio);
Lora_Sataus LoraOvertime_Event(void);
#endif
#endif