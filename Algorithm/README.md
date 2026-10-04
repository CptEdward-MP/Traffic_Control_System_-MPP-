# Traffic Light Algorithm

The traffic controller dynamically determines the green-light duration for each lane based on the number of cars waiting in that lane.

The controller operates as a state machine with three states:

- `STATE_DEFAULT`
- `STATE_YELLOWLIGHT_WAIT`
- `STATE_EMERGENCY`

Under normal operation, the controller gives a lane a dynamically calculated green-light duration, switches all lights to yellow, and then moves the green light to the next lane.

## Configuration

The green-light duration is constrained to a minimum of `10` and a maximum of `30`.

The yellow-light transition lasts for `4` countdown cycles.

## `traffic_t` Structure

The traffic controller stores its current state in the `traffic_t` structure:

### `traffic_car_body[3]`

Stores the number of cars detected in each lane.

```text
traffic_car_body[0] -> Lane 0
traffic_car_body[1] -> Lane 1
traffic_car_body[2] -> Lane 2
```

When a lane receives a green-light duration calculation, its current car count is used to determine the duration.

After the duration is calculated, that lane's car count is reset to `0`.

This means the current lane's traffic is considered served once its green-light duration has been scheduled.

### `light_state[3]`

Stores the traffic-light state for each lane.

Each element represents the light facing the corresponding lane:

```text
light_state[0] -> Lane 0
light_state[1] -> Lane 1
light_state[2] -> Lane 2
```

During normal operation, exactly one lane is green and the other two lanes are red.

During the yellow transition, all three lanes are yellow.

### `traffic_emergency_vehicle`

Stores three boolean flags indicating whether an emergency vehicle is present in each lane.

The three least-significant bits represent the lanes:

```text
0bXXXXX[lane0][lane1][lane2]
```

The value can be checked directly to determine whether any emergency vehicle is present:

```c
if (traffic_emergency_vehicle != 0)
{
    // Emergency vehicle detected
}
```

The current implementation detects the emergency condition and changes the state to `STATE_EMERGENCY`. The emergency-state handling itself is currently not implemented.

### `traffic_state`

Stores the current operating state of the controller.

The available states are:

```text
STATE_DEFAULT
STATE_YELLOWLIGHT_WAIT
STATE_EMERGENCY
```

#### `STATE_DEFAULT`

Normal traffic-light operation.

The active lane's green-light countdown is decremented once per update.

When the countdown reaches `5`, the next green-light duration is calculated.

When the countdown reaches `0`, the controller switches all lights to yellow and enters `STATE_YELLOWLIGHT_WAIT`.

#### `STATE_YELLOWLIGHT_WAIT`

Transition state between lanes.

All three lights remain yellow while `yellowlight_countdown` is decremented.

When the countdown reaches `0`, the next lane is selected and given a green light. The other two lanes are set to red, and the controller returns to `STATE_DEFAULT`.

#### `STATE_EMERGENCY`

Emergency-vehicle handling state.

The controller enters this state when:

```c
traffic_emergency_vehicle != 0
```

The current implementation does not yet contain any behavior for this state.

### `lane_select`

Stores the lane that currently has the green light.

The lane is rotated sequentially:

```c
i->lane_select = (current_lane + 1) % 3;
```

Therefore, the normal sequence is:

```text
Lane 0 -> Lane 1 -> Lane 2 -> Lane 0 -> ...
```

The lane with the green light is also the lane whose traffic count is used when calculating the next green-light duration.

### `yellowlight_countdown`

Stores the remaining duration of the yellow-light transition.

It is initialized to:

```text
4
```

When changing lanes, all three lights are set to yellow and the countdown is reset to `4`.

Each update decrements the countdown:

```c
i->yellowlight_countdown--;
```

When it reaches `0`, the next lane receives the green light.

### `greenlight_countdown`

Stores the actual active green-light countdown.

This is the countdown that is decremented while the currently selected lane has a green light.

It is initialized from the traffic count of the initial lane.

When the countdown reaches `5`, a new green-light duration is calculated and stored in `greenlight_countdown_snapshot`.

When the countdown reaches `0`, the current green-light cycle ends and the controller changes to the yellow-light state.

After the lane change is initiated, the countdown is replaced with the previously calculated snapshot:

```c
i->greenlight_countdown = i->greenlight_countdown_snapshot;
```

### `greenlight_countdown_snapshot`

Stores the next dynamically calculated green-light duration.

It is calculated when:

```text
greenlight_countdown == 5
```

The snapshot is calculated from the current car count of the active lane:

```c
MIN_GREEN_LIGHT_TIMING + (traffic_car_body[lane_select] / 2)
```

The resulting value is capped at `MAX_GREEN_LIGHT_TIMING`.

After the snapshot is taken, the current lane's car count is reset to `0`.

The snapshot is then used as the green-light countdown after the current cycle finishes.

## Green-Light Timing Algorithm

The green-light duration is calculated using:

```c
greenlight_duration = MIN_GREEN_LIGHT_TIMING + (car_count / 2);
```

With the configured limits:

```text
Minimum: 10
Maximum: 30
```

The resulting duration is therefore:

```text
10 + (number_of_cars / 2)
```

with the result capped at `30`.

For example:

| Car Count | Calculated Duration |
|-----------|---------------------|
| 0         | 10                  |
| 10        | 15                  |
| 20        | 20                  |
| 30        | 25                  |
| 40        | 30                  |
| 60        | 30                  |

The maximum prevents a lane with a large number of cars from holding the green light indefinitely.

## When the Snapshot Is Taken

The controller does not continuously modify the active green-light countdown.

Instead, while the current green light is running:

```text
greenlight_countdown
```

is decremented normally.

When it reaches `5`, the controller calculates the next duration:

```c
if (i->greenlight_countdown == 5)
{
    traffic_setup_next_greenlight(i);
}
```

This function:

1. Reads the current car count for `lane_select`.
2. Calculates the new green-light duration.
3. Stores the result in `greenlight_countdown_snapshot`.
4. Caps the result at `MAX_GREEN_LIGHT_TIMING`.
5. Resets the current lane's car count to `0`.

This means the controller can calculate the next timing before the current green-light cycle has completely finished.

## Traffic-Light State Transition

A normal lane transition works as follows:

```text
             greenlight_countdown > 5
                       |
                       v
                Continue green
                       |
                       |
              countdown == 5
                       |
                       v
             Calculate snapshot
                       |
                       |
              countdown == 0
                       |
                       v
              Set all lights yellow
                       |
                       v
             STATE_YELLOWLIGHT_WAIT
                       |
                       v
             Decrement yellow timer
                       |
              countdown == 0
                       |
                       v
              Select next lane
                       |
                       v
             Set selected lane green
             Set other lanes red
                       |
                       v
                STATE_DEFAULT
```

## `traffic_setup_next_greenlight()`

Calculates and stores the next green-light duration.

The selected lane's traffic count is then reset:

## `traffic_yellowlight_state()`

Handles the yellow-light transition.

Each call decrements `yellowlight_countdown`.

When it reaches `0`:

1. The next lane is selected.
2. The selected lane is set to green.
3. The other two lanes are set to red.
4. The controller returns to `STATE_DEFAULT`.

Lane selection is sequential:

## `traffic_change_lane()`

Starts the transition to the next lane.

All three traffic lights are set to yellow:

The controller then enters the yellow-light state, and resets the yellow countdown:

## `traffic_countdown()`

Handles the active green-light countdown.

The function:

1. Decrements `greenlight_countdown`.
2. Calculates the next green-light snapshot when the countdown reaches `5`.
3. Changes lanes when the countdown reaches `0`.
4. Loads the calculated snapshot as the countdown for the next green-light cycle.
5. Clears the snapshot.

## `traffic_state_update()`

Dispatches behavior according to the current traffic state.

### Default State

In `STATE_DEFAULT`, the controller first checks for an emergency vehicle:

It then processes the normal green-light countdown.

### Yellow State

In `STATE_YELLOWLIGHT_WAIT`, the controller processes the yellow-light countdown.

### Emergency State

In `STATE_EMERGENCY`, no action is currently performed.

The emergency handling logic is reserved for future implementation.

## Initialization

`traffic_init_state()` initializes the traffic controller.

The initial traffic counts are copied into `traffic_car_body`.
Lane 0 is set to green, while lanes 1 and 2 are set to red.
