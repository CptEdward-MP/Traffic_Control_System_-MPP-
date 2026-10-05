1. What is GPIO?

GPIO stands for General Purpose Input/Output.

It allows the STM32 to use pins as:

Input → read a button/sensor
Output → control an LED, motor, etc.

In our project, we use GPIO to control the STM32 pins without using STM32 HAL.

2. Basic GPIO Process

For a GPIO pin, we follow these steps:

Select GPIO Port + Pin
        ↓
Enable GPIO Clock
        ↓
Configure Pin Mode
        ↓
Configure Pull/Speed
        ↓
Read or Control the Pin

3. Important GPIO Registers
Register	Purpose
MODER	Selects pin mode
OTYPER	Push-pull / open-drain
OSPEEDR	Output speed
PUPDR	Pull-up / pull-down
IDR	Reads pin
ODR	Stores output state
BSRR	Sets or resets output

MODER uses two bits for each pin:
00 → Input
01 → Output
10 → Alternate Function
11 → Analog

4. Enabling the GPIO Clock

Before using GPIOC, its clock must be enabled.

RCC->AHB1ENR |= (1U << 2);

Here, bit 2 = GPIOC clock enable.

Simply:

Turn ON the GPIOC peripheral before using it.
5. Configuring the Pin

For PC13 as an output:

GPIOC->MODER &= ~(3U << 26);
GPIOC->MODER |=  (1U << 26);

This sets:

PC13 → General Purpose Output

We also configure:
OTYPER  → Push-pull
OSPEEDR → Low speed
PUPDR   → No pull

6. Controlling the Pin
Set HIGH
GPIOC->BSRR = (1U << 13);
PC13 → HIGH
Set LOW
GPIOC->BSRR = (1U << 29);
PC13 → LOW
BSRR is divided into two parts:

Lower 16 bits → SET
Upper 16 bits → RESET

So:

Bit 13 → Set PC13
Bit 29 → Reset PC13

Basically:

ODR = output state
BSRR = command to change the output state.
7. Our PAL Layer

Instead of writing register code throughout the application, we created simple functions:

PAL_GPIO_Init();
PAL_GPIO_Set();
PAL_GPIO_Reset();
PAL_GPIO_Write();
PAL_GPIO_Toggle();
PAL_GPIO_Read();

So the application can simply say:
PAL_GPIO_Set(&LED);

instead of directly manipulating BSRR.

The structure is:

Application
     ↓
   PAL GPIO
     ↓
   CMSIS
     ↓
 STM32 Registers
     ↓
  Hardware

This keeps the application code simple and reusable.
8. What We Achieved

We configured an STM32F411 GPIO pin at the register level, without HAL, and used it to control an LED.

The main idea is:

GPIOC
  ↓
Enable Clock
  ↓
Configure MODER
  ↓
Configure GPIO settings
  ↓
Use BSRR
  ↓
Control LED

“We implemented a bare-metal GPIO abstraction layer for the STM32F411 using CMSIS register definitions, allowing GPIO pins to be initialized, read, set, reset and toggled without using STM32 HAL.”

