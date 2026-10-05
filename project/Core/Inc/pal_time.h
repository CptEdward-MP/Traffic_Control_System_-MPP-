#ifndef PAL_TIME_H
#define PAL_TIME_H

#include <stdint.h>

/*
 * PAL_Time V0
 *
 * Provides a simple millisecond time base using
 * the Cortex-M4 SysTick timer.
 *
*/

void PAL_Clock_Init(void);

void PAL_Time_Init(void);

void PAL_Time_Tick(void);

void PAL_Time_DelayMs(uint32_t ms);

uint32_t PAL_Time_GetMs(void);

#endif /* PAL_TIME_H */
