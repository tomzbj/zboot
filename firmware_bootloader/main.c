#include "misc.h"
#include "usart.h"
#include "iap.h"
#include "xprintf.h"
#include "flash_eeprom.h"
#include <stdio.h>

//static int flag_jump;

void SystemInit(void)
{
#if defined (GD32F350)
    RCU_APB2EN = BIT(0);
    RCU_CTL0 |= RCU_CTL0_IRC8MEN; // enable IRC8M
    ( {  while(RESET == (RCU_CTL0 & RCU_CTL0_IRC8MSTB));}); // reset RCU
    RCU_CFG0 = 0x00000000;
    RCU_CFG1 = 0x00000000;
    RCU_CFG2 = 0x00000000;
    RCU_CTL0 &= ~(RCU_CTL0_HXTALEN | RCU_CTL0_CKMEN | RCU_CTL0_PLLEN
            | RCU_CTL0_HXTALBPS);
    RCU_CTL1 &= ~RCU_CTL1_IRC28MEN;
    RCU_ADDCTL &= ~RCU_ADDCTL_IRC48MEN;
    RCU_INT = 0x00000000U;
    RCU_ADDINT = 0x00000000U;
#elif defined (STM32F303xC) || defined (STM32F072) || defined (STM32F030) || defined (STM32F10X_HD)|| defined (STM32F401xx)
    RCC_DeInit();
//    FLASH_PrefetchBufferCmd(ENABLE);
#if defined (STM32F401xx)
    FLASH->ACR |= FLASH_ACR_PRFTEN;
#else
    FLASH->ACR |= FLASH_ACR_PRFTBE;
#endif
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
#endif

#if defined (GD32F350)
    SCB->VTOR = NVIC_VECTTAB_FLASH;
#elif defined (STM32F303xC) | defined (STM32F10X_HD) | defined (STM32F401xx)
    SCB->VTOR = NVIC_VectTab_FLASH;     // invalid on cortex-m0
#endif

}

static int flag_jump = 0;

int main(void)
{
    SystemInit();
    SysTick_Config(8000000UL); // delay 1s and then jump to app
    IAP_Config();
    USART_Config();
    IAP_Sysinfo_t* inf = IAP_GetInfo();
    FLASH_EEPROM_Config(inf->eeprom_base, FLASH_PAGE_SIZE);
#if defined (GD32F350) || defined (STM32F303xC) || defined (STM32F10X_HD)
    *(unsigned long*)0xe000ed24 = 0x00070000; // enable usage fault
#endif
    while(1) {
        USART_Poll();
        if(flag_jump)
            IAP_JumpToApp();
    }
}

void SysTick_Handler(void)
{
    if(IAP_IsAppValid() && USART_NoComm()) {
        flag_jump = 1;      // do not simply jump from here, use a flag instead
    }
}

void BusFault_Handler(void)
{
#if defined (GD32F350) || defined (STM32F303xC) || defined (STM32F10X_HD)
    if(SCB->CFSR & 0x0200) {
        xprintf("Read error @ %08lx\n", (unsigned long)(SCB->BFAR));
    }
#endif
    NVIC_SystemReset();
    while(1)
        ;
}
void _init(void)
{
}
