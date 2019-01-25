#ifndef __LED_H
#define __LED_H

////////////////////////////////////////////////////////////////////////////////////	

//LED端口定义
#define LEDR PBout(5)	 
#define LEDG PBout(6)	 
#define LEDB PBout(7)	  

#define PB1 PBout(3)	 
#define PB2 PBout(4)	  

//函数声明
void LED_Init(void);//初始化	

#endif
