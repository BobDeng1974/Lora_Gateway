# Readme

### GPIO映射表

| GPIO | 分配项目  | 备注 |
| ---- | --------- | ---- |
| PB3  | BUTTON1   |      |
| PB4  | BUTTON2   |      |
| PB5  | LED_R     |      |
| PB6  | LED_G     |      |
| PB7  | LED_B     |      |
| PB8  | BEEP      |      |
|      |           |      |
| PA2  | USART2_TX |      |
| PA3  | USART2_RX |      |







+ LED_RGB亮度不足，更换限流电阻
+ 修改cJSON.c   malloc/free  为rtthread提供的  rt_malloc/rt_free      47/48
+ 