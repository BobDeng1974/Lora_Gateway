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

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
}  
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART2->SR&0X40)==0);//ѭ������,ֱ���������   
	USART2->DR = (u8) ch;      
	return ch;
}
#endif

#if EN_USART2_RX   //���ʹ���˽���
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
char USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
u8 ESP_RX_FLAG=0;       //����״̬���
u8 UASRT2_RX_BUFFER_LEN=0;  //��Ч�ַ�����
#endif

/**
 * ��ʼ������2
 * @param   bound[������]
 * @return 
 * @brief 
 **/
void uart2_init(u32 bound)
{
  //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
 	
	//USART2�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA2��PA3
	
	//����2��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3����ΪUSART2

  //USART2 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������1
	
	//ʹ�ܴ���2
	USART_Cmd(USART2, ENABLE);  
	
	//�����־λ
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
#if EN_USART2_RX	
	
	//��������ж�
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	//Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��NVIC�Ĵ���

#endif
		
}

/**
 * ������д��һ����������
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
 * ��մ��ڽ��ջ�������
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
 * ��ȡUSART2�½��յ����ݳ���
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
		result = UASRT2_RX_BUFFER_LEN - Current_RX_Len;	//�½��ճ���
		Current_RX_Len = UASRT2_RX_BUFFER_LEN;			//�����³���
	}
	
    return result;
}

/**
 * ��������2��������
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
 * ����2�����ж�
 * @param   
 * @return 
 * @brief ���Ͻ��մ�������,ֱ�����������ճ����Զ�����
 **/
void USART2_IRQHandler(void)                	//����2�жϷ������
{
	u8 res;
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�
	{
		res = USART2->DR; //��ȡ���յ�������
		//�����յ����ַ����뻺������
		USART_RX_BUF[UASRT2_RX_BUFFER_LEN++] = res;
		if(UASRT2_RX_BUFFER_LEN >= USART_REC_LEN-1)
		{
			UASRT2_RX_BUFFER_LEN = 0;
		}
	}
}

