#include "traffic_manager.h"

#include <stdio.h>
#include <unistd.h>



#define FALLBACK_RED_LIGHT_TIMING 10


// This runs at the start, it picks the lane and initializes traffic light counts
f_inline static
void traffic_setup_redlight(traffic_t* i)
{
    traffic_lane_select max_cars_lane = LANE0;

    max_cars_lane = i->traffic_car_body[0] > i->traffic_car_body[1]           ?
        (i->traffic_car_body[0] > i->traffic_car_body[2] ? LANE0 : LANE2)     :
        (i->traffic_car_body[1] > i->traffic_car_body[2] ? LANE1 : LANE2);


    // let cars with max lanes go first
    i->red_light_countdown[max_cars_lane] = 0;

    i->lane_select = max_cars_lane;


    i->red_light_countdown[(max_cars_lane + 1) % 3] = FALLBACK_RED_LIGHT_TIMING;

    i->red_light_countdown[(max_cars_lane + 2) % 3] = FALLBACK_RED_LIGHT_TIMING * 2;

    i->greenlight_countdown = FALLBACK_RED_LIGHT_TIMING;
}


f_inline static
void traffic_update_redlight(traffic_t* i)
{

    traffic_lane_select current_lane = i->lane_select;

    i->red_light_countdown[current_lane] = FALLBACK_RED_LIGHT_TIMING * 2;

    i->greenlight_countdown = FALLBACK_RED_LIGHT_TIMING;

    i->lane_select = (current_lane + 1) % 3;
}


f_inline static
void traffic_redlight_countdown(traffic_t* i)
{
    for (u8 count = 0; count < 3; count++)
    {
        /**
         * The lane currently active will have 0 as the redlight count.
         * This is because it counts the u8 greenlight_countdown
         * To countdown for the selected lane
         */
        if (count == i->lane_select) continue;


        if (i->red_light_countdown[count] == 0)
        {
            traffic_update_redlight(i);
            return;
        }

        i->red_light_countdown[count]--;
        i->traffic_car_body[i->lane_select] = 0;
    }

    i->greenlight_countdown--;

    return;
}



f_inline static
void traffic_state_update(traffic_t* i)
{
    switch(i->traffic_state)
    {
        case STATE_HALT:
            pal_printf("Aargh! Program is halted!");
            break;

        case STATE_DEFAULT:

            if (i->traffic_emergency_vehicle != 0)
            {
                // TODO: Implement emergency vehicle handling
            }

            traffic_redlight_countdown(i);

            break;


        case STATE_YELLOWLIGHT_WAIT:

            break;
    }

    return;
}


void traffic_init_state(traffic_t* i, u8 lane[])
{
    for (u8 count = 0; count < 3; count++)
    {
        i->traffic_car_body[count] = lane[count];
    }

    i->traffic_state = STATE_DEFAULT;


    traffic_setup_redlight(i);
}


void traffic_print_state(traffic_t* i)
{
    for (u8 count = 0; count < 3; count++)
    {
        pal_printf("CAR COUNT: %d ", i->traffic_car_body[count]);
    }
    pal_printf("\n");

    for (u8 count = 0; count < 3; count++)
    {
        if (count == i->lane_select)
        {
            pal_printf("GREEN COUNT: %d ", i->greenlight_countdown);
            continue;
        }

        pal_printf("RED COUNT: %d ", i->red_light_countdown[count]);
    }

    pal_printf("\n");

    pal_printf("LANE SELECT: %d\n", i->lane_select);

    pal_printf("\n");

    return;
}



int main(void)
{
    traffic_t main_state;

    u8 initialize_car_count[3] = {17, 20, 25};

    traffic_init_state(&main_state, initialize_car_count);

    while(1)
    {
        traffic_print_state(&main_state);

        traffic_state_update(&main_state);

        sleep(1);
    }


    return 0;
}
