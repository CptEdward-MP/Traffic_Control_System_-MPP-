# PAL ADC Driver – STM32F411

## 1. Overview

This document explains how the `PAL_ADC` driver was designed and how to use it on the STM32F411.

The driver was written using the STM32 CMSIS device header and direct peripheral-register access rather than the STM32 HAL/LL ADC APIs.

The goal was to create a small, reusable ADC abstraction:

```text
Application
    |
    v
 PAL_ADC
    |
    v
STM32F411 ADC1 registers
```

The GPIO configuration is kept separate from the ADC driver. `PAL_GPIO` is responsible for configuring the ADC input pin as analog, while `PAL_ADC` is responsible for ADC1 configuration and conversion.

### Current design

- MCU: STM32F411
- ADC peripheral: ADC1
- Resolution: 12-bit
- Trigger: software (`SWSTART`)
- Initial mode: polling
- DMA: disabled
- ADC interrupts: disabled
- Data alignment: right aligned
- Initial test: one channel
- Next extension: six-channel regular sequence and continuous conversion

> **Note:** The surrounding application may still use STM32 HAL for other peripherals and system initialization. This driver specifically avoids HAL for the ADC peripheral.

---

# 2. Driver Files

The driver consists of:

```text
pal_adc.h
pal_adc.c
```

## `pal_adc.h`

The header exposes the public ADC interface:

```c
#ifndef PAL_ADC_H
#define PAL_ADC_H

#include "stm32f411xe.h"
#include <stdint.h>
#include <stdbool.h>

void PAL_ADC1_Init(void);
bool PAL_ADC1_ReadChannel(uint8_t channel, uint16_t *value);

#endif /* PAL_ADC_H */
```

The application should include only this header rather than directly manipulating ADC registers.

---

# 3. Why CMSIS Register Access?

Instead of using functions such as HAL ADC initialization and HAL ADC conversion APIs, the driver accesses the peripheral registers directly:

```c
ADC1->CR1
ADC1->CR2
ADC1->SR
ADC1->SMPR1
ADC1->SMPR2
ADC1->SQR1
ADC1->SQR2
ADC1->SQR3
ADC1->DR
```

CMSIS provides the `ADC1` structure and register definitions through:

```c
#include "stm32f411xe.h"
```

This lets us write code that corresponds directly to the STM32 reference manual.

For example:

```c
ADC1->CR2 |= (1U << 0);
```

means setting the `ADON` bit in `ADC_CR2`.

The benefit for this driver is that every configuration decision can be traced directly to a register field in the reference manual.

---

# 4. Step 1 – Configure the ADC GPIO

An ADC input is not a normal digital input. The GPIO must be put into analog mode.

For ADC1 channel 0, the input used during the initial test was PA0.

The application creates a `PAL_GPIO_Pin_t` object:

```c
PAL_GPIO_Pin_t adc_pin = {
    .port = GPIOA,
    .pin  = 0
};
```

Then it configures the pin:

```c
PAL_GPIO_Init(&adc_pin,
              PAL_GPIO_MODE_ANALOG,
              PAL_GPIO_NOPULL,
              PAL_GPIO_SPEED_LOW);
```

The important configuration is:

```c
MODER = 11b
PUPDR = 00b
```

for the selected pin.

The GPIO driver was also designed so that analog mode forces no pull-up/pull-down:

```c
if (mode == PAL_GPIO_MODE_ANALOG)
{
    gpio->port->MODER |= (3U << shift);
    gpio->port->PUPDR &= ~(3U << shift);
}
```

This keeps GPIO responsibility inside `PAL_GPIO` instead of duplicating GPIO register manipulation inside `PAL_ADC`.

---

# 5. Step 2 – Enable the ADC Clock

Before accessing ADC1, its peripheral clock must be enabled.

From the STM32F411 reference manual:

```text
RCC_APB2ENR
        |
        +-- bit 8 = ADC1EN
```

Therefore the driver contains:

```c
static void PAL_ADC1_EnableClock(void)
{
    RCC->APB2ENR |= (1U << 8);
}
```

and `PAL_ADC1_Init()` calls it first:

```c
void PAL_ADC1_Init(void)
{
    PAL_ADC1_EnableClock();
    ...
}
```

### Why ADC1 owns its clock

The responsibility is intentionally kept with the ADC driver:

```text
PAL_GPIO
    -> enables GPIO clock

PAL_ADC
    -> enables ADC1 clock
```

This prevents the application from having to know which RCC register and bit belong to ADC1.

---

# 6. Step 3 – ADC Resolution

The STM32F411 ADC supports configurable resolution through `ADC_CR1.RES`.

For the initial driver we use the reset configuration:

```text
RES = 00
```

which corresponds to 12-bit resolution.

Therefore no explicit write is required in the first version.

The resulting conversion range is:

```text
0 ... 4095
```

For example, approximately:

```text
0 V       -> 0
1.65 V    -> 2048
3.3 V     -> 4095
```

The exact voltage relationship depends on the ADC reference/supply conditions and the actual analog input.

---

# 7. Step 4 – Configure End-of-Conversion Behavior

The driver uses polling rather than interrupts.

The important status flag is:

```text
ADC_SR.EOC
```

The driver enables per-conversion EOC behavior using:

```c
ADC1->CR2 |= (1U << 10);
```

This sets:

```text
EOCS = 1
```

With this configuration, the `EOC` flag is generated for each regular conversion.

The application can therefore wait for a conversion using:

```c
while ((ADC1->SR & (1U << 1)) == 0U)
{
}
```

where bit 1 is `EOC`.

After the conversion completes, reading `ADC1->DR` obtains the converted value.

---

# 8. Step 5 – Configure the Regular Conversion Sequence

The ADC does not simply receive a channel number and immediately convert it. It has a regular conversion sequence.

The sequence registers determine which channel is converted and in what order.

For a single conversion:

```text
SQR1.L = 0000
```

which means one regular conversion.

The first conversion is selected through:

```text
SQR3.SQ1
```

For channel 0:

```c
ADC1->SQR3 &= ~(0x1FU << 0);
```

Since channel 0 is represented by zero, no set operation is necessary after clearing it.

Conceptually:

```text
SQR1
+-----------------------+
| L = 0 -> 1 conversion |
+-----------------------+

SQR3
+----------------+
| SQ1 = channel 0|
+----------------+
```

---

# 9. Step 6 – Configure Sampling Time

The ADC needs a sampling period before performing the conversion.

For ADC channels 0–9, the sampling time is configured through `ADC_SMPR2`.

For channel 0, the driver selected 84 ADC cycles:

```c
ADC1->SMPR2 &= ~(7U << 0);
ADC1->SMPR2 |=  (4U << 0);
```

The field value is:

```text
100b
```

which corresponds to 84 cycles according to the reference manual.

The important point is that **sampling time belongs to the ADC channel configuration**, not the sequence itself.

When the driver is expanded to arbitrary channels, the sampling-time configuration must be handled for each channel that is used.

---

# 10. Step 7 – Software Start

The initial driver uses a software trigger rather than an external trigger.

`ADC_CR2.SWSTART` is bit 30.

A conversion is started with:

```c
ADC1->CR2 |= (1U << 30);
```

The sequence is therefore:

```text
Configure ADC
      |
      v
Set SWSTART
      |
      v
ADC performs conversion
      |
      v
EOC = 1
      |
      v
Read ADC_DR
```

---

# 11. Step 8 – Polling the Conversion

The first version intentionally avoids interrupts and DMA.

The basic polling operation is:

```c
ADC1->CR2 |= (1U << 30);

while ((ADC1->SR & (1U << 1)) == 0U)
{
}

*value = (uint16_t)ADC1->DR;
```

This makes the operation easy to understand:

1. Start conversion.
2. Wait until `EOC` becomes 1.
3. Read the data register.
4. Return the result.

---

# 12. Current Single-Channel Read API

The public API is:

```c
bool PAL_ADC1_ReadChannel(uint8_t channel, uint16_t *value);
```

The channel is selected dynamically by writing `SQR3.SQ1`:

```c
ADC1->SQR3 &= ~(0x1FU << 0);
ADC1->SQR3 |= ((uint32_t)channel << 0);
```

Then the conversion is started and polled:

```c
ADC1->CR2 |= (1U << 30);

while ((ADC1->SR & (1U << 1)) == 0U)
{
}

*value = (uint16_t)ADC1->DR;
```

The complete current concept is:

```c
bool PAL_ADC1_ReadChannel(uint8_t channel, uint16_t *value)
{
    if (value == NULL)
    {
        return false;
    }

    ADC1->SQR3 &= ~(0x1FU << 0);
    ADC1->SQR3 |= ((uint32_t)channel << 0);

    ADC1->CR2 |= (1U << 30);

    while ((ADC1->SR & (1U << 1)) == 0U)
    {
    }

    *value = (uint16_t)ADC1->DR;

    return true;
}
```

---

# 13. Using the Driver in `main.c`

Include the driver header:

```c
#include "pal_adc.h"
#include "pal_gpio.h"
```

Configure the ADC input pin:

```c
PAL_GPIO_Pin_t adc_pin = {
    .port = GPIOA,
    .pin  = 0
};

PAL_GPIO_Init(&adc_pin,
              PAL_GPIO_MODE_ANALOG,
              PAL_GPIO_NOPULL,
              PAL_GPIO_SPEED_LOW);
```

Initialize ADC1:

```c
PAL_ADC1_Init();
```

Create a variable for the conversion result:

```c
uint16_t value;
```

Read the channel:

```c
PAL_ADC1_ReadChannel(0, &value);
```

For continuous testing, the application can repeatedly read and transmit the value:

```c
while (1)
{
    PAL_ADC1_ReadChannel(0, &value);

    USB_SendInt(value);

    HAL_Delay(100);
}
```

Here `HAL_Delay()` and `USB_SendInt()` are application-side facilities. Their use does not make the ADC driver itself HAL-based.

---

# 14. Driver Architecture

The intended separation is:

```text
                    Application
                        |
                        v
              +-------------------+
              |     PAL_ADC       |
              +-------------------+
                 |             |
                 v             v
             ADC1 regs      RCC regs

                    +
                    |
                    v
              STM32F411 HW
```

GPIO is separate:

```text
Application
    |
    +----> PAL_GPIO ----> GPIOA registers
    |
    +----> PAL_ADC  ----> ADC1 registers
```

This prevents the ADC driver from becoming responsible for general GPIO configuration.

---

# 15. Extending to Six Channels

The next planned extension is a six-channel regular sequence.

The STM32 ADC uses `SQR1.L` to specify the number of conversions.

For six conversions:

```c
ADC1->SQR1 &= ~(0xFU << 20);
ADC1->SQR1 |=  (5U << 20);
```

The value is `5` because the encoded length is:

```text
L = number of conversions - 1
```

The first six sequence positions are stored in `SQR3`:

```text
SQ1 -> bits 4:0
SQ2 -> bits 9:5
SQ3 -> bits 14:10
SQ4 -> bits 19:15
SQ5 -> bits 24:20
SQ6 -> bits 29:25
```

For channels 0–5:

```c
ADC1->SQR3 = 0U;

ADC1->SQR3 |= (0U << 0);     /* SQ1 = CH0 */
ADC1->SQR3 |= (1U << 5);     /* SQ2 = CH1 */
ADC1->SQR3 |= (2U << 10);    /* SQ3 = CH2 */
ADC1->SQR3 |= (3U << 15);    /* SQ4 = CH3 */
ADC1->SQR3 |= (4U << 20);    /* SQ5 = CH4 */
ADC1->SQR3 |= (5U << 25);    /* SQ6 = CH5 */
```

The resulting sequence is:

```text
CH0 -> CH1 -> CH2 -> CH3 -> CH4 -> CH5
```

---

# 16. Continuous Conversion

Continuous conversion is controlled by `ADC_CR2.CONT`, bit 1.

Set it with:

```c
ADC1->CR2 |= (1U << 1);
```

Then one software start begins repeated sequences:

```text
       +-----------------------------------+
       |                                   |
       v                                   |
CH0 -> CH1 -> CH2 -> CH3 -> CH4 -> CH5 ---+
```

The application no longer needs to issue `SWSTART` after every conversion sequence.

The intended operation is:

```text
SWSTART
   |
   v
CH0 -> EOC -> DR
   |
   v
CH1 -> EOC -> DR
   |
   v
CH2 -> EOC -> DR
   |
   v
CH3 -> EOC -> DR
   |
   v
CH4 -> EOC -> DR
   |
   v
CH5 -> EOC -> DR
   |
   +------> repeat
```

Because `EOCS = 1`, polling `EOC` can be used to obtain each conversion result.

---

# 17. Important Difference: Single Read vs Continuous Sequence

The current single-channel API:

```c
PAL_ADC1_ReadChannel(channel, &value);
```

is appropriate when the application wants to select one channel and perform one conversion.

For continuous multi-channel operation, a sequence-based API is more appropriate, for example:

```c
PAL_ADC1_StartContinuous();
PAL_ADC1_ReadSequence(values);
```

where:

```c
uint16_t values[6];
```

contains the six most recently consumed conversion results.

This avoids repeatedly changing `SQR3` while the ADC is already running.

---

# 18. Recommended Development Order

The driver was intentionally developed incrementally.

### Stage 1 – GPIO

Create a reusable GPIO abstraction:

```text
PAL_GPIO
```

Verify that an ADC input can be configured as analog.

### Stage 2 – ADC clock

Find `ADC1EN` in `RCC_APB2ENR` and enable it directly.

### Stage 3 – ADC configuration

Configure:

- resolution
- EOC behavior
- sequence length
- channel
- sampling time

### Stage 4 – Single conversion

Use:

```text
SWSTART
   |
   v
EOC polling
   |
   v
DR
```

### Stage 5 – Dynamic channel selection

Move channel selection into the read function using `SQR3.SQ1`.

### Stage 6 – Six-channel sequence

Configure `SQR1.L = 5` and `SQ1` through `SQ6`.

### Stage 7 – Continuous conversion

Set `CR2.CONT = 1` and start the ADC once.

### Stage 8 – Sequence polling API

Read six results into an application buffer.

### Future stages

Possible future extensions include:

- channel-specific sampling-time configuration
- validation of channel numbers
- timeout handling instead of infinite polling
- ADC calibration/startup handling if required by the target configuration
- interrupt-driven conversion
- DMA-based sampling
- configurable number of channels
- configurable sequences

These are intentionally separate from the first polling implementation.

---

# 19. Important Design Lessons

## 19.1 Start from the reference manual

The driver was built by identifying the hardware operation first and then mapping each operation to a register field.

For example:

```text
Need ADC1 clock
       |
       v
RCC_APB2ENR.ADC1EN

Need software trigger
       |
       v
ADC_CR2.SWSTART

Need conversion complete indication
       |
       v
ADC_SR.EOC

Need result
       |
       v
ADC_DR
```

This is preferable to copying a large HAL implementation when the purpose is learning and building a low-level abstraction.

## 19.2 Keep hardware responsibilities local

`PAL_ADC` owns ADC configuration.

`PAL_GPIO` owns GPIO configuration.

The application should ideally only express intent:

```c
PAL_GPIO_Init(...);
PAL_ADC1_Init();
PAL_ADC1_ReadChannel(...);
```

rather than manipulating registers everywhere in application code.

## 19.3 Build one working path before adding complexity

The driver was first proven with one ADC channel and polling.

Only after that worked should the design be extended to multiple channels and continuous conversion.

This makes debugging significantly easier because each new feature introduces only a small number of new variables.

---

# 20. Quick Reference

## Initialization

```c
PAL_GPIO_Pin_t adc_pin = {
    .port = GPIOA,
    .pin  = 0
};

PAL_GPIO_Init(&adc_pin,
              PAL_GPIO_MODE_ANALOG,
              PAL_GPIO_NOPULL,
              PAL_GPIO_SPEED_LOW);

PAL_ADC1_Init();
```

## Single-channel read

```c
uint16_t value;

PAL_ADC1_ReadChannel(0, &value);
```

## Output example

```c
USB_SendInt(value);
```

## Register map used by the driver

| Register | Purpose |
|---|---|
| `RCC->APB2ENR` | Enable ADC1 clock |
| `ADC1->CR1` | Resolution / scan configuration |
| `ADC1->CR2` | Trigger, continuous mode, EOC behavior, ADC enable |
| `ADC1->SR` | Conversion status (`EOC`) |
| `ADC1->SMPR1` | Sampling times for higher ADC channels |
| `ADC1->SMPR2` | Sampling times for lower ADC channels |
| `ADC1->SQR1` | Regular sequence length |
| `ADC1->SQR2` | Higher sequence positions |
| `ADC1->SQR3` | First six sequence positions |
| `ADC1->DR` | Conversion result |

---

# 21. Summary

The `PAL_ADC` driver was built from the STM32F411 ADC register model rather than from HAL ADC APIs.

The basic polling path is:

```text
GPIO analog configuration
          |
          v
Enable ADC1 clock
          |
          v
Configure ADC
          |
          v
Select channel / sequence
          |
          v
Software start
          |
          v
Poll EOC
          |
          v
Read DR
          |
          v
Return ADC value
```

The next evolution is a six-channel continuous sequence:

```text
          SWSTART
             |
             v
   +-----------------------+
   | CH0 CH1 CH2 CH3 CH4 CH5 |
   +-----------------------+
             |
             +------ repeat
```

The important design principle is to keep the application independent of the ADC register details while keeping the driver itself simple enough that every operation can be traced back to the STM32F411 reference manual.
