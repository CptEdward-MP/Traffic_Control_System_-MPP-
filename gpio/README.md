# PAL GPIO V0

A small register-level GPIO abstraction for STM32F411.

## Files

- `pal_gpio.h` — public API and pin/configuration types
- `pal_gpio.c` — implementation using STM32 CMSIS register definitions
- `example.c` — simple PC13 LED example

## Design

```text
Application
    |
    v
PAL GPIO API
    |
    v
STM32F411 CMSIS registers
    |
    v
GPIO hardware
```

This layer does **not** use STM32 HAL.

## Supported in V0

- GPIO input/output mode
- Push-pull output
- No pull / pull-up / pull-down
- Low/medium/fast/high GPIO speed
- Set/reset/write/toggle output
- Read input
- Automatic AHB1 GPIO clock enable for GPIOA-E/H

## Deliberately not included yet

- Alternate-function configuration
- Open-drain configuration
- Interrupt configuration
- Debouncing
- Port locking
- HAL compatibility
- Delay/timing functionality

Those should be added only when the project actually needs them.

## Usage

```c
static PAL_GPIO_Pin_t LED1 = {
    .port = GPIOC,
    .pin = 13U
};

PAL_GPIO_Init(&LED1,
              PAL_GPIO_MODE_OUTPUT,
              PAL_GPIO_NOPULL,
              PAL_GPIO_SPEED_LOW);

PAL_GPIO_Set(&LED1);
PAL_GPIO_Reset(&LED1);
PAL_GPIO_Toggle(&LED1);
```

The application does not need to know about `MODER`, `BSRR`, `ODR`, or RCC clock bits.

## Important

This implementation targets **STM32F411** and expects `stm32f411xe.h` to be available in the include path.

It is a thin abstraction, not a replacement for a complete vendor HAL.
