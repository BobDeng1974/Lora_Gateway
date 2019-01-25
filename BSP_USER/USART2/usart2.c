/**
  * @file   usart2.c
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-6
  * @brief	
  **/

/* include-------------------------------------------------- */
#include "sys.h"
#include "usart2.h"
#include "rtthread.h"
#include "stdio.h"

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
}  
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕   
	USART2->DR = (u8) ch;      
	return ch;
}
#endif

#if EN_USART2_RX   //如果使能了接收
//注意,读取USARTx->SR能避免莫名其妙的错误   	
char USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
u8 ESP_RX_FLAG=0;       //接收状态标记
u8 UASRT2_RX_BUFFER_LEN=0;  //有效字符长度
#endif

/**
 * 初始化串口2
 * @param   bound[波特率]
 * @return 
 * @brief 
 **/
void uart2_init(u32 bound)
{
  //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
 	
	//USART2端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA2，PA3
	
	//串口2对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3复用为USART2

  //USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART2, &USART_InitStructure); //初始化串口1
	
	//使能串口2
	USART_Cmd(USART2, ENABLE);  
	
	//清除标志位
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
#if EN_USART2_RX	
	
	//开启相关中断
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化NVIC寄存器

#endif
		
}

/**
 * 往串口写入一定长度数据
 * @param   
 * @return 
 * @brief 
 **/
void USART2_Write(char *Data, uint8_t len)
{
    uint8_t i;

    USART_ClearFlag(USART2, USART_FLAG_TC);
    for(i = 0; i < len; i++)
    {
			while(RESET == USART_GetFlagStatus(USART2,USART_FLAG_TXE));
      USART_SendData(USART2, *Data++);
    }
}

/**
 * 清空串口接收缓存数组
 * @param   
 * @return 
 * @brief 
 **/
void USART2_Clear(void)
{
	ESP_RX_FLAG = 0;
	UASRT2_RX_BUFFER_LEN = 0;	
}


/**
 * 获取USART2新接收的数据长度
 * @param   
 * @return 
 * @brief 
 **/
uint32_t USART2_GetRcvNum(void)
{
	static uint32_t Current_RX_Len = 0;
	uint32_t result = 0;
	
	if(UASRT2_RX_BUFFER_LEN == 0)
	{
		Current_RX_Len = 0;
		result = 0;
	}
	else if(Current_RX_Len != UASRT2_RX_BUFFER_LEN)
	{
		result = UASRT2_RX_BUFFER_LEN - Current_RX_Len;	//新接收长度
		Current_RX_Len = UASRT2_RX_BUFFER_LEN;			//保存新长度
	}
	
    return result;
}

/**
 * 拷贝串口2接收数据
 * @param   
 * @return 
 * @brief 
 **/
void USART2_GetRcvData(char *buf, uint32_t rcv_len)
{
	if(buf)
	{
			rt_memcpy(buf, USART_RX_BUF, rcv_len);
	}
}


/**
 * 串口2接收中断
 * @param   
 * @return 
 * @brief 不断接收串口数据,直到大于最大接收长度自动清零
 **/
void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	u8 res;
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断
	{
		res = USART2->DR; //读取接收到的数据
		//将接收到的字符放入缓存数组
		USART_RX_BUF[UASRT2_RX_BUFFER_LEN++] = res;
		if(UASRT2_RX_BUFFER_LEN >= USART_REC_LEN-1)
		{
			UASRT2_RX_BUFFER_LEN = 0;
		}
	}
}

