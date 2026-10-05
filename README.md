# Traffic Control System

A modular embedded traffic-control system built around an **STM32F411**, designed to demonstrate traffic-state management, digital vehicle detection, physical traffic lights, cooperative task scheduling, USB telemetry, and PC-based real-time visualization.

The project is structured so that the **traffic-control logic remains independent of the hardware**, while hardware access is handled through abstraction layers.

---

## Overview

The system monitors three traffic lanes using digital sensors and dynamically controls their traffic lights.

Each lane has:

- A vehicle-count sensor
- A red LED
- A green LED

The STM32 acts as the **main controller and source of truth**.

The PC/Pygame application is only a **monitoring and visualization interface**. It does not control the traffic system.

```text
                    ┌──────────────────────┐
                    │      STM32F411       │
                    │                      │
                    │   Traffic Manager    │
                    │         │            │
                    │    ┌────┴────┐       │
                    │    │ Sensors │       │
                    │    └────┬────┘       │
                    │         │            │
                    │    Vehicle Task      │
                    │         │            │
                    │    Traffic FSM       │
                    │         │            │
                    │    Physical LEDs     │
                    │         │            │
                    │      USB CDC         │
                    └─────────┬────────────┘
                              │
                              │ Telemetry
                              ▼
                    ┌──────────────────────┐
                    │    Python / Pygame   │
                    │                      │
                    │  Traffic Visualization│
                    │  Connection FSM      │
                    └──────────────────────┘
```

---

# Features

- Three-lane traffic control
- Digital vehicle detection
- Rising-edge sensor detection
- Traffic-dependent green-light duration
- Minimum and maximum green-light limits
- Round-robin lane selection
- Yellow-light transition state
- Emergency state
- Physical LED traffic-light output
- CMSIS-based GPIO abstraction
- Hardware abstraction through PAL modules
- Cooperative scheduler
- Periodic application tasks
- USB CDC telemetry
- PC visualization using Python and Pygame
- Automatic COM13 / COM15 reconnection
- PC-side connection state machine
- Hardware-independent traffic-management logic

---

# Project Architecture

The project separates the system into several layers.

```text
Application
    │
    ├── Traffic Manager
    ├── Vehicle Task
    ├── Traffic Manager Task
    └── PC Display Task
            │
            ▼
Services / Abstraction
    │
    ├── Scheduler
    ├── Sensor
    ├── Traffic Hardware
    └── USB
            │
            ▼
PAL / Hardware Abstraction
    │
    ├── PAL GPIO
    └── PAL Time
            │
            ▼
STM32 Hardware
```

The main idea is to prevent application logic from directly depending on STM32 HAL GPIO operations.

---

# Main Components

## 1. Traffic Manager

### `traffic_manager.c`
### `traffic_manager.h`

This is the **core of the traffic-control system**.

It contains the traffic state machine and traffic decision logic.

The traffic manager operates on a `traffic_t` structure containing information such as:

- Current traffic state
- Vehicle count for each lane
- Currently selected lane
- Traffic-light states
- Green-light countdown
- Yellow-light countdown
- Emergency-vehicle state

The traffic manager does not directly control GPIO pins.

Instead, it modifies the logical traffic state.

```text
Traffic Manager
       │
       ├── lane selection
       ├── countdown
       ├── state transitions
       └── light_state[]
```

This keeps the traffic algorithm independent of the physical board.

---

# Traffic Algorithm

At startup, the system selects the lane with the highest number of vehicles.

After startup, lanes are processed in round-robin order:

```text
L0 → L1 → L2 → L0 → ...
```

The number of vehicles affects the **duration** of the green light rather than continuously changing the lane order.

Green time is calculated approximately as:

```text
Green Time = 10 + (number of cars / 2)
```

with:

```text
Minimum = 10 seconds
Maximum = 30 seconds
```

Therefore:

```text
0 cars  → 10 s
10 cars → 15 s
20 cars → 20 s
40 cars → 30 s
60 cars → 30 s
```

The maximum limit prevents extremely high traffic counts from producing excessively long green phases.

---

# Traffic States

The traffic manager operates as a state machine.

Conceptually:

```text
             ┌──────────────┐
             │   DEFAULT    │
             └──────┬───────┘
                    │
             countdown = 0
                    │
                    ▼
          ┌───────────────────┐
          │ YELLOWLIGHT_WAIT  │
          └─────────┬─────────┘
                    │
              yellow complete
                    │
                    ▼
             ┌─────────────┐
             │   DEFAULT   │
             └─────────────┘


Emergency:

DEFAULT ───────► EMERGENCY
                   │
             emergency complete
                   │
                   ▼
                DEFAULT
```

The system also contains a `STATE_HALT` definition in the current implementation, although the normal traffic flow no longer transitions into HALT when all lanes are empty. Empty lanes continue receiving the minimum green time.

---

# 2. Traffic Hardware

### `traffic_hw.c`
### `traffic_hw.h`

This module connects the **logical traffic-light state** to the physical LEDs.

The traffic manager only knows:

```text
LIGHT_RED
LIGHT_GREEN
LIGHT_YELLOW
```

The hardware module translates these states into GPIO outputs.

Current physical mapping:

| Lane | Red | Green |
|---|---|---|
| Lane 0 | PB5 | PB15 |
| Lane 1 | PB4 | PB13 |
| Lane 2 | PA15 | PB12 |

There are currently no physical yellow LEDs.

Therefore the hardware representation of yellow is:

```text
Red   = OFF
Green = OFF
```

This allows the logical state machine to contain a yellow state without requiring a physical yellow LED.

---

# 3. PAL GPIO

### `pal_gpio.c`
### `pal_gpio.h`

The GPIO PAL provides a hardware abstraction layer around STM32 GPIO registers.

The application can use functions such as:

```c
PAL_GPIO_Init()
PAL_GPIO_Set()
PAL_GPIO_Reset()
PAL_GPIO_Write()
PAL_GPIO_Toggle()
PAL_GPIO_Read()
```

Instead of directly manipulating GPIO registers throughout the application.

The PAL supports:

- Input
- Output
- Analog
- Pull-up
- Pull-down
- Speed configuration

The GPIO PAL directly accesses STM32 registers rather than relying on HAL GPIO functions.

This makes the GPIO interface reusable by higher-level modules.

---

# 4. Sensor Module

### `sensor.c`
### `sensor.h`

The sensor module represents the three vehicle-detection inputs.

Current mapping:

| Lane | Sensor |
|---|---|
| Lane 0 | PA0 |
| Lane 1 | PA1 |
| Lane 2 | PA2 |

Sensors are configured as digital inputs.

The sensor module uses the GPIO PAL rather than directly accessing STM32 GPIO registers.

```text
Vehicle Sensor
      │
      ▼
  sensor.c
      │
      ▼
 pal_gpio.c
      │
      ▼
 STM32 GPIO
```

---

# 5. Vehicle Task

### `vehicle_task.c`
### `vehicle_task.h`

`VehicleTask` periodically reads the three sensors.

The task uses **rising-edge detection**.

A vehicle is counted when:

```text
LOW → HIGH
```

Holding the sensor HIGH does not continuously increase the vehicle count.

Example:

```text
LOW
 │
 ▼
HIGH       → +1 vehicle
 │
 │ hold
 │
 ▼
HIGH       → no additional vehicle
 │
 ▼
LOW        → sensor armed again
 │
 ▼
HIGH       → +1 vehicle
```

This prevents a continuously active sensor from counting the same vehicle repeatedly.

---

# 6. Traffic Manager Task

### `traffic_manager_task.c`
### `traffic_manager_task.h`

This is a thin application wrapper around the traffic manager.

Conceptually:

```text
TrafficManagerTask()
       │
       ├── traffic_state_update()
       │
       └── traffic_update_physical_leds()
```

The task therefore performs two operations:

1. Advance the traffic state machine.
2. Apply the resulting logical light state to the physical LEDs.

The actual traffic algorithm remains inside `traffic_manager.c`.

---

# 7. Cooperative Scheduler

### `scheduler.c`
### `scheduler.h`

The project uses a small **cooperative scheduler** rather than an RTOS.

Tasks are registered with:

```text
callback
context
period
```

For example:

```c
Scheduler_RegisterTask(
    VehicleTask,
    &main_state,
    1
);
```

The scheduler periodically checks whether each task's period has elapsed.

Conceptually:

```text
                  Scheduler
                     │
       ┌─────────────┼─────────────┐
       ▼             ▼             ▼
 VehicleTask   TrafficTask    PCDisplayTask
    1 ms          1000 ms          100 ms
```

The scheduler itself does not know what a task does.

It simply decides **when the callback should execute**.

This keeps the scheduler generic.

---

# Task Execution Model

The current task configuration is approximately:

| Task | Period | Responsibility |
|---|---:|---|
| `VehicleTask` | 1 ms | Read sensors / count vehicles |
| `TrafficManagerTask` | 1000 ms | Advance traffic state machine |
| `PC_DisplayTask` | 100 ms | Send telemetry to PC |

The scheduler is cooperative, meaning tasks execute sequentially.

There is no preemption.

Therefore:

```text
Task A
   ↓
Task B
   ↓
Task C
```

If Task A takes too long, Tasks B and C must wait.

This is an intentional characteristic of the current simple scheduler.

---

# 8. PAL Time

### `pal_time.c`
### `pal_time.h`

The PAL time module provides a simple application-level time interface:

```c
PAL_Time_GetMs()
PAL_Time_Tick()
PAL_Time_Init()
PAL_Time_DelayMs()
```

The project uses the STM32 HAL's existing SysTick.

`SysTick_Handler()` updates both:

```text
HAL tick
   +
PAL time
```

The PAL therefore does not create a second hardware SysTick.

This allows the scheduler to use:

```c
Time_GetMs()
```

without depending directly on the underlying time implementation.

---

# 9. USB Interface

### `usb.c`
### `usb.h`

The USB module provides a simple interface for sending information over USB CDC.

Higher-level modules do not need to directly deal with the USB CDC implementation.

For example:

```c
USB_SendString(...)
```

can be used by application-level telemetry.

The USB connection creates a virtual COM port on the PC.

---

# 10. PC Display Task

### `pc_display.c`
### `pc_display.h`

The PC display module converts the current traffic state into a compact telemetry packet.

Example:

```text
L0:12,L1:7,L2:21,G:1,T:18
```

Meaning:

```text
L0 = 12 vehicles
L1 = 7 vehicles
L2 = 21 vehicles
G  = Lane 1 currently selected
T  = 18 seconds remaining
```

The task periodically sends this information through USB CDC.

```text
traffic_t
    │
    ▼
PC_DisplayTask
    │
    ▼
PC_Display_Send()
    │
    ▼
USB_SendString()
    │
    ▼
USB CDC
    │
    ▼
Python
```

---

# 11. Python / Pygame Monitor

### `PC.py`

The PC application provides a real-time visualization of the embedded system.

It displays:

- Lane 0 traffic
- Lane 1 traffic
- Lane 2 traffic
- Current green lane
- Next green lane
- Green countdown
- STM32 connection status

The PC application does **not** control the STM32.

It is strictly a monitoring system.

```text
STM32
  │
  │ telemetry
  ▼
PC.py
  │
  ▼
Pygame
```

---

# PC Connection State Machine

The Python application also contains its own connection state machine.

It repeatedly attempts to connect to:

```text
COM13
COM15
```

If neither is available, it waits and retries indefinitely.

```text
          ┌─────────────────┐
          │  DISCONNECTED   │
          └────────┬────────┘
                   │
                   ▼
             TRY COM13
                   │
              failed
                   ▼
             TRY COM15
                   │
              failed
                   ▼
                WAIT
                   │
                   └───────────────┐
                                   │
                                   ▼
                              TRY COM13


Successful connection:

TRY COM13 / COM15
        │
        ▼
   CONNECTED
        │
        │ USB disconnected
        ▼
   DISCONNECTED
        │
        └──────► retry
```

This allows the PC monitor to recover automatically if the STM32 is unplugged and reconnected.

---

# PC Independence

The PC application is deliberately not part of the control loop.

If:

- Pygame crashes
- USB is disconnected
- The COM port disappears
- The PC is turned off

the STM32 can continue operating its traffic-control system.

The physical traffic lights and sensor processing remain on the embedded controller.

This establishes a clear separation:

```text
CONTROL SYSTEM
     │
     ▼
   STM32
     │
     ├── Sensors
     ├── Traffic FSM
     ├── Scheduler
     └── Physical LEDs

MONITORING SYSTEM
     │
     ▼
 Python / Pygame
```

---

# Main Data Flow

A vehicle-detection event follows this path:

```text
Physical Sensor
      │
      ▼
 PAL GPIO
      │
      ▼
 Sensor Module
      │
      ▼
 VehicleTask
      │
      ▼
 traffic_car_body[]
      │
      ▼
 Traffic Manager
      │
      ▼
 Traffic State
      │
      ├──────────────► Physical LEDs
      │
      ▼
 PC Display Task
      │
      ▼
 USB CDC
      │
      ▼
 Python / Pygame
```

The same `traffic_t` state is therefore reflected in both the physical system and the PC visualization.

---

# Example System Cycle

Suppose the system starts with:

```text
Lane 0 = 5 cars
Lane 1 = 20 cars
Lane 2 = 8 cars
```

At initialization, Lane 1 has the highest traffic and becomes green.

Its calculated green time is:

```text
10 + 20/2
= 20 seconds
```

After the phase completes, the controller proceeds in round-robin order:

```text
Lane 1
  ↓
Lane 2
  ↓
Lane 0
  ↓
Lane 1
  ↓
...
```

Traffic counts affect the duration assigned to each lane, while the normal lane sequence remains round-robin after startup.

---

# Important Edge Cases

The system is designed to handle several important boundary conditions.

### Empty road

```text
0 / 0 / 0
```

The system continues cycling and gives each lane the minimum green time.

### Maximum traffic

Very large traffic counts cannot increase green time beyond:

```text
30 seconds
```

### Sensor held HIGH

A continuously HIGH sensor produces only one vehicle count until it returns LOW and rises again.

### USB disconnected

The STM32 continues running independently.

The PC monitor returns to its connection state machine and retries COM13 and COM15.

### New vehicles during another lane's GREEN

New vehicle counts are accumulated and influence that lane when its turn arrives.

The system does not immediately interrupt the current green phase simply because another lane becomes busier.

---

# Hardware Interface

## Vehicle Sensors

```text
PA0 → Lane 0 sensor
PA1 → Lane 1 sensor
PA2 → Lane 2 sensor
```

## Traffic LEDs

```text
Lane 0:
    Red   → PB5
    Green → PB15

Lane 1:
    Red   → PB4
    Green → PB13

Lane 2:
    Red   → PA15
    Green → PB12
```

## PC Communication

```text
STM32 USB CDC
      │
      ▼
Windows COM Port
      │
      ▼
Python / PySerial
      │
      ▼
Pygame
```

---

# Design Principles

The project follows several design principles:

### Separation of concerns

Traffic logic does not directly manipulate GPIO.

### Hardware abstraction

GPIO and timing are accessed through PAL interfaces.

### Generic scheduling

The scheduler knows when a task should execute, not what the task does.

### Single source of truth

The STM32 `traffic_t` structure represents the actual traffic state.

### PC as observer

The PC visualizes the embedded system but does not control it.

### Small modules

Each module has a focused responsibility, making the system easier to test, understand, and extend.

---

# Future Extensions

The current architecture can be extended with:

- More lanes
- Additional vehicle sensors
- Traffic-density algorithms
- Adaptive lane prioritization
- Emergency vehicle detection
- Pedestrian crossings
- Logging and statistics
- More advanced scheduling
- RTOS migration
- Improved PC visualization
- Automated stress testing
- Hardware-in-the-loop testing

The modular architecture is intended to allow these features to be added without rewriting the entire system.

---

# Project Goal

This project is not only a traffic-light controller.

It is an embedded-systems exercise demonstrating how a larger application can be decomposed into:

```text
Hardware
   ↓
PAL / Abstraction
   ↓
Drivers / Modules
   ↓
Application Tasks
   ↓
State Machine
   ↓
Scheduler
   ↓
Communication
   ↓
PC Visualization
```

The primary goal is to keep the **control logic understandable, modular, testable, and independent from the hardware implementation wherever practical.**
```

