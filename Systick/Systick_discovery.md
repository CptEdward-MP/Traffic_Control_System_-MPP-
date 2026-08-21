# SysTick Discovery — STM32F411 Bare-Metal

## Purpose

This document records our discovery of **SysTick** while building a bare-metal STM32F411 framework without relying on STM32 HAL.

The goal is to understand:
- what SysTick is
- where its registers come from
- how the processor clock becomes a time base
- how to configure SysTick manually
- how `COUNTFLAG` can be used for polling
- how this can later become a reusable PAL timing module

---

## 1. Where SysTick Comes From

The STM32F411 uses a **Cortex-M4** processor core.

Not every register used by the MCU is documented in the STM32 reference manual.

```text
                    STM32F411
                       |
          +------------+------------+
          |                         |
   STM32-specific              Cortex-M4 core
     hardware                     features
          |                         |
      RM0383                  ARM Cortex-M4
          |                  documentation
          |                         |
    RCC, GPIO, UART,          SysTick, NVIC,
    I2C, timers, etc.         core peripherals
```

SysTick is part of the **Cortex-M4 core**, so its detailed registers are documented by ARM.

---

## 2. SysTick's Job

SysTick is a **24-bit system timer inside the Cortex-M4 core**.

Its job is not to provide the clock to peripherals. Instead, it uses a clock to **count clock ticks**.

```text
Processor clock
      |
      v
   SysTick
      |
      v
 countdown
      |
      v
 reaches 0
      |
      v
 COUNTFLAG / optional interrupt
```

This lets us turn clock cycles into time intervals.

For example:

```text
16 MHz processor clock
        |
        v
16,000 clock cycles
        |
        v
approximately 1 ms
```

---

## 3. SysTick vs Peripheral Clocks

For GPIOC we previously did:

```c
RCC->AHB1ENR |= (1U << 2);
```

This enables the **GPIOC peripheral clock**.

We did **not** need SysTick for GPIO to work.

### Peripheral clock

Answers:

> Does this peripheral have its required clock so that its hardware can operate?

```text
RCC
 |
 +--> AHB1
       |
       +--> GPIOC
```

### SysTick

Answers:

> How many processor clock ticks have elapsed?

```text
CPU clock
   |
   +--> SysTick
          |
          +--> time base
```

Therefore:

```text
RCC --> provides/distributes clocks
SysTick --> counts a selected clock to create a time base
```

---

## 4. SysTick Registers

The ARM Cortex-M4 documentation gives four SysTick registers:

| Register | Address | Purpose |
|---|---:|---|
| `SYST_CSR` | `0xE000E010` | Control and Status |
| `SYST_RVR` | `0xE000E014` | Reload Value |
| `SYST_CVR` | `0xE000E018` | Current Value |
| `SYST_CALIB` | `0xE000E01C` | Calibration |

For our first delay implementation, the important three are:

```text
SYST_CSR
SYST_RVR
SYST_CVR
```

---

## 5. CMSIS Names vs ARM Documentation Names

The names in the ARM documentation are not necessarily the names used by CMSIS in C.

| ARM documentation | CMSIS C |
|---|---|
| `SYST_CSR` | `SysTick->CTRL` |
| `SYST_RVR` | `SysTick->LOAD` |
| `SYST_CVR` | `SysTick->VAL` |
| `SYST_CALIB` | `SysTick->CALIB` |

Therefore our C code uses:

```c
SysTick->CTRL
SysTick->LOAD
SysTick->VAL
SysTick->CALIB
```

This is the same general idea as:

```c
GPIOC->MODER
GPIOC->ODR
GPIOC->BSRR
```

---

## 6. Where the C Definition Comes From

Our application can include:

```c
#include "stm32f411xe.h"
```

The STM32 device header includes the appropriate CMSIS Cortex-M definitions.

Conceptually:

```text
main.c
  |
  v
stm32f411xe.h
  |
  v
core_cm4.h
  |
  v
SysTick_Type
  |
  v
SysTick
```

CMSIS represents the hardware registers using a C structure and provides the `SysTick` peripheral pointer.

Therefore:

```c
SysTick->LOAD
```

means:

> Access the SysTick LOAD hardware register.

---

## 7. `SYST_CSR` / `SysTick->CTRL`

Important bits:

```text
Bit 16 : COUNTFLAG
Bit 2  : CLKSOURCE
Bit 1  : TICKINT
Bit 0  : ENABLE
```

### Bit 0 — ENABLE

```text
0 = counter disabled
1 = counter enabled
```

```c
SysTick->CTRL |= (1U << 0);
```

enables the counter.

### Bit 2 — CLKSOURCE

```text
0 = external/reference clock
1 = processor clock
```

For our basic implementation:

```c
SysTick->CTRL |= (1U << 2);
```

### Bit 1 — TICKINT

```text
0 = no SysTick exception request
1 = generate SysTick exception when counter reaches zero
```

For our first implementation:

```text
TICKINT = 0
```

because we are using polling.

### Bit 16 — COUNTFLAG

This becomes `1` when the timer has counted down to zero since the flag was last read.

We can poll it:

```c
while (!(SysTick->CTRL & (1U << 16)))
{
}
```

Meaning:

> Wait until SysTick reaches zero.

---

## 8. `SYST_RVR` / `SysTick->LOAD`

The reload register contains the value loaded into the counter.

The ARM documentation gives:

```text
For N processor clock cycles:

RELOAD = N - 1
```

Example:

```text
Processor clock = 16 MHz

16,000,000 cycles / second
16,000 cycles / millisecond
```

Therefore:

```c
SysTick->LOAD = 16000U - 1U;
```

for a 1 ms period.

---

## 9. `SYST_CVR` / `SysTick->VAL`

This contains the current value of the countdown.

Writing any value clears the current counter and clears `COUNTFLAG`.

Therefore:

```c
SysTick->VAL = 0U;
```

is part of the initialization sequence.

The ARM documentation recommends:

1. Program reload value.
2. Clear current value.
3. Program Control and Status register.

So:

```c
SysTick->LOAD = 16000U - 1U;
SysTick->VAL  = 0U;
SysTick->CTRL = (1U << 2) | (1U << 0);
```

---

## 10. Complete Basic SysTick Implementation

For a processor clock of 16 MHz:

```c
#include "stm32f411xe.h"

static void SysTick_Init(void)
{
    /*
     * 16 MHz processor clock
     *
     * 16,000 cycles = 1 ms
     *
     * RELOAD = N - 1
     */
    SysTick->LOAD = 16000U - 1U;

    /* Clear current counter */
    SysTick->VAL = 0U;

    /*
     * CLKSOURCE = 1
     * ENABLE    = 1
     *
     * TICKINT remains 0
     */
    SysTick->CTRL = (1U << 2) | (1U << 0);
}
```

---

## 11. Creating a 1 ms Delay

```c
static void delay_1ms(void)
{
    while (!(SysTick->CTRL & (1U << 16)))
    {
    }
}
```

Every time this function returns, approximately 1 ms has passed.

---

## 12. Creating `delay_ms()`

```c
static void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        while (!(SysTick->CTRL & (1U << 16)))
        {
        }
    }
}
```

Now:

```c
delay_ms(500);
```

means:

```text
500 x 1 ms
     |
     v
approximately 500 ms
```

---

## 13. Using SysTick With Our PAL GPIO

```c
#include "pal_gpio.h"
#include "stm32f411xe.h"

static PAL_GPIO_Pin_t LED1 =
{
    .port = GPIOC,
    .pin  = 13U
};

static void SysTick_Init(void)
{
    SysTick->LOAD = 16000U - 1U;
    SysTick->VAL = 0U;
    SysTick->CTRL = (1U << 2) | (1U << 0);
}

static void delay_ms(uint32_t ms)
{
    while (ms--)
    {
        while (!(SysTick->CTRL & (1U << 16)))
        {
        }
    }
}

int main(void)
{
    PAL_GPIO_Init(&LED1,
                  PAL_GPIO_MODE_OUTPUT,
                  PAL_GPIO_NOPULL,
                  PAL_GPIO_SPEED_LOW);

    /* Enable HSI */
    RCC->CR |= (1U << 0);

    /* Wait for HSI */
    while (!(RCC->CR & (1U << 1)))
    {
    }

    SysTick_Init();

    while (1)
    {
        PAL_GPIO_Set(&LED1);
        delay_ms(500);

        PAL_GPIO_Reset(&LED1);
        delay_ms(500);
    }
}
```

---

## 14. Important Clock Assumption

The example above assumes:

```text
Processor clock = 16 MHz
```

because:

```text
16 MHz / 1000 = 16,000 cycles per millisecond
```

However, **enabling HSI does not by itself prove that the processor is using HSI as SYSCLK**.

The actual clock path must be understood through RCC configuration.

The next RCC discovery should establish:

```text
Clock source
     |
     v
SYSCLK
     |
     v
HCLK
     |
     +----> PCLK1
     |
     +----> PCLK2
```

Only after that should the SysTick reload value be based on the actual processor clock.

---

## 15. Polling vs Interrupt

### Polling

Current approach:

```c
while (!(SysTick->CTRL & (1U << 16)))
{
}
```

The CPU waits until the counter reaches zero.

This is simple and useful for learning.

### Interrupt

Set:

```text
TICKINT = 1
```

Then:

```text
SysTick
   |
   v
COUNTFLAG
   |
   v
SysTick exception
   |
   v
SysTick_Handler()
```

A future implementation could maintain:

```c
volatile uint32_t system_ms;

void SysTick_Handler(void)
{
    system_ms++;
}
```

This lets the main program continue doing other work instead of blocking inside a delay loop.

---

## 16. Why SysTick Is Useful for Other Peripherals

UART, I2C, PWM and timers also involve timing, but SysTick does not directly provide their peripheral clocks.

Conceptually:

```text
RCC
 |
 +--> peripheral clocks
 |
 +--> processor clock
          |
          +--> SysTick
```

Examples:

```text
UART:
peripheral clock -> baud-rate timing

I2C:
peripheral clock -> SCL timing

Timer:
timer clock -> counter -> PWM frequency

SysTick:
processor clock -> countdown -> software time base
```

---

## 17. Discovery Workflow

The workflow we are using for every peripheral:

```text
1. Find the peripheral
        ↓
2. Find its clock source
        ↓
3. Enable its clock if required
        ↓
4. Find its registers
        ↓
5. Understand important bits
        ↓
6. Calculate required timing
        ↓
7. Write minimal bare-metal code
        ↓
8. Test on hardware
        ↓
9. Wrap it in PAL/API
```

For SysTick:

```text
Cortex-M4
   ↓
SysTick documentation
   ↓
CTRL / LOAD / VAL
   ↓
processor clock
   ↓
reload calculation
   ↓
COUNTFLAG
   ↓
delay_ms()
```

---

## 18. Current Understanding Checklist

- [ ] What is SysTick?
- [ ] Is SysTick an STM32F411 peripheral or a Cortex-M4 core peripheral?
- [ ] Where is SysTick documented?
- [ ] What are `CTRL`, `LOAD`, and `VAL`?
- [ ] What does `ENABLE` do?
- [ ] What does `CLKSOURCE` do?
- [ ] What does `TICKINT` do?
- [ ] What does `COUNTFLAG` tell us?
- [ ] Why is `LOAD = N - 1`?
- [ ] Why does `SysTick->VAL = 0` appear during initialization?
- [ ] How does the processor clock determine the SysTick period?
- [ ] Why can GPIO work without SysTick?
- [ ] Why does GPIO still need its RCC peripheral clock?
- [ ] What is the difference between a peripheral clock and a time base?
- [ ] How could SysTick eventually become a system millisecond counter?

---

## 19. Next Discovery

The next step is the **actual STM32F411 clock tree**:

```text
HSI / HSE / PLL
      ↓
    SYSCLK
      ↓
     HCLK
      ↓
 ┌────┴────┐
PCLK1     PCLK2
```

We will use `RCC_CR` and `RCC_CFGR` to determine:

```text
SYSCLK = ?
HCLK   = ?
PCLK1  = ?
PCLK2  = ?
```

Once these are known, the SysTick reload value can be calculated from the **actual processor frequency** instead of assuming 16 MHz.

---

## Core Takeaway

```text
RCC
 ↓
provides/distributes clocks

SysTick
 ↓
counts the processor clock

COUNTFLAG
 ↓
tells us a period completed

delay_ms()
 ↓
turns those periods into a usable software API
```

**SysTick does not power GPIO, UART, I2C, etc. It is a mechanism for turning the processor clock into a software-accessible time base.**
