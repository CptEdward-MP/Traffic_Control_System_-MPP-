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

    ADC1->CR1 |= (1U << 8);   /* SCAN = 1 */

    ADC1->CR2 |= (1U << 10);  /* EOCS */
    ADC1->CR2 |= (1U << 0);   /* ADON */



    /* Three conversions */
    ADC1->SQR1 &= ~(0xFU << 20);
    ADC1->SQR1 |= (2U << 20);   /* L = 2 -> 3 conversions */

    /* SQ1 = Channel 0 */
    ADC1->SQR3 &= ~(0x1FU << 0);
    ADC1->SQR3 |= (0U << 0);

    /* SQ2 = Channel 1 */
    ADC1->SQR3 &= ~(0x1FU << 5);
    ADC1->SQR3 |= (1U << 5);

    /* SQ3 = Channel 2 */
    ADC1->SQR3 &= ~(0x1FU << 10);
    ADC1->SQR3 |= (2U << 10);

    /* CH0 */
    ADC1->SMPR2 &= ~(7U << 0);
    ADC1->SMPR2 |= (4U << 0);

    /* CH1 */
    ADC1->SMPR2 &= ~(7U << 3);
    ADC1->SMPR2 |= (4U << 3);

    /* CH2 */
    ADC1->SMPR2 &= ~(7U << 6);
    ADC1->SMPR2 |= (4U << 6);
}


bool PAL_ADC1_ReadSequence(uint16_t *values)
{
    if (values == NULL)
    {
        return false;
    }

    /*
     * Start the 3-channel conversion sequence.
     *
     * CH0 → CH1 → CH2 → STOP
     */
    ADC1->CR2 |= (1U << 30);   /* SWSTART */

    /*
     * Wait for CH0 conversion to complete.
     */
    while ((ADC1->SR & (1U << 1)) == 0U)
    {
    }

    values[0] = (uint16_t)ADC1->DR;


    /*
     * Wait for CH1 conversion to complete.
     */
    while ((ADC1->SR & (1U << 1)) == 0U)
    {
    }

    values[1] = (uint16_t)ADC1->DR;


    /*
     * Wait for CH2 conversion to complete.
     */
    while ((ADC1->SR & (1U << 1)) == 0U)
    {
    }

    values[2] = (uint16_t)ADC1->DR;


    return true;
}
