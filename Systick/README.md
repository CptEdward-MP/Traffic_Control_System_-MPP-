# PAL_Time — User Guide

## Purpose

`PAL_Time` provides simple time-related functions for application code.

The main goal of this module is to allow application developers to work with time **without needing to understand the underlying hardware or operating system implementation**.

You only need to know the `PAL_Time` API.

```c
#include "pal_time.h"
```

The main functions are:

```c
void PAL_Time_Init(void);

void PAL_Time_DelayMs(uint32_t ms);

uint32_t PAL_Time_GetMs(void);
```

---

# 1. Initialization

Initialize `PAL_Time` once during application startup:

```c
PAL_Time_Init();
```

Example:

```c
#include "pal_time.h"

int main(void)
{
    PAL_Time_Init();

    while (1)
    {
        /* Application code */
    }
}
```

You normally only need to call `PAL_Time_Init()` **once**.

---

# 2. Delay

If you need the application to wait for a specific amount of time, use:

```c
PAL_Time_DelayMs(500);
```

The value is specified in milliseconds.

For example:

```c
PAL_Time_DelayMs(100);
```

means:

```text
Wait approximately 100 milliseconds
```

### Example

```c
PAL_GPIO_Set(&LED1);

PAL_Time_DelayMs(500);

PAL_GPIO_Reset(&LED1);

PAL_Time_DelayMs(500);
```

This produces approximately:

```text
LED ON
  |
  | 500 ms
  |
LED OFF
  |
  | 500 ms
  |
LED ON
  |
  ...
```

---

# 3. Getting the Current Time

Use:

```c
uint32_t now = PAL_Time_GetMs();
```

This gives the current system time in milliseconds.

For example:

```c
uint32_t now = PAL_Time_GetMs();
```

The value can be used to determine how much time has passed.

---

# 4. Measuring Elapsed Time

A common use of `PAL_Time_GetMs()` is measuring how long something takes.

```c
uint32_t start = PAL_Time_GetMs();

/* Do some work */

uint32_t elapsed = PAL_Time_GetMs() - start;
```

For example:

```c
uint32_t start = PAL_Time_GetMs();

/* Perform operation */

uint32_t elapsed = PAL_Time_GetMs() - start;

if (elapsed >= 100U)
{
    /* At least 100 ms have passed */
}
```

This is useful for timeouts and periodic operations.

---

# 5. Periodic Operations

If you want something to happen periodically, you can use `PAL_Time_GetMs()` instead of waiting.

Example:

```c
uint32_t last_time = PAL_Time_GetMs();

while (1)
{
    uint32_t now = PAL_Time_GetMs();

    if ((now - last_time) >= 100U)
    {
        last_time = now;

        /* Do something every 100 ms */
    }

    /* Other application code */
}
```

This allows the application to continue doing other work between periodic operations.

---

# 6. Example: Periodic Sensor Reading

Suppose a sensor needs to be read every 50 ms:

```c
uint32_t last_sensor_update = PAL_Time_GetMs();

while (1)
{
    uint32_t now = PAL_Time_GetMs();

    if ((now - last_sensor_update) >= 50U)
    {
        last_sensor_update = now;

        Read_Sensor();
    }

    /* Other application work */
}
```

The sensor update therefore occurs approximately every:

```text
50 ms
```

---

# 7. Example: Multiple Periodic Operations

Different parts of the application can maintain their own timestamps.

```c
uint32_t last_sensor = PAL_Time_GetMs();
uint32_t last_led    = PAL_Time_GetMs();
uint32_t last_uart   = PAL_Time_GetMs();

while (1)
{
    uint32_t now = PAL_Time_GetMs();

    if ((now - last_sensor) >= 10U)
    {
        last_sensor = now;

        /* Sensor update */
    }

    if ((now - last_led) >= 500U)
    {
        last_led = now;

        /* LED update */
    }

    if ((now - last_uart) >= 1000U)
    {
        last_uart = now;

        /* UART update */
    }
}
```

Here:

```text
Sensor -> every 10 ms
LED    -> every 500 ms
UART   -> every 1000 ms
```

All three operations use the same `PAL_Time` module.

---

# 8. Which Function Should I Use?

## I need to wait

Use:

```c
PAL_Time_DelayMs(100);
```

---

## I need the current time

Use:

```c
uint32_t now = PAL_Time_GetMs();
```

---

## I need to measure how long something took

Use:

```c
uint32_t start = PAL_Time_GetMs();

/* Work */

uint32_t elapsed = PAL_Time_GetMs() - start;
```

---

## I need something to happen periodically

Use:

```c
uint32_t last_time = PAL_Time_GetMs();

while (1)
{
    uint32_t now = PAL_Time_GetMs();

    if ((now - last_time) >= period)
    {
        last_time = now;

        /* Do work */
    }
}
```

---

# 9. Blocking vs Periodic Timing

`PAL_Time_DelayMs()` is useful when you simply need to wait:

```c
PAL_Time_DelayMs(500);
```

However, if your application needs to perform multiple activities, timestamp-based timing is usually more appropriate:

```c
if ((PAL_Time_GetMs() - last_time) >= period)
{
    last_time = PAL_Time_GetMs();

    /* Do work */
}
```

For example, instead of:

```c
PAL_Time_DelayMs(100);

Read_Sensor();

PAL_Time_DelayMs(100);

Update_LED();
```

you can structure the application around periodic checks so different activities can run independently.

---

# 10. Quick Reference

### Include

```c
#include "pal_time.h"
```

### Initialize

```c
PAL_Time_Init();
```

Call once during startup.

### Delay

```c
PAL_Time_DelayMs(1000);
```

Wait approximately 1000 ms.

### Get time

```c
uint32_t now = PAL_Time_GetMs();
```

Get the current time in milliseconds.

### Measure elapsed time

```c
uint32_t start = PAL_Time_GetMs();

/* Work */

uint32_t elapsed = PAL_Time_GetMs() - start;
```

### Run something periodically

```c
uint32_t last_time = PAL_Time_GetMs();

while (1)
{
    uint32_t now = PAL_Time_GetMs();

    if ((now - last_time) >= period)
    {
        last_time = now;

        /* Do work */
    }
}
```

---

# 11. What You Need to Know

To use `PAL_Time`, you only need to understand three concepts:

```text
PAL_Time_Init()
        |
        v
Start the timing service
```

```text
PAL_Time_DelayMs(ms)
        |
        v
Wait for the requested time
```

```text
PAL_Time_GetMs()
        |
        v
Get the current time in milliseconds
```

You do **not** need to know how the timing is implemented underneath.

The implementation may use a hardware timer, SysTick, an RTOS timer, or another mechanism in the future. Application code should continue using the same PAL interface.

---

# Mental Model

Think of `PAL_Time` simply as:

```text
                 PAL_Time
                    |
        +-----------+-----------+
        |           |           |
        v           v           v
      Start       Wait       Get Time
       Time        Time       in ms
        |           |           |
        v           v           v
 PAL_Time_Init  DelayMs()   GetMs()
```

> **PAL_Time gives application code a simple way to work with time without requiring knowledge of the underlying platform.**
