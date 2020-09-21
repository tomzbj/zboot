#ifndef _ZBOOT_MISC_H
#define _ZBOOT_MISC_H

#include "port.h"

#define _USE_EEPROM 1
#define _USE_CLI 1

// choose one 
#define _USE_USART1  0
#define _USE_USART2  1
#define _USE_USART3  0
#define _USE_USART4  0   // only available on stm32f0
#define _USE_UART4  0    // on stm32f1, f3, f4
#define _USE_UART5  0    

// choose one 
#define _USE_GPIOA   1
#define _USE_GPIOB   0
#define _USE_GPIOC   0
#define _USE_GPIOD   0

#define _USART_TXPIN 2
#define _USART_RXPIN 3

#define _GPIO_AF_TXPIN GPIO_AF_1
#define _GPIO_AF_RXPIN GPIO_AF_1

#define _USART_PIN_SWAP 1   // NOT available on stm32 f1/f4!

#endif
