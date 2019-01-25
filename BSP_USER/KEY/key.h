#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 


/*下面的方式是通过直接操作库函数方式读取IO*/
//#define KEY0 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3) //PE4
//#define KEY1 		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4)	//PE3 

/*下面方式是通过位带操作方式读取IO*/

#define KEY0 		PBin(3)   	//PE4
#define KEY1 		PBin(4)			//PE3 



void KEY_Init(void);	//IO初始化

#endif
