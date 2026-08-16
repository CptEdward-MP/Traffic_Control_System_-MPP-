# Bare-Metal GPIO — My Discovery Notes V0

**MCU:** STM32F411  
**Topic:** Register-level GPIO without STM32 HAL  
**Purpose:** Preserve the *process of understanding*, not just the final code.

---

## 1. Starting Point

The practical requirement was simple:

> Configure **PC13 as a GPIO output** and make an LED blink without using HAL.

The original CubeIDE-generated code used functions such as:

```c
HAL_Init();
MX_GPIO_Init();
HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
HAL_Delay(1000);
```

Instead of simply replacing these functions, the goal was to discover what the MCU is actually doing underneath.

---

# 2. The Discovery Workflow

The workflow that emerged was:

```text
Physical requirement
        ↓
Identify peripheral + pin
        ↓
Find required clock
        ↓
Find relevant register
        ↓
Find relevant bit/field
        ↓
Understand address + offset
        ↓
Understand CMSIS representation
        ↓
Manipulate register in C
        ↓
Test hardware
        ↓
Create abstraction only after understanding
```

For GPIO:

```text
Need PC13 as output
        ↓
GPIOC, pin 13
        ↓
Enable GPIOC clock
        ↓
Configure MODER13
        ↓
Configure output behavior
        ↓
Use ODR / BSRR
        ↓
Test LED
        ↓
Build PAL GPIO
```

---

# 3. The Three Important Sources

## Reference Manual — RM0383

Used to answer:

> **How does the hardware peripheral work?**

Examples:

- Which register controls GPIO mode?
- Which bit enables GPIOC?
- Which bits belong to PC13?
- What does BSRR do?

---

## STM32F411 CMSIS Device Header — `stm32f411xe.h`

Used to answer:

> **How is the hardware represented in C?**

Examples:

```c
RCC
GPIOC
GPIO_TypeDef
GPIOC_BASE
```

and register members such as:

```c
GPIOC->MODER
GPIOC->ODR
GPIOC->BSRR
```

---

## Datasheet / Board Documentation

Used to answer:

> **What physically exists on the MCU/board?**

Examples:

- Does the package expose PC13?
- What is PC13 connected to?
- Which alternate functions are available?
- What are the electrical characteristics?

---

# 4. What Is a Register?

A peripheral register is a hardware-controlled storage/control location at a particular memory address.

The CPU can access peripheral registers through the MCU memory map.

The key address relationship is:

```text
Peripheral base address
        +
Register offset
        ↓
Register address
```

An **offset is not the complete address**.

It tells us how far a register is from the beginning of its peripheral.

---

# 5. Peripheral Base Address vs Register Offset

The CMSIS header contains:

```c
#define PERIPH_BASE       0x40000000UL

#define AHB1PERIPH_BASE   (PERIPH_BASE + 0x00020000UL)
```

Therefore:

```text
AHB1PERIPH_BASE
= 0x40000000 + 0x00020000
= 0x40020000
```

For GPIOC:

```c
#define GPIOC_BASE (AHB1PERIPH_BASE + 0x0800UL)
```

Therefore:

```text
GPIOC_BASE
= 0x40020000 + 0x0800
= 0x40020800
```

There are therefore two levels of offsets:

```text
AHB1 peripheral region
        ↓
   + 0x0800
        ↓
      GPIOC
        ↓
   + register offset
        ↓
    GPIO register
```

For example:

```text
GPIOC_BASE = 0x40020800

MODER offset = 0x00

GPIOC_MODER
= 0x40020800 + 0x00
= 0x40020800
```

For ODR:

```text
ODR offset = 0x14

GPIOC_ODR
= 0x40020800 + 0x14
= 0x40020814
```

---

# 6. How `GPIOC->MODER` Works

The CMSIS header defines a structure similar to:

```c
typedef struct
{
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    ...
} GPIO_TypeDef;
```

The register layout corresponds to:

```text
Offset       Register
-------------------------
0x00         MODER
0x04         OTYPER
0x08         OSPEEDR
0x0C         PUPDR
0x10         IDR
0x14         ODR
0x18         BSRR
```

Conceptually:

```text
GPIOC
  ↓
GPIOC base address
  ↓
GPIO_TypeDef *
  ↓
MODER member
  ↓
hardware register
```

Therefore:

```c
GPIOC->MODER
```

means:

> Start at the GPIOC peripheral base and access the MODER member of the GPIO register structure.

---

# 7. What CMSIS Saves Us From Doing

Without CMSIS, we could manually access a register using a pointer:

```c
volatile uint32_t *GPIOC_MODER =
    (volatile uint32_t *)0x40020800UL;
```

Then:

```c
*GPIOC_MODER
```

would access the register at that address.

But without CMSIS we would have to manually define/calculcate addresses for:

```text
MODER
OTYPER
OSPEEDR
PUPDR
IDR
ODR
BSRR
...
```

CMSIS instead provides:

```c
GPIOC->MODER
GPIOC->ODR
GPIOC->BSRR
```

The hardware is exactly the same.

CMSIS simply gives C a clean representation of the hardware.

**Important:**

> CMSIS does not create the hardware. It describes the existing hardware registers and addresses in a way C can use.

---

# 8. RCC — Enabling the GPIOC Clock

Before using GPIOC, the GPIOC peripheral clock needs to be enabled.

The reference manual gives:

```text
RCC_AHB1ENR
```

and:

```text
GPIOCEN = bit 2
```

Therefore:

```c
RCC->AHB1ENR |= (1U << 2);
```

can be read as:

```text
RCC
 ↓
AHB1 peripheral clock enable register
 ↓
GPIOCEN
 ↓
bit 2
 ↓
set bit
 ↓
GPIOC clock enabled
```

This was the first direct replacement for:

```c
__HAL_RCC_GPIOC_CLK_ENABLE();
```

---

# 9. Bit Manipulation

The basic operations discovered were:

| C expression | Meaning |
|---|---|
| `x |= (1U << n)` | Set bit `n` |
| `x &= ~(1U << n)` | Clear bit `n` |
| `x ^= (1U << n)` | Toggle bit `n` |
| `x & (1U << n)` | Check whether bit `n` is 1 |

A mask such as:

```c
1U << 3
```

creates:

```text
0000 1000
```

with only bit 3 set.

---

# 10. GPIOC_MODER — Configuring PC13

`GPIOx_MODER` uses **two bits for each pin**.

For pin `y`:

```text
MODER field = bits 2y+1 : 2y
```

For PC13:

```text
2 × 13 = 26
```

Therefore:

```text
MODER13 = bits 27:26
```

The mode values are:

```text
00 → Input
01 → General-purpose output
10 → Alternate function
11 → Analog
```

We want:

```text
PC13 → General-purpose output

MODER13 = 01
```

So:

```c
GPIOC->MODER &= ~(3U << 26);
GPIOC->MODER |=  (1U << 26);
```

The first operation clears both bits:

```text
27 26
 0  0
```

The second inserts:

```text
27 26
 0  1
```

giving:

```text
01 = General-purpose output
```

---

# 11. Other GPIO Configuration Registers

| Register | Purpose | PC13 |
|---|---|---|
| `MODER` | Select input/output/alternate/analog mode | Bits 27:26 |
| `OTYPER` | Push-pull/open-drain | Bit 13 |
| `OSPEEDR` | Output speed | 2-bit field |
| `PUPDR` | No pull/pull-up/pull-down | 2-bit field |
| `IDR` | Read input state | Bit 13 |
| `ODR` | Read/write output state | Bit 13 |
| `BSRR` | Set/reset individual output bits | BS13 / BR13 |

For the simple LED experiment:

```text
OTYPER = push-pull
OSPEEDR = low
PUPDR = no pull
```

were explicitly configured in the working test.

---

# 12. ODR vs BSRR

This was one of the confusing parts initially.

The useful mental model is:

> **ODR represents the output state.**

> **BSRR provides commands to set or reset individual output bits.**

## ODR

`GPIOx_ODR` contains one output-data bit per GPIO pin.

For PC13:

```text
ODR13 = 1 → PC13 HIGH
ODR13 = 0 → PC13 LOW
```

For example:

```c
GPIOC->ODR |= (1U << 13);
```

sets ODR13.

And:

```c
GPIOC->ODR &= ~(1U << 13);
```

clears ODR13.

---

# 13. BSRR

`GPIOx_BSRR` is at offset:

```text
0x18
```

It is divided into two halves:

```text
31                 16 15                  0
┌────────────────────┬────────────────────┐
│ BR15 ... BR0       │ BS15 ... BS0       │
│ RESET              │ SET                │
└────────────────────┴────────────────────┘
```

Lower 16 bits:

```text
BS0 ... BS15
```

are **set commands**.

Upper 16 bits:

```text
BR0 ... BR15
```

are **reset commands**.

---

## Set PC13

We need:

```text
BS13 = 1
```

Therefore:

```c
GPIOC->BSRR = (1U << 13);
```

Meaning:

```text
BS13 = 1
 ↓
Set ODR13
 ↓
PC13 HIGH
```

---

## Reset PC13

We need:

```text
BR13 = 1
```

The reset half starts at bit 16.

Therefore:

```text
13 + 16 = 29
```

So:

```c
GPIOC->BSRR = (1U << 29);
```

Meaning:

```text
BR13 = 1
 ↓
Reset ODR13
 ↓
PC13 LOW
```

---

# 14. Why BSRR Is Useful

With ODR, changing one bit usually involves a read-modify-write operation:

```text
Read ODR
   ↓
modify bit
   ↓
Write ODR
```

BSRR gives the GPIO hardware a direct set/reset command:

```text
Write BSRR
   ↓
Hardware sets/resets selected ODR bit
```

This is why the manual describes BSRR as supporting **atomic bit set/reset**.

The important intuition is:

```text
ODR  → state
BSRR → command to change state
```

---

# 15. First Working Bare-Metal GPIO Program

The working version was:

```c
#include "stm32f411xe.h"

static void GPIOC_Init(void)
{
    /* Enable GPIOC clock */
    RCC->AHB1ENR |= (1U << 2);

    /* PC13 = General Purpose Output */
    GPIOC->MODER &= ~(3U << 26);
    GPIOC->MODER |=  (1U << 26);

    /* Push-pull */
    GPIOC->OTYPER &= ~(1U << 13);

    /* Low speed */
    GPIOC->OSPEEDR &= ~(3U << 26);

    /* No pull-up / pull-down */
    GPIOC->PUPDR &= ~(3U << 26);
}

static void delay(void)
{
    for (volatile uint32_t i = 0; i < 10000000U; i++)
    {
    }
}

int main(void)
{
    GPIOC_Init();

    while (1)
    {
        /* PC13 LOW */
        GPIOC->BSRR = (1U << 29);

        delay();

        /* PC13 HIGH */
        GPIOC->BSRR = (1U << 13);

        delay();
    }
}
```

This worked as the first register-level GPIO test.

---

# 16. Why the Delay Is Not a Real Time Unit

The temporary delay was:

```c
for (volatile uint32_t i = 0; i < 10000000U; i++)
{
}
```

This works as a rough visible delay, but:

```text
1,000,000 iterations ≠ fixed time
```

The duration depends on:

- CPU clock frequency
- compiler optimization
- generated instructions
- execution conditions

Therefore the next natural topic is:

```text
Clock system
    ↓
CPU frequency
    ↓
SysTick / timer
    ↓
real time base
```

For the GPIO discovery stage, the busy loop was enough.

---

# 17. General GPIO Workflow

This is the reusable procedure to use whenever working with GPIO:

```text
1. Define physical requirement
        ↓
2. Identify port + pin
        ↓
3. Check physical pin/board information
        ↓
4. Find GPIO peripheral base
        ↓
5. Enable GPIO peripheral clock
        ↓
6. Configure MODER
        ↓
7. Configure OTYPER if needed
        ↓
8. Configure OSPEEDR if needed
        ↓
9. Configure PUPDR if needed
        ↓
10. Configure AFR if using alternate function
        ↓
11. Choose IDR / ODR / BSRR operation
        ↓
12. Manipulate required bits/fields
        ↓
13. Test hardware
```

---

# 18. The Deeper Pattern

The important thing learned was not simply:

> "How to blink PC13."

The reusable engineering pattern is:

```text
Requirement
    ↓
Peripheral
    ↓
Clock
    ↓
Register
    ↓
Bit / Field
    ↓
C representation
    ↓
Hardware behavior
```

This same pattern should be reusable for:

```text
GPIO
Timer
UART
SPI
I2C
ADC
DMA
...
```

The registers change.

The **discovery process remains similar**.

---

# 19. PAL — Pin Abstraction Layer

After understanding the register-level implementation, the next practical step was to create a thin **PAL (Pin Abstraction Layer)** so teammates can use GPIO without repeatedly writing register calculations.

The desired architecture is:

```text
Application
      ↓
PAL GPIO
      ↓
STM32 CMSIS register definitions
      ↓
Hardware
```

The PAL should hide details such as:

```text
MODER
BSRR
ODR
RCC clock bits
```

from application code.

The application can instead use functions such as:

```c
PAL_GPIO_Init();
PAL_GPIO_Set();
PAL_GPIO_Reset();
PAL_GPIO_Write();
PAL_GPIO_Toggle();
PAL_GPIO_Read();
```

A pin can be represented using:

```c
typedef struct
{
    GPIO_TypeDef *port;
    uint8_t pin;
} PAL_GPIO_Pin_t;
```

This allows the same API to operate on:

```text
GPIOA
GPIOB
GPIOC
GPIOD
GPIOE
GPIOH
```

without creating separate functions for every physical pin.

---

# 20. What I Should Remember

- A register is a hardware control/status location at a specific address.
- A peripheral base address tells where a peripheral starts.
- A register offset tells where a register is inside that peripheral.
- `base + offset = register address`.
- CMSIS provides C definitions for the MCU's register layout and addresses.
- Using CMSIS does **not** mean using HAL.
- `RCC_AHB1ENR` controls AHB1 peripheral clocks.
- GPIOC clock enable is bit 2.
- PC13 uses MODER bits 27:26.
- `01` in MODER13 selects general-purpose output.
- `ODR` represents output state.
- `BSRR` provides individual set/reset commands.
- BSRR bit 13 sets PC13.
- BSRR bit 29 resets PC13.
- For multi-bit fields, clear the field and then insert the desired value.
- A busy loop is a temporary delay, not a reliable time base.
- The goal is to understand the hardware first and abstract it afterward.

---

# 21. Study Framework Record

This GPIO topic followed the intended study framework:

| Framework part | GPIO example |
|---|---|
| What is it + intuition | Registers are hardware storage/control locations; base + offset identifies them; bit manipulation changes fields. |
| Practical application | Configure PC13 and blink an LED without HAL. |
| Documentation/source | RM0383 + STM32F411 CMSIS header + datasheet/board information. |
| Future deep dive | Clock tree, SysTick, alternate functions, atomic operations, GPIO electrical behavior, PAL evolution. |

---

## End of V0

> **Preserve the discovery process, not just the final code.**
