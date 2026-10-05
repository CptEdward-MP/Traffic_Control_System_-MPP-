#include "pal_time.h"
#include "stm32f411xe.h"

#define PAL_TIME_CPU_HZ      16000000U
#define PAL_TIME_TICK_HZ     1000U
#define PAL_TIME_RELOAD      ((PAL_TIME_CPU_HZ / PAL_TIME_TICK_HZ) - 1U)

static volatile uint32_t system_ms = 0U;


//=====================================================//
void PAL_Time_Tick(void)
{
    system_ms++;          // Place PAL_Time_Tick(); inside stm32f4xx_it.c , systick_handler
}
//=======================================================//
//
//
//void PAL_Clock_Init(void)
//{
//    /* Enable HSI */
//    RCC->CR |= (1U << 0);
//
//    while (!(RCC->CR & (1U << 1)))
//    {
//    }
//
//    /* Enable PWR clock */
//    RCC->APB1ENR |= (1U << 28);
//
//    /* Voltage scale 2 */
//    PWR->CR |= (1U << 15);
//
//    /* 2 Flash wait states for 84 MHz */
//    FLASH->ACR = (FLASH->ACR & ~0xFU) | 2U;
//
//    /* PLL: 16 MHz HSI -> 84 MHz */
//    RCC->PLLCFGR = 0U;
//
//    RCC->PLLCFGR |= (8U   << 0);    // PLLM = 8
//    RCC->PLLCFGR |= (168U << 6);    // PLLN = 168
//    RCC->PLLCFGR |= (1U   << 16);   // PLLP = /4
//
//    /* Enable PLL */
//    RCC->CR |= (1U << 24);
//
//    while (!(RCC->CR & (1U << 25)))
//    {
//    }
//
//    /* PLL -> SYSCLK */
//    RCC->CFGR |= (2U << 0);
//
//    while (((RCC->CFGR >> 2) & 0x3U) != 2U)
//    {
//    }
//}
//
//
//
//
//void PAL_Time_Init(void)
//{
//    /* 84 MHz CPU -> 1 ms tick */
//    SysTick->LOAD = PAL_TIME_RELOAD;
//
//    /* Clear current counter */
//    SysTick->VAL = 0U;
//
//    /* Processor clock + interrupt + counter */
//    SysTick->CTRL = (1U << 2) |
//                    (1U << 1) |
//                    (1U << 0);
//
//    system_ms = 0U;
//}
void PAL_Time_Init(void)
{
    system_ms = 0U;
}


uint32_t PAL_Time_GetMs(void)
{
    return system_ms;
}

void PAL_Time_DelayMs(uint32_t ms)
{
    uint32_t start = PAL_Time_GetMs();

    while ((PAL_Time_GetMs() - start) < ms)
    {
    }
}
