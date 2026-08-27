#include "traffic_manager.h"

#include <stdio.h>
#include <unistd.h>



f_inline
u8 traffic_get_greenlight_value(traffic_t* i, traffic_lane_select new_lane)
{
    u8 green_light_temp =
        i->traffic_car_body[new_lane] - i->traffic_car_body[i->lane_select];

    green_light_temp /= 4;

    if (green_light_temp < 10) green_light_temp = 10;

    return green_light_temp;
}

f_inline
bool traffic_DEFAULT_greenlight_countdown(traffic_t* i)
{

    if (i->greenlight_countdown == 0) return true;

    i->greenlight_countdown--;
    i->traffic_car_body[i->lane_select] = 0;

    return false;
}


f_inline
void traffic_DEFAULT_check_if_lane_changed(traffic_t* i)
{
    traffic_lane_select max_cars_lane = i->lane_select;

    if (i->greenlight_countdown == 0)
    {
        max_cars_lane = i->traffic_car_body[0] > i->traffic_car_body[1]   ?
            (i->traffic_car_body[0] > i->traffic_car_body[2] ? LANE0 : LANE2)                 :
            (i->traffic_car_body[1] > i->traffic_car_body[2] ? LANE1 : LANE2);
    }

    if (max_cars_lane != i->lane_select)
    {
        i->yellowlight_countdown = 5;

        i->greenlight_countdown = traffic_get_greenlight_value(i, max_cars_lane);

        i->lane_select = max_cars_lane;

        i->light_state[i->lane_select] = LIGHT_YELLOW;
    }
}

f_inline
void traffic_state_update(traffic_t* i)
{
    switch(i->traffic_state)
    {
        case STATE_HALT:
            printf("Aargh! Program is halted!");
            break;

        case STATE_DEFAULT:
            if (i->traffic_emergency_vehicle != 0)
            {
                // TODO: Implement emergency vehicle handling
            }

            /**
             * COUNTDOWN FOR GREEN LIGHT
             */
            bool lane_changed = traffic_DEFAULT_greenlight_countdown(i);

            /**
             * Check if lane has changed
             */
            if (lane_changed == true)
                traffic_DEFAULT_check_if_lane_changed(i);

            break;

        case STATE_YELLOWLIGHT_WAIT:

            if (i->yellowlight_countdown == 0)
            {
                i->traffic_state = STATE_DEFAULT;

                // Set all lane to red first
                for (u8 count = 0; count < 3; ++count) i->light_state[count] = LIGHT_RED;
                // Change the updated lane to green
                i->light_state[i->lane_select] = LIGHT_GREEN;

            }

            i->yellowlight_countdown--;
            break;
    }

    return;
}

void traffic_init_state(traffic_t* i, u8 lane[])
{
    for (u8 count = 0; count < 3; ++count)
        i->traffic_car_body[count] = lane[count];

    i->traffic_state = STATE_DEFAULT;
    i->lane_select = LANE0;
    i->yellowlight_countdown = 0;
    i->greenlight_countdown = 0;

    traffic_state_update(i);
}


f_inline
void traffic_print_state(traffic_t* i)
{
    for (u8 count = 0; count < 3; ++count)
    {
        printf("CAR COUNT: %d ", i->traffic_car_body[count]);
    }
    printf("\n");
    printf("LANE: %d\n", i->lane_select);
    printf("GREENLIGHT_TIME: %d\n", i->greenlight_countdown);
    printf("YELLOWLIGHT_TIME: %d\n", i->yellowlight_countdown);
    printf("PROGRAM STATE (DEFAULT: 1): %d\n", i->traffic_state);
}

int main(void)
{
    traffic_t main_state;

    u8 initialize_car_count[3] = {0, 5, 5};

    traffic_init_state(&main_state, initialize_car_count);

    while(1)
    {
        traffic_print_state(&main_state);

        traffic_state_update(&main_state);

        sleep(1);
    }


    return 0;
}
