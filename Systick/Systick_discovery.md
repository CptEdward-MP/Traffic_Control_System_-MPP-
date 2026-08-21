# SysTick Discovery — STM32F411 Bare-Metal

## Purpose

This document records the discovery and implementation of **SysTick** while building a bare-metal STM32F411 framework without relying on STM32 HAL.

The goal is to understand:

* what SysTick is
* where its registers come from
* how the processor clock becomes a time base
* how to configure SysTick manually
* how SysTick interrupts work
* how a 1 ms system time base is created
* how the SysTick implementation is eventually hidden behind `PAL_Time`

This document is intended for developers who want to understand the **low-level implementation**.

Application developers do not need this knowledge to use `PAL_Time`.

---

# 1. Where SysTick Comes From

The STM32F411 uses a **Cortex-M4** processor core.

Not every register used by the MCU is documented in the STM32 reference manual.

Conceptually:

```text
                    STM32F411
                       |
             +---------+---------+
             |                   |
             v                   v
     STM32-specific         Cortex-M4
        hardware          core peripherals
             |                   |
             v                   v
           RM0383          ARM documentation
             |                   |
             |              SysTick, NVIC,
             |              core peripherals
             |
       RCC, GPIO, UART,
       I2C, timers, etc.
```

SysTick is part of the **Cortex-M4 core**, rather than an STM32-specific peripheral.

Therefore, its detailed register definition comes from the ARM Cortex-M documentation.

---

# 2. What Is SysTick?

SysTick is a **24-bit timer inside the Cortex-M4 core**.

Its basic job is to count clock ticks.

Conceptually:

```text
Processor clock
      |
      v
   SysTick
      |
      v
  Countdown
      |
      v
 Counter reaches 0
      |
      v
 SysTick event
      |
      +------> COUNTFLAG
      |
      +------> optional interrupt
```

This allows processor clock cycles to be converted into a useful software time base.

For example, with an 84 MHz processor clock:

```text
84,000,000 clock cycles / second
```

Therefore:

```text
84,000 clock cycles / millisecond
```

A SysTick period of approximately 1 ms can therefore be created by configuring the appropriate reload value.

---

# 3. SysTick vs Peripheral Clocks

It is important not to confuse SysTick with the clocks used by STM32 peripherals.

For example, GPIOC requires its peripheral clock:

```c
RCC->AHB1ENR |= (1U << 2);
```

This enables the GPIOC peripheral clock.

SysTick is different.

### Peripheral clock

The question is:

> Does this peripheral have the clock required for its hardware to operate?

Conceptually:

```text
RCC
 |
 +--> AHB1
       |
       +--> GPIOC
```

### SysTick

The question is:

> How many processor clock ticks have elapsed?

Conceptually:

```text
CPU clock
   |
   +--> SysTick
           |
           +--> time base
```

Therefore:

```text
RCC
 |
 +--> provides/distributes clocks
 |
 +--> processor clock
          |
          +--> SysTick
```

SysTick does not provide the clock for GPIO, UART, I2C, etc.

It uses the processor clock to create a software timing reference.

---

# 4. SysTick Registers

The ARM Cortex-M documentation defines four SysTick registers:

| Register     |      Address | Purpose            |
| ------------ | -----------: | ------------------ |
| `SYST_CSR`   | `0xE000E010` | Control and Status |
| `SYST_RVR`   | `0xE000E014` | Reload Value       |
| `SYST_CVR`   | `0xE000E018` | Current Value      |
| `SYST_CALIB` | `0xE000E01C` | Calibration        |

For our implementation, the important registers are:

```text
SYST_CSR
SYST_RVR
SYST_CVR
```

---

# 5. ARM Names vs CMSIS Names

The names used in ARM documentation are represented by CMSIS using C structures.

| ARM documentation | CMSIS C          |
| ----------------- | ---------------- |
| `SYST_CSR`        | `SysTick->CTRL`  |
| `SYST_RVR`        | `SysTick->LOAD`  |
| `SYST_CVR`        | `SysTick->VAL`   |
| `SYST_CALIB`      | `SysTick->CALIB` |

Therefore, our C code uses:

```c
SysTick->CTRL
SysTick->LOAD
SysTick->VAL
SysTick->CALIB
```

This is similar to accessing STM32 peripheral registers:

```c
GPIOC->MODER
GPIOC->ODR
GPIOC->BSRR
```

The difference is that SysTick belongs to the Cortex-M4 core rather than the STM32 peripheral section.

---

# 6. Where Does `SysTick` Come From in C?

Our code includes:

```c
#include "stm32f411xe.h"
```

The device header provides the STM32F411 definitions and includes the appropriate CMSIS Cortex-M definitions.

Conceptually:

```text
main.c
   |
   v
stm32f411xe.h
   |
   v
CMSIS Cortex-M definitions
   |
   v
SysTick_Type
   |
   v
SysTick
```

CMSIS represents the SysTick registers using a C structure and provides the `SysTick` peripheral pointer.

Therefore:

```c
SysTick->LOAD
```

means:

> Access the SysTick LOAD hardware register.

---

# 7. SysTick Control Register

The important control bits are:

```text
Bit 16 : COUNTFLAG
Bit 2  : CLKSOURCE
Bit 1  : TICKINT
Bit 0  : ENABLE
```

---

## 7.1 ENABLE

Bit 0 controls whether the counter is running.

```text
0 = counter disabled
1 = counter enabled
```

In C:

```c
SysTick->CTRL |= (1U << 0);
```

enables the counter.

---

## 7.2 CLKSOURCE

Bit 2 selects the SysTick clock source.

For our implementation:

```text
CLKSOURCE = 1
```

which selects the processor clock.

In C:

```c
SysTick->CTRL |= (1U << 2);
```

---

## 7.3 TICKINT

Bit 1 controls whether SysTick generates an exception when the counter reaches zero.

```text
0 = no SysTick exception
1 = generate SysTick exception
```

Our current implementation uses:

```text
TICKINT = 1
```

because the timing system uses a SysTick interrupt to maintain the system millisecond counter.

---

## 7.4 COUNTFLAG

Bit 16 indicates that the counter has reached zero since the flag was last read.

In a polling implementation, it could be checked using:

```c
while (!(SysTick->CTRL & (1U << 16)))
{
}
```

However, **the current PAL_Time implementation does not use COUNTFLAG for its system time**.

Instead, it uses the SysTick interrupt.

COUNTFLAG remains useful for understanding the underlying SysTick hardware and for simple polling-based experiments.

---

# 8. Reload Register

`SysTick->LOAD` contains the reload value.

When the counter reaches zero, the reload value is loaded back into the counter.

For a desired number of clock cycles `N`:

```text
RELOAD = N - 1
```

Therefore:

```c
SysTick->LOAD = N - 1U;
```

---

# 9. Current 84 MHz Clock Configuration

The current framework configures the STM32F411 to run at:

```text
84 MHz
```

The clock configuration uses the 16 MHz HSI as the PLL input.

Conceptually:

```text
16 MHz HSI
    |
    v
   PLL
    |
    v
84 MHz SYSCLK
    |
    v
Processor clock
    |
    v
  SysTick
```

The current configuration uses:

```c
#define PAL_TIME_CPU_HZ  84000000U
```

Therefore:

```text
CPU frequency = 84,000,000 Hz
```

---

# 10. Creating a 1 ms SysTick Period

The required number of processor clock cycles for 1 ms is:

```text
84,000,000 cycles/second
-------------------------
1,000 milliseconds/second
```

Therefore:

```text
84,000 cycles per millisecond
```

The SysTick reload value is:

```text
84,000 - 1
```

which gives:

```text
83,999
```

The implementation therefore calculates:

```c
#define PAL_TIME_TICK_HZ     1000U

#define PAL_TIME_RELOAD \
    ((PAL_TIME_CPU_HZ / PAL_TIME_TICK_HZ) - 1U)
```

With:

```text
PAL_TIME_CPU_HZ = 84,000,000
PAL_TIME_TICK_HZ = 1,000
```

the result is:

```text
PAL_TIME_RELOAD = 83,999
```

---

# 11. Current SysTick Initialization

The current implementation configures SysTick as follows:

```c
void PAL_Time_Init(void)
{
    /* 84 MHz CPU -> 1 ms tick */
    SysTick->LOAD = PAL_TIME_RELOAD;

    /* Clear current counter */
    SysTick->VAL = 0U;

    /* Processor clock + interrupt + counter */
    SysTick->CTRL = (1U << 2) |
                    (1U << 1) |
                    (1U << 0);

    system_ms = 0U;
}
```

The important sequence is:

```text
Set reload value
      |
      v
Clear current counter
      |
      v
Enable processor clock
      |
      v
Enable SysTick interrupt
      |
      v
Enable SysTick
```

---

# 12. Why Clear `SysTick->VAL`?

The current value register contains the current countdown value.

Writing to it clears the current counter and clears the COUNTFLAG state.

Therefore:

```c
SysTick->VAL = 0U;
```

is part of the initialization sequence.

The general initialization sequence is:

```text
1. Configure LOAD
2. Clear VAL
3. Configure CTRL
```

For the current implementation:

```c
SysTick->LOAD = PAL_TIME_RELOAD;

SysTick->VAL = 0U;

SysTick->CTRL = (1U << 2) |
                (1U << 1) |
                (1U << 0);
```

---

# 13. Interrupt-Based Time Base

The current implementation uses the SysTick interrupt.

The flow is:

```text
             84 MHz processor clock
                       |
                       v
                    SysTick
                       |
                       v
                  1 ms period
                       |
                       v
               SysTick exception
                       |
                       v
               SysTick_Handler()
                       |
                       v
                PAL_Time_Tick()
                       |
                       v
                  system_ms++
```

The handler calls:

```c
void SysTick_Handler(void)
{
    PAL_Time_Tick();
}
```

and:

```c
void PAL_Time_Tick(void)
{
    system_ms++;
}
```

Therefore every SysTick interrupt represents approximately:

```text
1 ms
```

and every interrupt increments:

```c
system_ms
```

by one.

---

# 14. System Millisecond Counter

The PAL implementation maintains:

```c
static volatile uint32_t system_ms = 0U;
```

The `volatile` qualifier is important because this variable is modified from interrupt context.

Every SysTick interrupt executes:

```c
system_ms++;
```

Therefore:

```text
After initialization:

system_ms = 0

after 1 ms:

system_ms = 1

after 2 ms:

system_ms = 2

after 100 ms:

system_ms = 100
```

This creates the basic system time source used by `PAL_Time`.

---

# 15. From SysTick to PAL_Time

The low-level implementation eventually becomes a reusable abstraction.

Conceptually:

```text
84 MHz CPU
    |
    v
 SysTick
    |
    v
1 ms interrupt
    |
    v
SysTick_Handler()
    |
    v
PAL_Time_Tick()
    |
    v
system_ms
    |
    +------------------+
    |                  |
    v                  v
PAL_Time_GetMs()   PAL_Time_DelayMs()
```

The important separation is:

```text
Low-level implementation
        |
        v
     SysTick
        |
        v
   PAL_Time module
        |
        v
Application
```

The application does not need to access SysTick directly.

---

# 16. Polling vs Interrupt

There are two fundamentally different ways to use SysTick.

## Polling

The program waits for COUNTFLAG:

```c
while (!(SysTick->CTRL & (1U << 16)))
{
}
```

Conceptually:

```text
Start countdown
      |
      v
Keep checking COUNTFLAG
      |
      v
Counter reaches zero
      |
      v
Continue
```

This is useful for understanding SysTick and for simple blocking delays.

---

## Interrupt

The processor receives a SysTick exception when the counter reaches zero.

```text
SysTick
   |
   v
Counter reaches zero
   |
   v
Interrupt
   |
   v
SysTick_Handler()
   |
   v
PAL_Time_Tick()
```

The application does not need to continuously poll the timer.

This is the approach used by the current `PAL_Time` implementation.

---

# 17. Why the Current Implementation Uses Interrupts

The interrupt approach allows us to maintain a continuously increasing system time.

Instead of doing:

```c
while (!(SysTick->CTRL & (1U << 16)))
{
}
```

for every delay, the system can maintain:

```c
system_ms
```

in the background.

Application code can then ask:

```c
PAL_Time_GetMs();
```

whenever it needs the current time.

This also enables timestamp-based periodic operations.

For example:

```c
uint32_t now = PAL_Time_GetMs();

if ((now - last_time) >= 100U)
{
    last_time = now;

    /* Perform periodic work */
}
```

---

# 18. SysTick Handler Ownership

There must be exactly **one** SysTick handler in the final firmware.

For the current bare-metal implementation:

```c
void SysTick_Handler(void)
{
    PAL_Time_Tick();
}
```

The handler belongs to the timing implementation.

If another framework or library also tries to provide:

```c
void SysTick_Handler(void)
```

there will be a conflict.

This is particularly important when integrating HAL or another RTOS.

The project must clearly define who owns the SysTick interrupt.

---

# 19. Clock Frequency Matters

SysTick timing depends directly on the processor clock.

For example:

```text
84 MHz CPU
    |
    v
84,000 cycles / ms
    |
    v
RELOAD = 83,999
```

If the CPU frequency changes:

```text
100 MHz CPU
```

then the required reload value also changes.

Therefore:

```text
CPU frequency
      |
      v
SysTick reload
      |
      v
1 ms timing
```

The SysTick configuration cannot be separated from the actual processor clock.

---

# 20. Why Clock Configuration Must Be Understood

It is not enough to simply enable HSI.

For example:

```c
RCC->CR |= (1U << 0);
```

enables HSI.

That does not automatically mean that HSI is the final processor clock.

The actual clock path must be determined from the RCC configuration.

For the current implementation:

```text
16 MHz HSI
    |
    v
   PLL
    |
    v
84 MHz SYSCLK
    |
    v
Processor clock
    |
    v
SysTick
```

The clock configuration therefore determines the value used for the SysTick calculation.

---

# 21. Current Clock and SysTick Relationship

The current system can be viewed as:

```text
             STM32F411
                 |
                 v
             16 MHz HSI
                 |
                 v
                PLL
                 |
                 v
             84 MHz SYSCLK
                 |
                 v
          Processor clock
                 |
                 v
              SysTick
                 |
                 v
            1 ms tick
                 |
                 v
         system_ms counter
                 |
                 v
             PAL_Time
```

This is the complete chain from the hardware clock to the application-level time API.

---

# 22. Bare-Metal Example

A minimal low-level SysTick configuration for the current 84 MHz system is:

```c
#include "stm32f411xe.h"

#define CPU_HZ      84000000U
#define TICK_HZ     1000U
#define RELOAD      ((CPU_HZ / TICK_HZ) - 1U)

static void SysTick_Init(void)
{
    SysTick->LOAD = RELOAD;

    SysTick->VAL = 0U;

    SysTick->CTRL = (1U << 2) |
                    (1U << 1) |
                    (1U << 0);
}
```

The corresponding interrupt handler is:

```c
void SysTick_Handler(void)
{
    /* 1 ms has elapsed */
}
```

The PAL implementation extends this by maintaining the system time:

```c
static volatile uint32_t system_ms = 0U;

void PAL_Time_Tick(void)
{
    system_ms++;
}
```

---

# 23. Bare-Metal vs PAL Layer

It is useful to distinguish the two layers.

## Bare-metal layer

This layer deals directly with:

```text
SysTick->CTRL
SysTick->LOAD
SysTick->VAL
RCC
CPU clock
interrupt handler
```

Its purpose is to understand and configure the hardware.

---

## PAL layer

The PAL layer provides:

```c
PAL_Time_Init();

PAL_Time_DelayMs();

PAL_Time_GetMs();
```

Its purpose is to provide a stable interface to application code.

---

## Application layer

The application simply does:

```c
PAL_Time_Init();

uint32_t now = PAL_Time_GetMs();

PAL_Time_DelayMs(100);
```

The application does not need to know how the time source is implemented.

---

# 24. Current Architecture

The complete architecture is:

```text
                     Application
                          |
                          v
                      PAL_Time
                          |
              +-----------+-----------+
              |                       |
              v                       v
      PAL_Time_GetMs()       PAL_Time_DelayMs()
              |
              v
          system_ms
              |
              v
       PAL_Time_Tick()
              |
              v
       SysTick_Handler()
              |
              v
           SysTick
              |
              v
      84 MHz CPU clock
```

The clock is produced through:

```text
16 MHz HSI
    |
    v
   PLL
    |
    v
84 MHz SYSCLK
```

---

# 25. Important Lessons

### SysTick is not an STM32 peripheral

It belongs to the Cortex-M4 core.

### SysTick does not provide peripheral clocks

RCC handles the STM32 peripheral clocking.

### SysTick uses a clock to count

The processor clock determines how quickly the counter runs.

### `LOAD` determines the period

For `N` clock cycles:

```text
LOAD = N - 1
```

### `VAL` contains the current countdown

Writing to `VAL` clears the current counter.

### `CTRL` controls the timer

Important bits are:

```text
ENABLE
CLKSOURCE
TICKINT
COUNTFLAG
```

### Interrupt mode creates a useful system time base

The current implementation uses:

```text
SysTick interrupt
      |
      v
PAL_Time_Tick()
      |
      v
system_ms++
```

---

# 26. Discovery Workflow

The workflow used for understanding peripherals is:

```text
1. Find the peripheral/core feature
          ↓
2. Determine where it is documented
          ↓
3. Find its clock source
          ↓
4. Find its registers
          ↓
5. Understand the important bits
          ↓
6. Calculate the required timing
          ↓
7. Write minimal bare-metal code
          ↓
8. Test on hardware
          ↓
9. Wrap the implementation in a PAL/API
```

For SysTick:

```text
Cortex-M4
    ↓
ARM SysTick documentation
    ↓
CTRL / LOAD / VAL
    ↓
Processor clock
    ↓
Reload calculation
    ↓
SysTick interrupt
    ↓
system_ms
    ↓
PAL_Time
```

---

# 27. Understanding Checklist

* [ ] What is SysTick?
* [ ] Is SysTick an STM32F411 peripheral or a Cortex-M4 core peripheral?
* [ ] Where is SysTick documented?
* [ ] What are `CTRL`, `LOAD`, and `VAL`?
* [ ] What does `ENABLE` do?
* [ ] What does `CLKSOURCE` do?
* [ ] What does `TICKINT` do?
* [ ] What does `COUNTFLAG` tell us?
* [ ] Why is `LOAD = N - 1`?
* [ ] Why is `SysTick->VAL = 0` used during initialization?
* [ ] How does the processor clock determine the SysTick period?
* [ ] Why can GPIO operate without SysTick?
* [ ] Why does GPIO still require its RCC peripheral clock?
* [ ] What is the difference between a peripheral clock and a software time base?
* [ ] What is the difference between polling and interrupt-based SysTick?
* [ ] Why does the current implementation use `TICKINT = 1`?
* [ ] How does `SysTick_Handler()` connect to `PAL_Time_Tick()`?
* [ ] Why must there be only one SysTick handler?
* [ ] How does the 84 MHz processor clock produce a 1 ms tick?
* [ ] How does the SysTick implementation become `PAL_Time`?

---

# 28. Final Mental Model

The most important concept is the complete chain:

```text
                 RCC / Clock Configuration
                          |
                          v
                    84 MHz CPU clock
                          |
                          v
                       SysTick
                          |
                   83,999 reload
                          |
                          v
                     1 ms period
                          |
                          v
                  SysTick interrupt
                          |
                          v
                  SysTick_Handler()
                          |
                          v
                   PAL_Time_Tick()
                          |
                          v
                    system_ms++
                          |
                          v
                      PAL_Time
                          |
                          v
                    Application
```

The fundamental idea is:

> **SysTick is a Cortex-M4 hardware timer that can turn the processor clock into a periodic event. The current implementation uses that event every 1 ms to maintain a software millisecond counter, which is then exposed through the `PAL_Time` abstraction.**
