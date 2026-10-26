# Traffic Light Algorithm

This project implements a three-lane traffic controller using a state-machine-based approach.

The controller dynamically adjusts green-light timing according to the amount of traffic waiting in the lane. It also supports yellow-light transitions, an emergency-vehicle mode, and a halted state when there are no cars to serve.

## Overview

The controller manages three traffic lanes:

- Lane 0
- Lane 1
- Lane 2

Under normal operation, one lane receives a green light while the other lanes remain red.

The amount of traffic in the selected lane influences how long that lane receives a green light. After the green phase, the controller briefly changes the lights to yellow before moving to the next lane.

The normal lane order is sequential:

```text
Lane 0 -> Lane 1 -> Lane 2 -> Lane 0 -> ...
```

The controller can also enter special operating modes when an emergency vehicle is detected or when there are no cars waiting.

## Green-Light Timing

Green-light duration is dynamically calculated from the number of cars waiting in the lane.

The timing is based on:

```text
10 + (number of cars / 2)
```

The result is constrained between:

```text
Minimum green duration: 10
Maximum green duration: 30
```

Examples:

| Cars Waiting | Green Duration |
|-------------:|---------------:|
| 0            | 10             |
| 10           | 15             |
| 20           | 20             |
| 30           | 25             |
| 40           | 30             |
| 60           | 30             |

The maximum duration prevents a heavily congested lane from keeping the green light indefinitely.

## Initial Operation

When the controller starts, it examines the initial traffic levels and selects the lane with the highest number of waiting cars.

That lane receives the first green light.

The other two lanes receive red lights.

The initial green duration is then determined from the selected lane's traffic level.

Once that initial timing has been scheduled, the traffic represented by that initial count is considered served.

## Normal Traffic Operation

During normal operation, the currently selected lane remains green while its countdown runs.

The controller does not continuously change the duration of the active green phase based on newly arriving cars. Instead, it prepares the timing for the next lane while the current green phase is 5 seconds before end.

This allows traffic arriving during the current green phase to influence a future green phase without unexpectedly extending the current one.

### Preparing the Next Green Phase

When the current green countdown reaches the preparation point, the controller calculates the green duration that will be used by the next lane.

The next lane is the following lane in the normal sequence.

Its traffic level is used to determine the next duration, subject to the 10-to-30 range.

When the current green phase finally ends, the prepared duration becomes the starting countdown for the next green phase.

## Yellow-Light Transition

When a green phase finishes, the controller does not immediately switch to another green light.

Instead, it enters a yellow-light transition.

During this transition:

- All traffic lights are yellow.
- No lane has a green light.
- The transition lasts for 4 countdown cycles.

After the yellow period finishes, the controller selects the next lane in sequence.

The selected lane becomes green and the other lanes become red.

The controller then returns to normal operation.

The normal transition is therefore:

```text
Green
  |
  v
Yellow transition
  |
  v
Next lane green
```

## Lane Selection

Under normal operation, lanes are selected sequentially rather than by traffic volume.

The sequence is:

```text
Lane 0
  |
  v
Lane 1
  |
  v
Lane 2
  |
  v
Lane 0
  |
  v
...
```

Traffic volume determines the duration of a lane's green phase, while the lane order itself remains fixed.

## Emergency Mode

The controller supports detection of an emergency vehicle.

When an emergency vehicle is detected during normal operation, the controller leaves the normal traffic sequence and enters the emergency state.

During emergency handling, the regular lane rotation is suspended and the traffic lights are placed into a safe red-light configuration.

The emergency mode uses the greenlight countdown before returning the controller to its normal initialization process.

Once the emergency handling period is complete, the controller reinitializes normal traffic operation. The normal lane-selection and timing process then starts again based on the current traffic situation.

The emergency state is therefore treated as a temporary interruption of normal traffic control rather than as another normal lane in the rotation.

## Halt State

The controller also supports a halted state.

After normal traffic processing, the controller checks whether there are any cars remaining across the lanes.

If no cars are waiting, the controller enters:

```text
STATE_HALT
```

While halted, the controller remains stopped until new traffic is detected.

When new cars arrive, normal traffic operation is initialized again and the controller resumes traffic management.

The high-level behavior is:

```text
Cars present
    |
    v
Normal operation
    |
    v
No cars remaining
    |
    v
STATE_HALT
    |
    | New cars arrive
    v
Normal operation
```

## State Machine

The controller has four operating states:

```text
STATE_DEFAULT
STATE_YELLOWLIGHT_WAIT
STATE_EMERGENCY
STATE_HALT
```

### `STATE_DEFAULT`

This is the normal traffic-control state.

The active lane has the green light and its countdown is processed.

While the green phase is approaching its end, the controller prepares the duration for the next lane.

If the current green phase finishes, the controller begins the yellow-light transition.

If an emergency vehicle is detected, the controller switches to emergency handling.

If there are no cars remaining, the controller enters the halt state.

### `STATE_YELLOWLIGHT_WAIT`

This state provides the transition between two green phases.

All lights remain yellow for the configured transition period.

When the transition finishes, the next lane is selected and normal operation resumes.

### `STATE_EMERGENCY`

This state temporarily interrupts normal traffic operation.

The normal lane sequence is suspended and the traffic lights are placed into the emergency configuration.

After the emergency countdown finishes, the controller returns to normal operation through reinitialization.

### `STATE_HALT`

This state is used when there are no cars waiting.

The controller remains halted until new traffic is detected.

When traffic appears again, the normal traffic-light process is initialized and resumed.


## Traffic Timing Summary

| Setting | Value |
|---------|------:|
| Minimum green duration | 10 |
| Maximum green duration | 30 |
| Yellow transition | 4 cycles |
| Number of lanes | 3 |

## Example

Suppose the initial traffic levels are:

```text
Lane 0: 3 cars
Lane 1: 8 cars
Lane 2: 4 cars
```

Lane 1 has the most traffic, so it receives the first green light.

Its green duration is calculated from its traffic level and constrained by the configured limits.

While Lane 1 is green, the controller prepares the timing for Lane 2.

When Lane 1's green phase ends:

```text
Lane 1: Green
Lane 0: Red
Lane 2: Red
```

becomes:

```text
Lane 0: Yellow
Lane 1: Yellow
Lane 2: Yellow
```

After the yellow transition:

```text
Lane 0: Red
Lane 1: Red
Lane 2: Green
```

The controller continues through the lanes in the same sequential order.

## Design Summary

The controller separates traffic management into four high-level situations:

1. **Normal traffic** — dynamically timed green lights are rotated between lanes.
2. **Yellow transition** — all lights temporarily become yellow before the next lane receives green.
3. **Emergency handling** — normal traffic control is interrupted and the system temporarily enters emergency mode.
4. **No traffic** — the controller enters a halt state until new cars arrive.

The result is a predictable traffic sequence with adaptive green-light timing, controlled lane transitions, emergency handling, and automatic stopping when there is no traffic to process.
