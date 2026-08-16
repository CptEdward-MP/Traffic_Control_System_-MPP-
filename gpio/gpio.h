#ifndef PAL_GPIO_H
#define PAL_GPIO_H

#include "stm32f411xe.h"
#include <stdint.h>

/*
 * PAL GPIO - Pin Abstraction Layer
 *
 * Thin register-level GPIO abstraction for STM32F411.
 * No STM32 HAL is used.
 */

typedef enum
{
    PAL_GPIO_LOW  = 0U,
    PAL_GPIO_HIGH = 1U
} PAL_GPIO_State_t;

typedef enum
{
    PAL_GPIO_MODE_INPUT  = 0U,
    PAL_GPIO_MODE_OUTPUT = 1U
} PAL_GPIO_Mode_t;

typedef enum
{
    PAL_GPIO_NOPULL = 0U,
    PAL_GPIO_PULLUP,
    PAL_GPIO_PULLDOWN
} PAL_GPIO_Pull_t;

typedef enum
{
    PAL_GPIO_SPEED_LOW = 0U,
    PAL_GPIO_SPEED_MEDIUM,
    PAL_GPIO_SPEED_FAST,
    PAL_GPIO_SPEED_HIGH
} PAL_GPIO_Speed_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint8_t pin;
} PAL_GPIO_Pin_t;

/*
 * Initialize one GPIO pin.
 *
 * For this V0 implementation:
 * - GPIO input/output modes are supported.
 * - Push-pull output is used.
 * - Pull-up/pull-down can be selected.
 * - Output speed can be selected.
 * - Alternate-function mode is intentionally not handled here.
 */
void PAL_GPIO_Init(PAL_GPIO_Pin_t *gpio,
                   PAL_GPIO_Mode_t mode,
                   PAL_GPIO_Pull_t pull,
                   PAL_GPIO_Speed_t speed);

/* Output operations */
void PAL_GPIO_Set(PAL_GPIO_Pin_t *gpio);
void PAL_GPIO_Reset(PAL_GPIO_Pin_t *gpio);
void PAL_GPIO_Write(PAL_GPIO_Pin_t *gpio, PAL_GPIO_State_t state);
void PAL_GPIO_Toggle(PAL_GPIO_Pin_t *gpio);

/* Input operation */
PAL_GPIO_State_t PAL_GPIO_Read(PAL_GPIO_Pin_t *gpio);

#endif /* PAL_GPIO_H */
