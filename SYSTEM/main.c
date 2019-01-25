/**
  ******************************************************************************
  * @file    STM32F407DEMO/main.c 
  * @author  Liang
  * @version V1.0.0
  * @date    2017-4-26
  * @brief   Main program body
  ******************************************************************************
  * @attention
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "sys.h"
#include "led.h"
#include "delay.h"
#include "usart2.h"
#include "esp8266.h"
#include "rtthread.h"
#include "stdio.h"
#include "radio.h"
#include "spi.h"
#include "app.h"
#include "key.h"
#include "lora_network.h"
#include "debug.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main()
{
	tRadioDriver *Radio = NULL;
	
	delay_init(168);
	uart2_init(9600);
	LED_Init();
	SpiInit();
	Main_APP();
}


