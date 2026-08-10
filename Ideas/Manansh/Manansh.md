# Intelligent Traffic Control System
### Proposed by: Manansh Pandey

# Project Overview

The goal of this project is to design and develop an intelligent traffic control system using an ARM Cortex-M4 based STM32F411 microcontroller.

Unlike traditional academic projects that rely on push buttons to simulate vehicles, this project aims to detect vehicles automatically using distance or proximity sensors. The collected sensor data will then be processed by a traffic management algorithm that controls traffic lights and a traffic gate.

The objective is to simulate a small but realistic traffic intersection while allowing every team member to contribute to different aspects of embedded system development.

---

# Proposed Hardware

## Microcontroller

- STM32F411 (ARM Cortex-M4)

This microcontroller provides sufficient processing capability, timers, communication peripherals and GPIOs while allowing us to gain experience with ARM-based embedded systems.

---

## Vehicle Detection Sensors

The exact sensor has not yet been finalized.

Possible options include:

- Time-of-Flight (ToF) Sensor
- IR Obstacle Sensor
- Ultrasonic Sensor

The final choice will be made after comparing all three technologies based on accuracy, cost, response time and ease of implementation.

---

## Traffic Lights

Traffic signals will be simulated using LEDs.

Each lane will have:

- Red LED
- Yellow LED
- Green LED

---

## Gate Control

One lane will contain a gate operated using a servo motor.

The gate will be controlled by the traffic management algorithm whenever required.

---

# Proposed Road Layout

The system will simulate a three-lane road intersection.

- Lane 1
- Lane 2
- Lane 3 (includes a servo-operated gate)

Each lane will have its own vehicle detection sensor and traffic lights.

Vehicles will be represented using cardboard cut-outs or any object capable of interrupting the selected sensor.

```
          Lane 1
             │
        [ Vehicle Sensor ]
             │
        Red Yellow Green

────────────────────────────

          Lane 2
             │
        [ Vehicle Sensor ]
             │
        Red Yellow Green

────────────────────────────

          Lane 3
             │
        [ Vehicle Sensor ]
             │
        Red Yellow Green
             │
         Servo Gate
```

---

# Vehicle Detection

Whenever a vehicle passes through the sensing region, the interruption will be counted as one vehicle.

Example

Vehicle 1 passes

Vehicle Count = 1

Vehicle 2 passes

Vehicle Count = 2

Each lane maintains an independent vehicle count which is continuously updated.

---

# System Flow

The expected flow of the system is

Vehicle

↓

Sensor detects vehicle

↓

Vehicle count updated

↓

Traffic management algorithm

↓

Traffic light decision

↓

LEDs and Servo updated

---

The hardware only provides information about the vehicles. The decision of when to change signals is handled entirely by the traffic management algorithm.

# Traffic Management Features

This document describes the planned functionality of the traffic control system.

---

# Dynamic Traffic Management

Instead of assigning every lane a fixed green signal duration, the system will determine signal timing based on the detected traffic.

For example,

- Lane 1 : 2 vehicles
- Lane 2 : 7 vehicles
- Lane 3 : 1 vehicle

The algorithm may decide to allocate a longer green signal to Lane 2 because it has the highest traffic density.

This allows traffic to be managed dynamically instead of following fixed timing intervals.

---

# Gate Control

The third lane includes a servo-operated gate.

The algorithm decides when the gate should open or close depending on the current traffic situation.

This demonstrates interaction between sensing, decision making and physical actuation.

---

# Pedestrian Crossing

The project also proposes a pedestrian crossing feature.

A push button will be placed near the road intersection.

When pressed,

- The current traffic cycle is paused.
- All vehicle traffic lights turn Red.
- Pedestrians are allowed to cross safely.

After the pedestrian crossing is complete, the normal traffic algorithm resumes operation.

As an extension, an additional sensor may be added on the opposite side of the crossing to detect when the pedestrian has completely crossed the road, allowing the system to resume automatically instead of relying on another button press.

---

# UART Debugging

All important system events will be transmitted over UART.

Examples include

- Vehicle detections
- Vehicle counts
- Current traffic light state
- Servo gate status
- Pedestrian requests
- Algorithm decisions
- Error messages

UART output will simplify debugging during development.

---

# Adaptive Signal Timing

Traffic signal duration will not be fixed.

The algorithm should continuously monitor traffic conditions and adjust signal timings according to vehicle density.

This forms the core intelligence of the project.

---

# Educational Objectives

Through this project we aim to gain experience with

- ARM Cortex-M microcontrollers
- Embedded C programming
- Sensor interfacing
- Servo motor control
- UART communication
- Embedded system integration
- Team-based software development using Git and GitHub

---

# Possible Future Improvements

If additional time is available, the following features may be explored.

- Emergency vehicle priority
- Multiple connected intersections
- Wireless monitoring
- Data logging
- Graphical PC dashboard
- AI-based traffic prediction
- Camera-based vehicle detection

# Proposed Work Distribution

> **Note:** This is only a proposed division of work based on my current understanding of the project and everyone's technical experience. The responsibilities can be modified later depending on personal interest, technical capability, and any new project requirements.

---

# Manansh

## System Integration & Hardware

### Responsibilities

- Procure the required hardware components.
- Assemble the complete hardware setup.
- Wire all peripherals to the STM32F411.
- Perform soldering and hardware assembly where required.
- Integrate all software modules developed by the team.
- Ensure all hardware and software components work together as a complete system.
- Assist other members during final integration and testing.

---

# Vasudev

## Traffic Management Algorithm

### Responsibilities

- Design the traffic management algorithm.
- Implement the traffic control logic.
- Implement adaptive signal timing.
- Implement gate control logic.
- Test the algorithm using software simulations and unit tests before hardware integration.
- Improve and optimize the algorithm based on testing results.

---

# Vaishali

## Sensor & Motor Control

### Responsibilities

- Interface the selected vehicle detection sensor (ToF / IR / Ultrasonic).
- Develop the software required to detect and count vehicles.
- Interface and control the servo motor.
- Test sensor accuracy and reliability.
- Verify proper operation before integration.

---

# Bharath

## User Interface & Debugging

### Responsibilities

- Develop UART debugging functionality.
- Design useful debug messages for development.
- Interface the LCD (if used).
- Implement pedestrian push-button handling.
- Test user interaction features.
- Assist during system testing and debugging.

---

# General Responsibilities

Every team member is expected to:

- Use Git and GitHub for version control.
- Create feature branches for their work.
- Submit Pull Requests instead of pushing directly to the main branch.
- Review documentation before starting implementation.
- Document any important design decisions.
- Help other team members whenever possible.
- Participate in testing and final system integration.

---

# Final Note

This work distribution is intended only as an initial proposal. It can be adjusted at any point during the project after discussion with the team. The primary objective is to ensure that everyone contributes meaningfully while also getting the opportunity to learn different aspects of embedded systems development.