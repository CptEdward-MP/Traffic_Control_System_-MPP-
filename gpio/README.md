# **PAL_GPIO — Simple Documentation**

## **1. What is GPIO?**

GPIO means **General Purpose Input/Output**.

It allows the STM32 to use its pins as:

- **Output** → send HIGH or LOW
- **Input** → read HIGH or LOW

---

## **2. Basic GPIO Process**

The basic process is:

**Enable Clock → Configure Pin → Control Pin → Read Pin if needed**

---

## **3. Enable GPIO Clock**

Before using GPIOC, we need to **turn its clock ON**.

This is done using the **RCC register**.

For GPIOC, **bit 2** is used to enable the clock.

```c
RCC->AHB1ENR |= (1u << 2);
```

> **RCC → turns the GPIO clock ON.**

---

## **4. Configure PC13**

We use the **MODER register** to decide what the pin does.

For **PC13**, bits **27 and 26** control the mode.

```text
00 → Input
01 → Output
10 → Alternate Function
11 → Analog
```

For our project, we select:

```text
01 → Output
```

> **MODER → decides the mode of the pin.**

---

## **5. Control the Pin**

We use the **BSRR (Bit Set/Reset Register)** to control the GPIO pin.

```text
Bits 0–15   → SET
Bits 16–31  → RESET
```

For PC13:

```text
Bit 13 → Set PC13 HIGH
Bit 29 → Reset PC13 LOW
```

So:

```c
GPIOC->BSRR = (1u << 13);
```

→ **PC13 HIGH**

```c
GPIOC->BSRR = (1u << 29);
```

→ **PC13 LOW**

> **BSRR → sets or resets the GPIO pin.**

---

## **6. PAL_GPIO Functions**

We created simple functions so the application does not need to directly work with registers.

| Function | Meaning |
|---|---|
| `PAL_GPIO_Init()` | Configure the pin |
| `PAL_GPIO_Set()` | Make pin HIGH |
| `PAL_GPIO_Reset()` | Make pin LOW |
| `PAL_GPIO_Write()` | Write HIGH/LOW |
| `PAL_GPIO_Toggle()` | Change HIGH ↔ LOW |
| `PAL_GPIO_Read()` | Read the pin |

---

## **7. Simple Flow**

```text
        RCC
         ↓
  Enable GPIO Clock
         ↓
       MODER
         ↓
  Configure PC13
         ↓
       BSRR
         ↓
   HIGH / LOW
         ↓
        LED
```

### **In one sentence:**

> **We implemented GPIO without using HAL by enabling the GPIO clock through RCC, configuring PC13 using MODER, and controlling it using BSRR through simple PAL_GPIO functions.**
