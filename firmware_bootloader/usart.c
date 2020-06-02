#include <string.h>
#include "zboot_misc.h"
#include "usart.h"
#include "iap.h"
#include "cli.h"
#include "xprintf.h"

#define MAX_LEN 1280

#define USARTx USART1

static struct {
    unsigned char msg[MAX_LEN];
    int size;
    int nocomm;
} g;

void USART_Config(void) {
#if defined (GD32F350) || defined (GD32F130_150) || defined (GD32F330)
    RCU_REG_VAL(RCU_USART1) |= BIT(RCU_BIT_POS(RCU_USART1));
    RCU_REG_VAL(RCU_GPIOA) |= BIT(RCU_BIT_POS(RCU_GPIOA));
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_2 | GPIO_PIN_3);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2 | GPIO_PIN_3);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_2);
    usart_deinit(USARTx); /* USART configure */
    USART_CTL0 (USARTx) = 0x0000000c;
    USART_CTL1 (USARTx) = 0x00000000;
    USART_BAUD (USARTx) = 0x00000010;
    USART_CTL0(USARTx) |= USART_CTL0_UEN; //usart_enable(USARTx);
#else

#if defined (STM32F303xC)
    RCC->APB2ENR |= RCC_APB2Periph_USART1;
    RCC->AHBENR |= RCC_AHBPeriph_GPIOB;

    GPIOB->OSPEEDR |= (GPIO_Speed_50MHz << (6 * 2)) | (GPIO_Speed_50MHz << (7 * 2));
    GPIOB->OTYPER |= (GPIO_OType_PP << (6 * 2)) | (GPIO_OType_PP << (7 * 2));
    GPIOB->MODER |= (GPIO_Mode_AF << (6 * 2)) | (GPIO_Mode_AF << (7 * 2));
    GPIOB->PUPDR |= (GPIO_PuPd_NOPULL << (6 * 2)) | (GPIO_PuPd_NOPULL << (7 * 2));
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_7);

//#elif 1
#elif defined (STM32F401xx) || defined (STM32F40_41xxx)
    RCC->APB2ENR |= RCC_APB2Periph_USART1;
    RCC->AHB1ENR |= RCC_AHB1Periph_GPIOA;

    GPIOA->OSPEEDR |= (GPIO_Speed_100MHz << (9 * 2)) | (GPIO_Speed_100MHz << (10 * 2));
    GPIOA->OTYPER |= (GPIO_OType_PP << (9 )) | (GPIO_OType_PP << (10));
    GPIOA->MODER |= (GPIO_Mode_AF << (9 * 2)) | (GPIO_Mode_AF << (10 * 2));
    GPIOA->PUPDR |= (GPIO_PuPd_NOPULL << (9 * 2)) | (GPIO_PuPd_NOPULL << (10 * 2));
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

#elif defined (STM32F072) || defined (STM32F042) || defined (STM32F030)
    RCC->APB2ENR |= RCC_APB2Periph_USART1;
    RCC->AHBENR |= RCC_AHBPeriph_GPIOA;

    GPIOA->OSPEEDR |= (GPIO_Speed_50MHz << (9 * 2)) | (GPIO_Speed_50MHz << (10 * 2));
    GPIOA->OTYPER |= (GPIO_OType_PP << (9 * 2)) | (GPIO_OType_PP << (10 * 2));
//    GPIOA->MODER |= (GPIO_Mode_OUT << (9 * 2)) | (GPIO_Mode_OUT << (10 * 2));
    GPIOA->MODER |= (GPIO_Mode_AF << (9 * 2)) | (GPIO_Mode_AF << (10 * 2));
    GPIOA->PUPDR |= (GPIO_PuPd_NOPULL << (9 * 2)) | (GPIO_PuPd_NOPULL << (10 * 2));
    GPIOA->AFR[1] = 0x00000110UL;// PA9 & 10 -> GPIO_AF_0

#elif defined (STM32F10X_HD) || defined (STM32F10X_MD_VL)


#if defined (STM32F10X_USART1_FORK)
    /* USE pa9, pa10, usart1 */
    RCC->APB2ENR |= RCC_APB2Periph_USART1;
    RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
    //RCC->APB2ENR |= RCC_APB2Periph_AFIO;
    
    GPIOA->CRH &= ~(0xf << ((9-8)*4));
    GPIOA->CRH &= ~(0xf << ((10-8)*4));
    GPIOA->CRH |= ((GPIO_Mode_AF_PP & 0xf) | GPIO_Speed_50MHz)  << ((9-8) *4); /* PA9-> AFPP */
    GPIOA->CRH |= (GPIO_Mode_IPU&0xf) << ((10-8) *4); /* PA10-> Input pull up */
#else
    RCC->APB1ENR |= RCC_APB1Periph_USART2;
    RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
    RCC->APB2ENR |= RCC_APB2Periph_AFIO;

    GPIOA->CRL &= ~(0xf << ((2)*4));
    GPIOA->CRL &= ~(0xf << ((3)*4));
    GPIOA->CRL |= (((GPIO_Mode_AF_PP&0xf) | GPIO_Speed_50MHz) << ((2) *4));
    GPIOA->CRL |= (((GPIO_Mode_IPU&0xf)) << ((3) *4));
#endif	/* STM32F10X_USART1_FORK */
#endif	/* STM32F10X_HD */
    USARTx->CR1 &= ~USART_CR1_UE; // stop
    USARTx->CR1 |= (USART_Mode_Tx | USART_Mode_Rx);
#if defined (STM32F10X_USART1_FORK)
    USARTx->BRR = 69;	// 8M / 115200 = 69.4
#else
#if !defined (STM32F401xx) && !defined(STM32F40_41xxx)
    USARTx->BRR = 16; // 8M / 16 = 500k
#else
    USARTx->BRR = 32; // 16M / 500k = 32
#endif
#endif  /* STM32F10X_USART1_FORK */

#if !defined (STM32F10X_HD) && !defined (STM32F401xx) && !defined (STM32F10X_MD_VL) && !defined (STM32F40_41xxx)
//    USARTx->CR2 |= USART_CR2_SWAP;	// comment or uncomment this line if needed
#endif
    USARTx->CR1 |= USART_CR1_UE;

#endif	/* GD32F350 */
    g.nocomm = 1;
    xdev_out(uputc);
//    while(1) { xprintf("Hello, world.\n"); ( {  for(volatile int i = 0; i < 100000UL; i++);}); }	// for debug
}

int USART_NoComm(void) {
    return g.nocomm;
}

void USART_Poll(void) {
    if (usart_flag_get1(USARTx, USART_FLAG_RXNE)) {
        if (g.size < MAX_LEN) {
#if defined(GD32F350) || defined (GD32F130_150) || defined (GD32F330)
            g.msg[g.size] = USART_RDATA(USARTx);
#elif defined(STM32F303xC) || defined (STM32F072) || defined (STM32F030) || defined (STM32F042)
            g.msg[g.size] = USARTx->RDR;
#elif defined(STM32F10X_HD) || defined (STM32F401xx) || defined (STM32F10X_MD_VL) || defined (STM32F40_41xxx)
            g.msg[g.size] = USARTx->DR;
#endif

            g.size++;
        }
        usart_flag_clear1(USARTx, USART_FLAG_RXNE);
    }
    if (usart_flag_get1(USARTx, USART_FLAG_IDLE)) {
        if(0) {
        }
#if _USE_CLI
        else if (strncasecmp((char*) g.msg, "##", 2) == 0) {
            CLI_Parse(g.msg, g.size);
            g.nocomm = 0;
        }
#endif
        else if (g.msg[0] >= 0x80) {
            IAP_Parse(g.msg, g.size);
            g.nocomm = 0;
        }
        g.size = 0;
#if defined(STM32F10X_HD) || defined (STM32F10X_MD_VL)
        volatile unsigned long tmp = tmp;
        tmp = USARTx->SR;
        tmp = USARTx->DR;
#else
        usart_flag_clear1(USARTx, USART_FLAG_IDLE);
#endif
    }
    if (usart_flag_get1(USARTx, USART_FLAG_ORE)) {
#if defined(STM32F10X_HD) || defined (STM32F10X_MD_VL)
        volatile unsigned long tmp = tmp;
        tmp = USARTx->SR;
        tmp = USARTx->DR;
#else
        usart_flag_clear1(USARTx, USART_FLAG_ORE);
#endif
    }
    if (usart_flag_get1(USARTx, USART_FLAG_FE)) {
#if defined(STM32F10X_HD) || defined (STM32F10X_MD_VL)
        volatile unsigned long tmp = tmp;
        tmp = USARTx->SR;
        tmp = USARTx->DR;
#else
        usart_flag_clear1(USARTx, USART_FLAG_FE);
#endif
    }
}

void uwrite(const void *data, int size) {
    while (size--)
        uputc(*(unsigned char*) data++);
}

void uputc(unsigned char c) {
    ( {  while(RESET == usart_flag_get1(USARTx, USART_FLAG_TXE));});
#if defined (GD32F350) || defined (GD32F130_150) || defined (GD32F330)
    USART_TDATA (USARTx) = (USART_TDATA_TDATA & c);
#elif defined (STM32F303xC) || defined (STM32F072) || defined (STM32F030) || defined (STM32F042)
    USARTx->TDR = c;
#elif defined (STM32F10X_HD) || defined (STM32F401xx) || defined (STM32F10X_MD_VL) || defined (STM32F40_41xxx)
    USARTx->DR = c;
#endif
    ( {  while(RESET == usart_flag_get1(USARTx, USART_FLAG_TC));});
}
