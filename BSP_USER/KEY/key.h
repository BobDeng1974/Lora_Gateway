#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 


/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
//#define KEY0 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3) //PE4
//#define KEY1 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)	//PE3 

/*���淽ʽ��ͨ��λ��������ʽ��ȡIO*/

#define KEY0 		PBin(3)   	//PE4
#define KEY1 		PBin(4)			//PE3 



void KEY_Init(void);	//IO��ʼ��

#endif
