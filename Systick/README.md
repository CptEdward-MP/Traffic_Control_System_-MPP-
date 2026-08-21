# PAL_Time / SysTick — README

## Purpose

`PAL_Time` is the application-facing timing API for our STM32F411 bare-metal project.

The goal is that a teammate can use timing **without knowing how SysTick works internally**.

---

## 1. Files

```text
pal_time.h
pal_time.c
```

The public API is:

```c
void PAL_Time_Init(void);
void PAL_Time_DelayMs(uint32_t ms);
uint32_t PAL_Time_GetMs(void);
```

Normally, application code only includes:

```c
#include "pal_time.h"
```

---

## 2. Initialization

Call `PAL_Time_Init()` once during startup:

```c
int main(void)
{
    // Other initialization

    PAL_Time_Init();

    while (1)
    {
        // Application
    }
}
```

Do not repeatedly call it inside the main loop.

---

## 3. Delay

To wait for a number of milliseconds:

```c
PAL_Time_DelayMs(500);
```

Example:

```c
PAL_GPIO_Set(&LED1);
PAL_Time_DelayMs(500);

PAL_GPIO_Reset(&LED1);
PAL_Time_DelayMs(500);
```

This gives approximately:

```text
LED ON
  |
  | 500 ms
  |
LED OFF
  |
  | 500 ms
  |
repeat
```

This is a **blocking delay**.

---

## 4. Get Current System Time

Use:

```c
uint32_t now = PAL_Time_GetMs();
```

The value represents milliseconds since:

```c
PAL_Time_Init();
```

was called.

Example:

```c
uint32_t start = PAL_Time_GetMs();

/* do some work */

uint32_t elapsed = PAL_Time_GetMs() - start;
```

---

## 5. Non-blocking Periodic Work

For periodic work, prefer checking the system time instead of blocking:

```c
uint32_t last_time = PAL_Time_GetMs();

while (1)
{
    uint32_t now = PAL_Time_GetMs();

    if ((now - last_time) >= 100U)
    {
        last_time = now;

        // Do something every 100 ms
    }

    // Other work can continue here
}
```

Multiple modules can use the same system time:

```text
                 PAL_Time
                    |
              system time
                    |
        +-----------+-----------+
        |           |           |
       UART       Sensor       LED
      timeout     timeout     periodic
```

---

## 6. Complete LED Blink Example

```c
#include "stm32f411xe.h"
#include "pal_gpio.h"
#include "pal_time.h"

static PAL_GPIO_Pin_t LED1 =
{
    .port = GPIOC,
    .pin  = 13U
};

int main(void)
{
    PAL_GPIO_Init(&LED1,
                  PAL_GPIO_MODE_OUTPUT,
                  PAL_GPIO_NOPULL,
                  PAL_GPIO_SPEED_LOW);

    PAL_Time_Init();

    while (1)
    {
        PAL_GPIO_Set(&LED1);
        PAL_Time_DelayMs(500);

        PAL_GPIO_Reset(&LED1);
        PAL_Time_DelayMs(500);
    }
}
```

---

## 7. What You Do NOT Need to Know

If you are only using PAL_Time, you do not need to understand these first:

```text
SysTick->CTRL
SysTick->LOAD
SysTick->VAL
COUNTFLAG
CLKSOURCE
TICKINT
RELOAD
SysTick_Handler()
```

Those are implementation details.

The application-facing interface is simply:

```c
PAL_Time_Init();
PAL_Time_DelayMs(...);
PAL_Time_GetMs();
```

---

## 8. Current V0 Limitation

PAL_Time V0 assumes:

```text
Processor clock = 16 MHz
```

The implementation therefore calculates SysTick timing using:

```c
#define PAL_TIME_CPU_HZ 16000000U
```

If the processor clock is later changed, the **PAL_Time implementation must be updated**.

For example:

```text
V0:

16 MHz CPU
    ↓
SysTick
    ↓
1 ms tick
    ↓
PAL_Time
```

Later:

```text
84 MHz CPU
    ↓
SysTick
    ↓
new reload value
    ↓
1 ms tick
    ↓
PAL_Time
```

The application API should remain unchanged:

```c
PAL_Time_Init();
PAL_Time_DelayMs(...);
PAL_Time_GetMs();
```

---

## 9. SysTick Handler Ownership

There must be exactly **one**:

```c
void SysTick_Handler(void)
```

in the final firmware.

Our PAL_Time implementation uses:

```c
void SysTick_Handler(void)
{
    system_ms++;
}
```

If a CubeIDE/HAL-generated file already contains:

```c
void SysTick_Handler(void)
{
    HAL_IncTick();
}
```

it conflicts with PAL_Time.

For the **no-HAL version**, the HAL SysTick handler must not also be present.

---

## 10. Is This HAL?

No.

The application uses:

```c
PAL_Time_DelayMs(500);
```

but PAL_Time V0 directly configures the Cortex-M4 SysTick registers.

The conceptual stack is:

```text
Application
     ↓
PAL_Time
     ↓
CMSIS/device definitions
     ↓
Cortex-M4 SysTick
     ↓
Hardware
```

It is not:

```text
Application
     ↓
HAL
     ↓
SysTick
```

Using:

```c
#include "stm32f411xe.h"
```

provides register definitions and does not mean that HAL is being used.

---

## 11. Which Function Should I Use?

### I just need to wait

```c
PAL_Time_DelayMs(100);
```

### I need the current time

```c
uint32_t now = PAL_Time_GetMs();
```

### I need something to happen periodically

Use `PAL_Time_GetMs()` with a timestamp:

```c
if ((PAL_Time_GetMs() - last_time) >= period)
{
    last_time = PAL_Time_GetMs();

    // Do work
}
```

---

## 12. Current Architecture

```text
                    Application
                         |
                +--------+--------+
                |                 |
                v                 v
            PAL_GPIO          PAL_Time
                                  |
                                  v
                               SysTick
                                  |
                                  v
                             CPU clock
```

Application code knows about:

```text
PAL_GPIO
PAL_Time
```

It does not need to know the underlying registers.

---

## 13. Future Improvements

### V1 — Clock-aware PAL_Time

Remove the hard-coded:

```c
16000000U
```

and calculate the SysTick reload value from the actual processor clock.

```text
RCC clock configuration
        ↓
actual CPU frequency
        ↓
PAL_Time
        ↓
SysTick reload
```

### V2 — More timing utilities

Possible future APIs:

```c
PAL_Time_ElapsedMs(...)
PAL_Time_HasElapsed(...)
```

### V3 — Different hardware implementation

If we later decide another hardware timer is more appropriate:

```text
PAL_Time
   |
   +--> SysTick
```

could become:

```text
PAL_Time
   |
   +--> TIMx
```

without requiring application code to change.

---

## 14. Quick Reference

Include:

```c
#include "pal_time.h"
```

Initialize once:

```c
PAL_Time_Init();
```

Delay:

```c
PAL_Time_DelayMs(1000);
```

Get current time:

```c
uint32_t now = PAL_Time_GetMs();
```

Periodic operation:

```c
if ((PAL_Time_GetMs() - last_time) >= period)
{
    last_time = PAL_Time_GetMs();

    // Do work
}
```

---

## Mental Model

> **PAL_Time gives the application a simple concept of time while hiding the SysTick implementation underneath.**
