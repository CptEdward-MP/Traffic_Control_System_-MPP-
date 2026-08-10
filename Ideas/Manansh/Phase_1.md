# Phase 1 — CMSIS-Based Bare-Metal Peripheral Development

## Purpose

Before building the Intelligent Traffic Control System, all four team members will first learn the STM32F411 by developing the peripheral functionality ourselves using **CMSIS and direct register-level programming**.

The college has specified that **STM32 HAL is not allowed**.

The purpose of this phase is therefore not simply to make reusable functions. The main goal is to understand:

- How the STM32F411 is structured.
- How the Cortex-M4 interacts with peripherals.
- How peripheral clocks are enabled.
- How registers configure peripherals.
- How data moves between the CPU, peripherals, and physical pins.
- How interrupts reach the CPU.
- How timers generate timing and PWM.
- How communication peripherals actually transmit and receive data.

At the end of Phase 1, all four members should understand the work done by every member, not only their own assigned topic.

---

# 1. What We Are Building

We will create a small **CMSIS-based peripheral library** that will later be used by the actual traffic-control application.

> We should avoid calling this a "HAL" in project documentation because the college has explicitly prohibited HAL. The purpose is still to create our own reusable peripheral layer, but it will be implemented directly using CMSIS/register-level programming.

The final project hardware currently includes:

- STM32F411
- ToF sensors using SPI
- LCD
- TTL UART connection to a PC
- Servo motor
- LEDs
- Push buttons for pedestrian interaction

This means Phase 1 should focus primarily on the peripherals that the final project actually needs.

---

# 2. Important Rule — No STM32 HAL

Do **not** use STM32 HAL functions such as:

```c
HAL_GPIO_WritePin(...);
HAL_UART_Transmit(...);
HAL_TIM_PWM_Start(...);
HAL_SPI_Transmit(...);
HAL_I2C_Master_Transmit(...);
```

The implementation should instead use the CMSIS device definitions and the STM32 registers.

For example:

```c
GPIOA->MODER
GPIOA->ODR
GPIOA->IDR

USART2->CR1
USART2->BRR
USART2->ISR
USART2->TDR

TIM3->PSC
TIM3->ARR
TIM3->CCR1
```

The exact registers depend on the peripheral and configuration.

The objective is to understand **why** each register is being configured, not just to memorize register names.

---

# 3. How to Think About CMSIS

The basic relationship should be understood as:

```text
Your C code
     ↓
CMSIS device definitions
     ↓
STM32 peripheral registers
     ↓
Peripheral hardware
     ↓
Physical pins / external device
```

CMSIS gives us standardized definitions for the Cortex-M core and STM32 device registers.

For example, instead of manually defining the address of a GPIO register, we can use:

```c
GPIOA->MODER
```

The important part is understanding what that register actually does in the hardware.

---

# 4. Phase 1 Learning Order

The topics are arranged from simpler concepts toward more difficult ones.

```text
1. RCC / Clock
       ↓
2. GPIO
       ↓
3. SysTick / Basic Timing
       ↓
4. UART
       ↓
5. Timers
       ↓
6. PWM
       ↓
7. EXTI + NVIC
       ↓
8. SPI
       ↓
9. I²C (only if required by the LCD)
```

### Why this order?

GPIO teaches the basic register configuration pattern.

SysTick introduces timing and the Cortex-M core.

UART is introduced early because it becomes a useful debugging tool for the rest of the project.

Timers are required to understand PWM.

PWM is required for the servo.

EXTI and NVIC introduce the ARM interrupt architecture.

SPI is required for the ToF sensors.

I²C is required only if the selected LCD uses I²C.

---

# 5. Topics That Are Not Currently Required

We should not implement every STM32 peripheral simply because it exists.

Unless the final hardware requires them, the following are not currently part of the mandatory Phase 1 scope:

- ADC
- DMA
- CAN
- USB
- I²S
- Advanced timer features

They can be studied later if the project requires them.

---

# 6. Work Distribution

The four members will have primary ownership of different areas.

The assigned member is the **primary implementer and teacher**, not the only person who needs to understand the topic.

---

## Member 1 — MCU Fundamentals & GPIO

### Topics

- STM32F411 basic architecture
- RCC
- Peripheral clocks
- GPIO
- SysTick / basic timing

### Responsibilities

- Understand the STM32 clock system.
- Understand why peripherals require clocks.
- Implement GPIO initialization.
- Implement GPIO input/output functionality.
- Implement LED control.
- Understand GPIO registers:
  - `MODER`
  - `OTYPER`
  - `OSPEEDR`
  - `PUPDR`
  - `IDR`
  - `ODR`
  - `BSRR`
- Implement basic timing/delay using SysTick.
- Demonstrate GPIO input using a push button.

### Example API

The final API can be designed by the member, but a simple example could be:

```c
void GPIO_Init_Output(GPIO_TypeDef *port, uint8_t pin);

void GPIO_Init_Input(GPIO_TypeDef *port, uint8_t pin);

void GPIO_Write(GPIO_TypeDef *port, uint8_t pin, uint8_t state);

void GPIO_Toggle(GPIO_TypeDef *port, uint8_t pin);

uint8_t GPIO_Read(GPIO_TypeDef *port, uint8_t pin);
```

These are examples only. The member should decide the appropriate interface after understanding the peripheral.

### Example Test

```text
GPIOA Pin 5 → LED

LED ON
   ↓
500 ms
   ↓
LED OFF
   ↓
500 ms
   ↓
repeat
```

Then test:

```text
Button → GPIO Input → LED
```

### Member 1 should be able to explain

- Why the GPIO clock must be enabled.
- What `MODER` does.
- Difference between `IDR` and `ODR`.
- Why `BSRR` can be useful.
- How a physical button becomes a digital value.
- How SysTick produces a time base.

---

# 7. Member 2 — Timers & PWM

### Topics

- Timer architecture
- Timer clock
- Prescaler
- Counter
- Auto-reload register
- Compare registers
- PWM generation

### Responsibilities

- Understand the STM32 timer peripheral.
- Implement basic timer configuration.
- Generate periodic timing.
- Implement PWM.
- Control a servo using PWM.
- Experiment with different PWM settings.

### Important Concept

The member should understand:

```text
Timer Clock
     ↓
Prescaler
     ↓
Counter
     ↓
ARR
     ↓
Update / Compare
     ↓
PWM
```

### Important Registers

Examples include:

| Register | Purpose |
|---|---|
| `PSC` | Prescaler |
| `ARR` | Auto-reload value |
| `CNT` | Counter |
| `CCR` | Compare value |
| `CCMR` | Capture/compare configuration |
| `CCER` | Channel enable/configuration |

### Example API

```c
void Timer_Init(...);

void Timer_Start(...);

void PWM_Init(...);

void PWM_SetDutyCycle(...);
```

The exact API should be decided after understanding the timer.

### Servo Test

The member should demonstrate several positions:

```text
PWM setting 1 → Servo position 1

PWM setting 2 → Servo position 2

PWM setting 3 → Servo position 3
```

They should calculate and explain the timer configuration rather than simply trying values until the servo moves.

### Member 2 should be able to explain

- What the timer clock is.
- What a prescaler does.
- How `PSC` affects the counter.
- What `ARR` controls.
- What `CCR` controls.
- How PWM frequency is calculated.
- How duty cycle is generated.
- Why changing the pulse width changes the servo position.

---

# 8. Member 3 — UART & SPI

## UART

### Responsibilities

- Understand USART/UART architecture.
- Configure UART using registers.
- Configure GPIO alternate functions.
- Configure baud rate.
- Implement character transmission.
- Implement string transmission.
- Implement reception.
- Connect STM32 to a PC through a TTL-UART interface.

### Hardware

```text
STM32F411
     │
     │ UART TX/RX
     ▼
TTL → USB Converter
     │
     ▼
PC Serial Terminal
```

### Example

```c
UART_Init(...);

UART_SendChar('A');

UART_SendString("Hello STM32\r\n");
```

### Expected test

```text
Hello STM32
UART test successful
```

UART will later be used for project debugging.

Example future output:

```text
Vehicle detected
Lane: 2
Distance: 243 mm
Vehicle count: 4
```

---

## SPI

### Responsibilities

- Understand SPI master mode.
- Understand SCK, MOSI, MISO and CS.
- Configure SPI using registers.
- Implement SPI transmit/receive.
- Implement chip-select control.
- Test communication with the selected ToF sensor.

### Basic SPI connection

```text
STM32                         ToF Sensor

MOSI ───────────────────────► MOSI

MISO ◄─────────────────────── MISO

SCK  ───────────────────────► SCK

CS   ───────────────────────► CS
```

### Concepts to understand

- Master/slave
- Full duplex
- Clock polarity (CPOL)
- Clock phase (CPHA)
- SPI clock speed
- Status flags
- Data register
- Chip select

The exact SPI mode and transaction sequence must come from the selected ToF sensor's datasheet.

### Example API

```c
void SPI_Init(...);

uint8_t SPI_Transfer(uint8_t data);
```

Again, these are examples and not mandatory APIs.

### Member 3 should be able to explain

For UART:

- What baud rate means.
- Which clock powers the USART.
- How baud rate is configured.
- How transmission occurs.
- How reception occurs.
- What the status flags mean.

For SPI:

- What each SPI signal does.
- Who generates the clock.
- What CPOL and CPHA mean.
- How one SPI transaction works.
- How chip select is controlled.
- How the MCU knows data has been transmitted/received.

---

# 9. Member 4 — EXTI, NVIC & I²C

## EXTI + NVIC

### Topics

- Interrupts
- Cortex-M exception/interrupt mechanism
- NVIC
- EXTI
- GPIO interrupts
- Interrupt handlers

### Responsibilities

- Understand how a peripheral event reaches the CPU.
- Configure external interrupts.
- Configure GPIO interrupt sources.
- Use NVIC.
- Implement a pedestrian push-button interrupt.
- Handle the interrupt flag correctly.
- Implement or demonstrate button debouncing.

### Basic flow

```text
Push Button
     ↓
GPIO
     ↓
EXTI
     ↓
NVIC
     ↓
CPU
     ↓
Interrupt Handler
```

### Example structure

```c
void EXTIx_IRQHandler(void)
{
    if (/* interrupt occurred */)
    {
        /* Handle button event */

        /* Clear interrupt flag */
    }
}
```

The exact implementation depends on the GPIO pin and EXTI line selected.

### Member 4 should be able to explain

- What an interrupt is.
- What EXTI does.
- What NVIC does.
- What an interrupt pending flag means.
- How the CPU reaches the interrupt handler.
- Why the interrupt flag must be cleared.
- Difference between polling and interrupts.
- Basic interrupt priority.

---

## I²C

I²C should only be implemented if the selected LCD requires it.

### Responsibilities

- Understand SDA and SCL.
- Understand START and STOP conditions.
- Understand addressing.
- Understand ACK/NACK.
- Configure I²C.
- Implement basic communication.
- Use it to communicate with the LCD if required.

### Basic transaction

```text
START
  ↓
Address
  ↓
ACK
  ↓
Data
  ↓
ACK
  ↓
STOP
```

### Member 4 should be able to explain

- Why I²C uses SDA and SCL.
- What START means.
- What an address is.
- What ACK/NACK means.
- What STOP means.
- How the STM32 communicates with the LCD.

---

# 10. Common Implementation Requirements

Every member must provide the following:

```text
1. Peripheral implementation
2. Header/API
3. Small working example
4. Documentation
5. Test result
6. Short teaching session
```

The implementation should be small enough to understand.

Do not build a huge framework during Phase 1.

The objective is learning and proving that the peripheral works.

---

# 11. Common Documentation Requirements

Every peripheral must have its own Markdown documentation.

Use this structure:

```markdown
# Peripheral Name

## 1. Objective

What are we trying to implement?

## 2. What Is This Peripheral?

Explain the peripheral in your own words.

## 3. STM32F411 Hardware

Which peripheral instance is being used?

Example:
USART2
SPI1
TIM3
GPIOA

## 4. Clock

What clock does the peripheral use?

How is it enabled?

## 5. Important Registers

| Register | Purpose |
|----------|---------|
| Register 1 | ... |
| Register 2 | ... |

## 6. Configuration

Explain the configuration step-by-step.

## 7. Implementation

Explain how the code works.

## 8. Example

Show a small example.

## 9. Testing

### Test 1

Objective:

Expected:

Observed:

Result: PASS / FAIL

## 10. Problems Encountered

Document problems encountered during development.

## 11. What I Learned

Explain the important concepts learned.

## 12. References

List the datasheet/reference manual sections used.
```

---

# 12. Documentation Rule — Explain, Don't Copy

Do not simply copy paragraphs from the STM32 reference manual.

The reference manual might say:

> A particular bit enables a peripheral function.

Your documentation should explain:

> We set this bit because the peripheral must be enabled before the CPU can configure/use it.

The goal is to prove **understanding**, not copying.

Use diagrams, tables and small examples wherever they make the concept easier to understand.

---

# 13. How to Read the STM32 Documentation

This is one of the most important skills for Phase 1.

Do not try to read the entire STM32 reference manual from page 1 to the end.

Instead, read it **for the specific peripheral you are implementing**.

There are three main documents you will use:

### 1. Datasheet

Use it to find:

- Pin functions
- Package information
- Electrical characteristics
- Available peripherals
- Alternate functions
- Memory information

### 2. Reference Manual

Use it to understand:

- Peripheral architecture
- Registers
- Bit fields
- Configuration sequence
- Peripheral behaviour

This is usually your main document for driver development.

### 3. Cortex-M / ARM documentation

Use it when studying:

- CPU architecture
- Exceptions
- Interrupts
- NVIC
- SysTick
- Core registers

---

# 14. How to Read a Peripheral Section

Suppose you are implementing SPI.

Do not immediately jump into the code.

Follow this process:

```text
1. Find SPI chapter
        ↓
2. Read "Introduction"
        ↓
3. Understand how SPI works
        ↓
4. Find clock information
        ↓
5. Find GPIO/alternate-function requirements
        ↓
6. Find configuration registers
        ↓
7. Understand important bits
        ↓
8. Find status flags
        ↓
9. Find transmission/reception procedure
        ↓
10. Write your own implementation
```

Do the same for GPIO, UART, timers, EXTI, etc.

---

# 15. How to Read a Register Description

When you encounter something like:

```text
CR1
```

do not just write:

```c
SPI1->CR1 = 0x0001;
```

First understand:

```text
CR1
 │
 ├── Bit 0 → What does it do?
 ├── Bit 1 → What does it do?
 ├── Bit 2 → What does it do?
 └── ...
```

Then determine which bits you actually need.

Prefer code where the reason is visible.

For example, conceptually:

```c
SPI1->CR1 |= /* enable SPI */;
```

is much easier to understand than blindly writing a magic hexadecimal value.

---

# 16. Use the Reference Manual Together With the Datasheet

For example, the reference manual may tell you:

```text
USART2 requires a particular configuration.
```

But the datasheet tells you which physical pins can be used for USART2.

So use them together:

```text
Reference Manual
    ↓
How the peripheral works

Datasheet
    ↓
Where the peripheral can appear physically
```

---

# 17. How to Find Information Efficiently

When reading a large PDF, use search.

Useful search terms include:

```text
"clock enable"
"register"
"configuration"
"bit"
"status flag"
"alternate function"
"transmit"
"receive"
"interrupt"
"prescaler"
"baud rate"
"PWM"
```

Do not read hundreds of pages unnecessarily.

Find the section you need, understand it, then implement it.

---

# 18. What Each Member Should Record While Reading

Before coding, make a small table.

Example:

| Question | Answer |
|---|---|
| Which peripheral am I using? | SPI1 |
| Which clock powers it? | ... |
| Which GPIO pins are needed? | ... |
| Which registers configure it? | ... |
| Which bits are important? | ... |
| What starts the peripheral? | ... |
| How is data transmitted? | ... |
| How do I know it finished? | ... |
| What errors can occur? | ... |
| Are interrupts involved? | ... |

Only after answering these questions should implementation begin.

---

# 19. Phase 1 Testing

Every member must test their implementation independently before integration.

Example:

## GPIO

```text
Test:
Toggle LED every 500 ms.

Expected:
LED changes state every 500 ms.

Observed:
LED changes state every 500 ms.

Result:
PASS
```

## UART

```text
Test:
Send "Hello STM32" to PC.

Expected:
Correct string appears in serial terminal.

Observed:
Correct string received.

Result:
PASS
```

## PWM

```text
Test:
Set three different servo positions.

Expected:
Servo moves to three different positions.

Observed:
...

Result:
PASS
```

## SPI

```text
Test:
Read a known register from the selected ToF sensor.

Expected:
Known device response.

Observed:
...

Result:
PASS
```

## EXTI

```text
Test:
Press pedestrian button.

Expected:
Interrupt occurs.

Observed:
UART reports button event.

Result:
PASS
```

---

# 20. Teaching Requirement

After completing their implementation, each member should give a short explanation to the other three.

The presentation should answer:

1. What is this peripheral?
2. What hardware does it control?
3. Which clock does it use?
4. Which registers are important?
5. How is it configured?
6. How does data/event flow through it?
7. What did you implement?
8. How did you test it?
9. What problems did you encounter?

The other members should be encouraged to ask questions.

The assigned member is considered complete only when the rest of the team has a basic understanding of the peripheral.

---

# 21. What Phase 1 Should Achieve

At the end of Phase 1, the team should be able to understand this system:

```text
                         STM32F411
                             │
              ┌──────────────┼──────────────┐
              │              │              │
             GPIO           TIMER          UART
              │              │              │
             LEDs           PWM             PC
                             │
                           Servo

              EXTI/NVIC                    SPI
                  │                          │
             Push Button                  ToF

                             I²C
                              │
                             LCD
```

More importantly, every member should understand what happens underneath these blocks.

The objective is not:

> "I know how to call a function that turns on an LED."

The objective is:

> "I understand how the STM32 clock enables the GPIO peripheral, how the GPIO registers configure the pin, how the CPU writes the output register, and how that produces a physical signal on the pin."

That level of understanding is the main purpose of Phase 1.

---

# 22. Phase 1 Completion Checklist

Before moving to Phase 2, verify:

- [ ] RCC / clock concepts understood
- [ ] GPIO implementation completed
- [ ] SysTick/basic timing completed
- [ ] UART implementation completed
- [ ] Timer implementation completed
- [ ] PWM implementation completed
- [ ] EXTI implementation completed
- [ ] NVIC concepts understood
- [ ] SPI implementation completed
- [ ] I²C implementation completed if required by LCD
- [ ] Each peripheral has documentation
- [ ] Each peripheral has a working example
- [ ] Each peripheral has test results
- [ ] No STM32 HAL functions are being used
- [ ] Each member has taught their assigned topics
- [ ] All four members understand the basic operation of all Phase 1 peripherals

---

# 23. Transition to Phase 2

Once Phase 1 is complete, the team will move to the actual:

**Intelligent Traffic Control System**

The Phase 1 peripheral layer will then be used to build the application involving:

- ToF-based vehicle detection
- Vehicle counting
- Three traffic lanes
- LED traffic signals
- Servo-operated gate
- Pedestrian crossing
- Push-button interaction
- LCD
- UART debugging
- Adaptive traffic timing
- Traffic management algorithm

The purpose of Phase 1 is therefore to make sure that when Phase 2 begins, everyone understands the hardware they are programming rather than simply using pre-written libraries.
