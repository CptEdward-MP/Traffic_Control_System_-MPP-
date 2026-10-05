#ifndef PAL_ADC_H
#define PAL_ADC_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stdbool.h>

void PAL_ADC1_Init(void);
bool PAL_ADC1_Read(uint16_t *value);

#endif
