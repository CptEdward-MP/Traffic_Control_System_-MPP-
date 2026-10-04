/*
 * pal_adc.c
 *
 *  Created on: Oct 4, 2026
 *      Author: Manansh Pandey
 */

#include <stddef.h>
#include "pal_adc.h"

static void PAL_ADC1_EnableClock(void)
{
    /*
     * ADC1 is connected to APB2.
     *
     * RCC_APB2ENR:
     * ADC1EN = bit 8
     */
    RCC->APB2ENR |= (1U << 8);
}





void PAL_ADC1_Init(void)
{
    /* Enable ADC1 peripheral clock */
    PAL_ADC1_EnableClock();

    /*
     * ADC_CR1
     *
     * RES   = 00 → 12-bit resolution
     * SCAN  = 0  → scan mode disabled
     * EOCIE = 0  → EOC interrupt disabled
     *
     * These are already 0 after reset,
     * so no write is required.
     */

    /*
     * ADC_CR2
     *
     * EOCS  = 1 → EOC after each conversion
     * DMA   = 0 → DMA disabled
     * ALIGN = 0 → right aligned
     * CONT  = 0 → single conversion
     * EXTEN = 00 → no external trigger
     * ADON  = 1 → enable ADC
     */

    ADC1->CR2 |= (1U << 10);  /* EOCS */
    ADC1->CR2 |= (1U << 0);   /* ADON */



    /* One conversion */
    ADC1->SQR1 &= ~(0xFU << 20);

    /* First conversion = Channel 0 */
    ADC1->SQR3 &= ~(0x1FU << 0);

    /* Channel 0 sampling time = 84 cycles */
    ADC1->SMPR2 &= ~(7U << 0);
    ADC1->SMPR2 |=  (4U << 0);
}


bool PAL_ADC1_ReadChannel(uint8_t channel, uint16_t *value)
{
    if (value == NULL)
    {
        return false;
    }

    /* Select channel as SQ1 */
    ADC1->SQR3 &= ~(0x1FU << 0);
    ADC1->SQR3 |= ((uint32_t)channel << 0);

    /* Start regular conversion */
    ADC1->CR2 |= (1U << 30);

    /* Wait for conversion to complete */
    while ((ADC1->SR & (1U << 1)) == 0U)
    {
    }

    /* Read converted value */
    *value = (uint16_t)ADC1->DR;

    return true;
}
