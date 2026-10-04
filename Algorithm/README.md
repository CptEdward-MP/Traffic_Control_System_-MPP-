# Traffic Light Algorithm

The program dynamically calculates the green-light duration based on the traffic density in each lane.

## `traffic_t` Structure

The traffic controller stores its current state in the `traffic_t` structure:

### `traffic_car_body[3]`

Stores the number of cars detected in each of the three lanes.

```text
traffic_car_body[0] -> Lane 0
traffic_car_body[1] -> Lane 1
traffic_car_body[2] -> Lane 2
```

The traffic density is used to dynamically determine how long the selected lane should receive a green light.

### `light_state[3]`

Stores the current traffic-light state for each lane.

Each element represents the colour of the traffic light facing that lane.

```text
light_state[0] -> Lane 0
light_state[1] -> Lane 1
light_state[2] -> Lane 2
```

### `traffic_emergency_vehicle`

Stores three boolean flags indicating whether an emergency vehicle is present in each lane.

The three least-significant bits represent the lanes:

```text
0bXXXXX[lane0][lane1][lane2]
```

For example:

```text
00000001 -> Emergency vehicle in Lane 2
00000010 -> Emergency vehicle in Lane 1
00000100 -> Emergency vehicle in Lane 0
00000111 -> Emergency vehicles in all three lanes
```

Because the value is non-zero whenever at least one emergency vehicle is detected, the presence of an emergency vehicle can be checked with a simple comparison:

```c
if (traffic_emergency_vehicle > 0)
{
    // Emergency vehicle detected
}
```

### `traffic_state`

Stores the current operating state of the traffic controller.

There are three possible states:

```text
DEFAULT
YELLOW
EMERGENCY
```

#### `DEFAULT`

The normal operating state. Green-light duration is dynamically calculated based on traffic density.

#### `YELLOW`

The transition state between traffic-light changes. During this state, all traffic lights are set to yellow.

The yellow-light duration is controlled by `yellowlight_countdown`, which defaults to `4`.

#### `EMERGENCY`

Used when an emergency vehicle is detected. The traffic controller changes its normal behaviour to handle the emergency vehicle.

### `lane_select`

Stores which lane is currently selected to receive the green light.

```text
lane_select -> currently selected lane
```

This value is used when managing the green-light countdown and determining which lane is currently being served.

### `yellowlight_countdown`

Stores the remaining duration of the yellow-light state.

Its default value is 4.

When the traffic controller enters the `YELLOW` state, all three traffic lights are set to yellow and this countdown is used to control the duration of the transition.

### `greenlight_countdown`

Stores the actual remaining green-light countdown for the currently selected lane.

This is the countdown that is actively decremented while the lane has a green light.

### `greenlight_countdown_snapshot`

Stores the dynamically calculated green-light duration.

The snapshot is calculated from the number of cars in each lane and represents the green-light duration that should be applied to the selected lane.

The snapshot is taken when (greenlight_countdown == 5)

This allows the next green-light duration to be calculated dynamically based on the latest traffic density while the current countdown is still running.

The distinction between the two variables is:

```text
greenlight_countdown
    -> Actual active countdown

greenlight_countdown_snapshot
    -> Dynamically calculated target duration
```

The snapshot is therefore used to determine the appropriate green-light duration, while `greenlight_countdown` tracks the remaining time of the currently active green light.

## Algorithm Overview

In the normal `DEFAULT` state, the controller uses the number of cars in each lane to dynamically determine the green-light duration.

The general flow is:

1. Read the traffic count for each lane from `traffic_car_body[3]`.
2. Determine which lane is currently selected using `lane_select`.
3. Run the selected lane's green light using `greenlight_countdown`.
4. When `greenlight_countdown` reaches `5`, calculate a new green-light duration based on the current traffic density.
5. Store the calculated duration in `greenlight_countdown_snapshot`.
6. Continue the current countdown.
7. When the current green-light cycle ends, use the calculated duration for the appropriate next cycle.
8. If an emergency vehicle is detected, indicated by `traffic_emergency_vehicle > 0`, switch to the `EMERGENCY` state.
9. During a traffic-light transition, enter the `YELLOW` state and set all traffic lights to yellow for the duration specified by `yellowlight_countdown`.

This separates the active countdown from the dynamically calculated duration, allowing the traffic controller to update its timing without unnecessarily modifying the countdown currently in progress.
