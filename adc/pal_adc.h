/*
 * pal_adc.h
 *
 *  Created on: Oct 4, 2026
 *      Author: Manansh Pandey
 */

#ifndef PAL_ADC_H
#define PAL_ADC_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stdbool.h>

void PAL_ADC1_Init(void);

bool PAL_ADC1_ReadChannel(uint8_t channel, uint16_t *value);

#endif /* PAL_ADC_H */
