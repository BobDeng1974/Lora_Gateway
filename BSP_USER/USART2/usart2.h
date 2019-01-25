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
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART2_RX 				1		//ʹ�ܣ�1��/��ֹ��0������1����


#if EN_USART2_RX
extern char USART_RX_BUF[USART_REC_LEN];  //���ջ���,���USART_REC_LEN���ֽ�.
extern u8 ESP_RX_FLAG;         		 //����״̬���
extern u8 UASRT2_RX_BUFFER_LEN;  //��Ч�ַ�����
#endif

void uart2_init(u32 bound);
void USART2_Clear(void);
void USART2_Write(char *Data, uint8_t len);
void sendPkt(char *p, int len);
uint32_t USART2_GetRcvNum(void);
void USART2_GetRcvData(char *buf, uint32_t rcv_len);


