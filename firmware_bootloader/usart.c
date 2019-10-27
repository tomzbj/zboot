#include <string.h>
#include "misc.h"
#include "usart.h"
#include "iap.h"
#include "cli.h"
#include "xprintf.h"

#define MAX_LEN 640

#if defined (GD32F350)
#define USARTx USART0

#elif defined (STM32F303xC)
#define USARTx USART3

#elif defined (STM32F072)
#define USARTx USART1

#elif defined (STM32F030)
#define USARTx USART1

#elif defined (STM32F10X_HD)
#define USARTx USART1

#endif

static struct {
    unsigned char msg[MAX_LEN];
    int size;
    int nocomm;
} g;

void USART_Config(void)
{
#if defined (GD32F350)
    RCU_REG_VAL(RCU_USART0) |= BIT(RCU_BIT_POS(RCU_USART0));
    RCU_REG_VAL(RCU_GPIOA) |= BIT(RCU_BIT_POS(RCU_GPIOA));
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_9 | GPIO_PIN_10);
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
    usart_deinit(USARTx); /* USART configure */
    USART_CTL0 (USARTx) = 0x0000000c;
    USART_CTL1 (USARTx) = 0x00000000;
    USART_BAUD (USARTx) = 0x00000010;
    USART_CTL0(USARTx) |= USART_CTL0_UEN; //usart_enable(USARTx);
#else

#if defined (STM32F303xC)
    RCC->APB1ENR |= RCC_APB1Periph_USART3;
    RCC->AHBENR |= RCC_AHBPeriph_GPIOB;

    GPIOB->OSPEEDR |= (GPIO_Speed_50MHz << (10 * 2)) | (GPIO_Speed_50MHz << (11 * 2));
    GPIOB->OTYPER |= (GPIO_OType_PP << (10 * 2)) | (GPIO_OType_PP << (11 * 2));
    GPIOB->MODER |= (GPIO_Mode_AF << (10 * 2)) | (GPIO_Mode_AF << (11 * 2));
    GPIOB->PUPDR |= (GPIO_PuPd_NOPULL << (10 * 2)) | (GPIO_PuPd_NOPULL << (11 * 2));
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);
#elif defined (STM32F072)
    RCC->APB2ENR |= RCC_APB2Periph_USART1;
    RCC->AHBENR |= RCC_AHBPeriph_GPIOB;

    GPIOB->OSPEEDR |= (GPIO_Speed_50MHz << (6 * 2)) | (GPIO_Speed_50MHz << (7 * 2));
    GPIOB->OTYPER |= (GPIO_OType_PP << (6 * 2)) | (GPIO_OType_PP << (7 * 2));
//    GPIOB->MODER |= (GPIO_Mode_OUT << (6 * 2)) | (GPIO_Mode_OUT << (7 * 2));
    GPIOB->MODER |= (GPIO_Mode_AF << (6 * 2)) | (GPIO_Mode_AF << (7 * 2));
    GPIOB->PUPDR |= (GPIO_PuPd_NOPULL << (6 * 2)) | (GPIO_PuPd_NOPULL << (7 * 2));
    GPIOB->AFR[0] = 0x00000000UL;// PB6 & 7 -> GPIO_AF_0

#elif defined (STM32F030)
    RCC->APB2ENR |= RCC_APB2Periph_USART1;
    RCC->AHBENR |= RCC_AHBPeriph_GPIOB;

    GPIOB->OSPEEDR |= (GPIO_Speed_50MHz << (6 * 2)) | (GPIO_Speed_50MHz << (7 * 2));
    GPIOB->OTYPER |= (GPIO_OType_PP << (6 * 2)) | (GPIO_OType_PP << (7 * 2));
//    GPIOB->MODER |= (GPIO_Mode_OUT << (6 * 2)) | (GPIO_Mode_OUT << (7 * 2));
    GPIOB->MODER |= (GPIO_Mode_AF << (6 * 2)) | (GPIO_Mode_AF << (7 * 2));
    GPIOB->PUPDR |= (GPIO_PuPd_NOPULL << (6 * 2)) | (GPIO_PuPd_NOPULL << (7 * 2));
    GPIOB->AFR[0] = 0x00000000UL;// PB6 & 7 -> GPIO_AF_0

#elif defined (STM32F10X_HD)
    RCC->APB2ENR |= RCC_APB2Periph_USART1;
    RCC->APB2ENR |= RCC_APB2Periph_GPIOA;

    GPIOA->CRH &= 0xf << ((9-8)*4);
    GPIOA->CRH &= 0xf << ((10-8)*4);
    GPIOA->CRH |= ((GPIO_Mode_AF_PP | GPIO_Speed_50MHz) << (9-8) *4);
    GPIOA->CRH |= ((GPIO_Mode_IPU| GPIO_Speed_50MHz) << (10-8) *4);
#endif
    USARTx->CR1 &= ~USART_CR1_UE; // stop
    USARTx->CR1 |= (USART_Mode_Tx | USART_Mode_Rx);
    USARTx->BRR = 16; // 8M / 500k = 16
#if !defined (STM32F10X_HD)
    USARTx->CR2 |= USART_CR2_SWAP;
#endif
    USARTx->CR1 |= USART_CR1_UE;

#endif
    g.nocomm = 1;
    xdev_out(uputc);
//    while(1) { xprintf("Hello, world.\n"); ( {  for(volatile int i = 0; i < 100000UL; i++);}); }
}

int USART_NoComm(void)
{
    return g.nocomm;
}

void USART_Poll(void)
{
    if(usart_flag_get1(USARTx, USART_FLAG_RXNE)) {
        if(g.size < MAX_LEN) {
#if defined(GD32F350)
            g.msg[g.size] = USART_RDATA(USARTx);
#elif defined(STM32F303xC) || defined (STM32F072) || defined (STM32F030)
            g.msg[g.size] = USARTx->RDR;
#elif defined(STM32F10X_HD)
            g.msg[g.size] = USARTx->DR;
#endif
            g.size++;
        }
        usart_flag_clear1(USARTx, USART_FLAG_RXNE);
    }
    if(usart_flag_get1(USARTx, USART_FLAG_IDLE)) {
        if(strncasecmp((char*)g.msg, "##", 2) == 0) {
            CLI_Parse(g.msg, g.size);
            g.nocomm = 0;
        }
        else if(g.msg[0] >= 0x80) {
            IAP_Parse(g.msg, g.size);
            g.nocomm = 0;
        }
        g.size = 0;
        usart_flag_clear1(USARTx, USART_FLAG_IDLE);
    }
    if(usart_flag_get1(USARTx, USART_FLAG_ORE)) {
        usart_flag_clear1(USARTx, USART_FLAG_ORE);
    }
}

void uwrite(const void* data, int size)
{
    while(size--)
        uputc(*(unsigned char*)data++);
}

void uputc(unsigned char c)
{
    ( {  while(RESET == usart_flag_get1(USARTx, USART_FLAG_TXE));});
#if defined (GD32F350)
    USART_TDATA (USARTx) = (USART_TDATA_TDATA & c);
#elif defined (STM32F303xC) || defined (STM32F072) || defined (STM32F030)
    USARTx->TDR = c;
#elif defined (STM32F10X_HD)
    USARTx->DR = c;
#endif
    ( {  while(RESET == usart_flag_get1(USARTx, USART_FLAG_TC));});
}
