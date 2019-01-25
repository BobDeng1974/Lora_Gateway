/**
  * @file    usart2.h
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-6
  * @brief	
  **/
	
/* include-------------------------------------------------- */	
#include "stm32f4xx_conf.h"
#include "sys.h" 

/* define--------------------------------------------------- */
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define EN_USART2_RX 				1		//使能（1）/禁止（0）串口1接收


#if EN_USART2_RX
extern char USART_RX_BUF[USART_REC_LEN];  //接收缓冲,最大USART_REC_LEN个字节.
extern u8 ESP_RX_FLAG;         		 //接收状态标记
extern u8 UASRT2_RX_BUFFER_LEN;  //有效字符长度
#endif

void uart2_init(u32 bound);
void USART2_Clear(void);
void USART2_Write(char *Data, uint8_t len);
void sendPkt(char *p, int len);
uint32_t USART2_GetRcvNum(void);
void USART2_GetRcvData(char *buf, uint32_t rcv_len);


