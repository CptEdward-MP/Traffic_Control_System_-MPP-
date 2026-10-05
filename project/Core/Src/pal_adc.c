#include "pal_adc.h"
#include <stddef.h>

static void PAL_ADC1_EnableClock(void)
{
    RCC->APB2ENR |= (1U << 8);   /* ADC1EN */
}

void PAL_ADC1_Init(void)
{
    PAL_ADC1_EnableClock();

    /* 12-bit resolution */
    ADC1->CR1 &= ~(3U << 24);

    /* Single conversion */
    ADC1->CR2 &= ~(1U << 1);

    /* Right aligned */
    ADC1->CR2 &= ~(1U << 11);

    /* Software trigger */
    ADC1->CR2 &= ~(3U << 28);

    /* EOC after conversion */
    ADC1->CR2 |= (1U << 10);

    /*
     * One conversion in the sequence
     * L = 0
     */
    ADC1->SQR1 &= ~(0xFU << 20);

    /*
     * SQ1 = Channel 1
     * ADC1_IN1 = PA1
     */
    ADC1->SQR3 &= ~(0x1FU << 0);
    ADC1->SQR3 |= (1U << 0);

    /*
     * Channel 1 sample time = 84 ADC cycles
     * CH1 uses SMP1, bits [5:3]
     */
    ADC1->SMPR2 &= ~(7U << 3);
    ADC1->SMPR2 |= (4U << 3);

    /* Enable ADC */
    ADC1->CR2 |= (1U << 0);
}

bool PAL_ADC1_Read(uint16_t *value)
{
    if (value == NULL)
        return false;

    /* Start conversion */
    ADC1->CR2 |= (1U << 30);   /* SWSTART */

    /* Wait for conversion */
    while ((ADC1->SR & (1U << 1)) == 0U)
    {
    }

    /* Read result */
    *value = (uint16_t)ADC1->DR;

    return true;
}
