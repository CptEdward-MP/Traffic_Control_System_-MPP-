#ifndef PAL_ADC_H
#define PAL_ADC_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    PAL_ADC_SAMPLE_3_CYCLES = 0U,
    PAL_ADC_SAMPLE_15_CYCLES,
    PAL_ADC_SAMPLE_28_CYCLES,
    PAL_ADC_SAMPLE_56_CYCLES,
    PAL_ADC_SAMPLE_84_CYCLES,
    PAL_ADC_SAMPLE_112_CYCLES,
    PAL_ADC_SAMPLE_144_CYCLES,
    PAL_ADC_SAMPLE_480_CYCLES
} PAL_ADC_SampleTime_t;


/*
 * Initialize ADC1.
 *
 * Configures ADC-wide settings:
 * - Continuous conversion
 * - EOC after each conversion
 * - ADC enabled
 */
void PAL_ADC1_Init(void);


/*
 * Configure the regular conversion sequence.
 *
 * channels : Array containing ADC channel numbers.
 * count    : Number of channels, from 1 to 10.
 *
 * All selected channels use the same sampling time.
 *
 * Returns:
 * true  -> configuration successful
 * false -> invalid parameters
 */
bool PAL_ADC1_ConfigureSequence(const uint8_t *channels,
                                uint8_t count);


/*
 * Start continuous ADC conversion.
 *
 * Call this once after initialization and sequence configuration.
 */
void PAL_ADC1_Start(void);


/*
 * Read the next completed ADC conversion.
 *
 * Blocks until EOC is set.
 *
 * Returns:
 * true  -> value successfully read
 * false -> invalid pointer
 */
bool PAL_ADC1_Read(uint16_t *value);


bool PAL_ADC1_ReadSequence(uint16_t *values);


#endif /* PAL_ADC_H */
