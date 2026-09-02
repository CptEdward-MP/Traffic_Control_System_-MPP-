#include "traffic_manager.h"

#include <stdio.h>
#include <unistd.h>

#define FALLBACK_RED_LIGHT_TIMING 20




f_inline static
void traffic_change_lane(traffic_t* i, traffic_lane_select which_lane_is_0)
{
    u8 lanes_with_traffic[2] = "";
    u8 array_counter = 0;


    // which lanes has non-zero car bodies
    for (u8 count = 0; count < 3; count++)
    {
        if (count != (u8)which_lane_is_0)
        {
            lanes_with_traffic[array_counter] = count;
            array_counter++;
        }
    }

    // change lane
    i->lane_select = (i->lane_select + 1) % 3;

    // the 0 car lane has its red light timing adjusted
    i->red_light_countdown[which_lane_is_0] =
        i->red_light_countdown[lanes_with_traffic[0]] + i->red_light_countdown[lanes_with_traffic[1]];


    return;
}



f_inline static
void traffic_DEFAULT_countdown(traffic_t* i)
{

    for (u8 count = 0; count < 3; count++)
    {

        if (i->red_light_countdown[count] == 0)
        {
            traffic_change_lane(i, (traffic_lane_select)count);
            return;
        }


        i->traffic_car_body[i->lane_select] = 0;



        i->red_light_countdown[count]--;

    }


    return;
}

f_inline static
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

            traffic_DEFAULT_countdown(i);

            break;

        case STATE_YELLOWLIGHT_WAIT:

            if (i->yellowlight_countdown == 0)
            {
                i->traffic_state = STATE_DEFAULT;

                // Set all lane to red first
                for (u8 count = 0; count < 3; count++) i->light_state[count] = LIGHT_RED;
                // Change the updated lane to green
                i->light_state[i->lane_select] = LIGHT_GREEN;

            }

            /**
             * TODO: Implement proper yellow lights
             * Garbage setup. Needs to select lane
             */
            i->yellowlight_countdown--;

            break;
    }

    return;
}

static
void traffic_init_state(traffic_t* i, u8 lane[])
{
    for (u8 count = 0; count < 3; ++count)
    {
        i->traffic_car_body[count] = lane[count];
        i->red_light_countdown[count] = FALLBACK_RED_LIGHT_TIMING;
        i->light_state[count] = LIGHT_RED;
    }

    i->traffic_state = STATE_DEFAULT;
    i->lane_select = LANE0;
    i->yellowlight_countdown = 0;

    i->light_state[i->lane_select] = LIGHT_GREEN;

    i->red_light_countdown[i->lane_select+2] =
        i->red_light_countdown[i->lane_select] + i->red_light_countdown[i->lane_select+1];
}


void traffic_print_state(traffic_t* i)
{
    for (u8 count = 0; count < 3; ++count)
        printf("CAR COUNT: %d   ", i->traffic_car_body[count]);

    printf("\n");

    for (u8 count = 0; count < 3; ++count)
    {
        if (count == i->lane_select)
        {
            printf("GREEN LIGHT: %d ", i->red_light_countdown[count]);
            continue;
        }

        printf("RED LIGHT: %d ", i->red_light_countdown[count]);
    }

    printf("\n");
    printf("LANE: %d\n", i->lane_select);
    printf("YELLOWLIGHT_TIME: %d\n", i->yellowlight_countdown);
    printf("PROGRAM STATE (DEFAULT: 1): %d\n", i->traffic_state);
}


int main(void)
{
    traffic_t main_state;

    u8 initialize_car_count[3] = {20, 20, 20};

    traffic_init_state(&main_state, initialize_car_count);

    while(1)
    {
        traffic_print_state(&main_state);

        traffic_state_update(&main_state);

        sleep(1);
    }


    return 0;
}
