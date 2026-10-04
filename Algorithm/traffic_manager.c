#include "traffic_manager.h"

#include <stdio.h>
#include <unistd.h>

#define MIN_GREEN_LIGHT_TIMING 10
#define MAX_GREEN_LIGHT_TIMING 30


// This runs at the start, it picks the lane and initializes traffic light counts
/* f_inline static */
/* void traffic_setup_redlight(traffic_t* i) */
/* { */
/*     traffic_lane_select max_cars_lane = LANE0; */

/*     max_cars_lane = i->traffic_car_body[0] > i->traffic_car_body[1]           ? */
/*         (i->traffic_car_body[0] > i->traffic_car_body[2] ? LANE0 : LANE2)     : */
/*         (i->traffic_car_body[1] > i->traffic_car_body[2] ? LANE1 : LANE2); */

/*     i->lane_select = max_cars_lane; */
/* } */


f_inline static
void traffic_setup_next_greenlight(traffic_t* i)
{
    i->greenlight_countdown_snapshot = MIN_GREEN_LIGHT_TIMING + (i->traffic_car_body[i->lane_select] / 2);
    i->traffic_car_body[i->lane_select] = 0;

    if (i->greenlight_countdown_snapshot > MAX_GREEN_LIGHT_TIMING)
        i->greenlight_countdown_snapshot = MAX_GREEN_LIGHT_TIMING;

    return;
}

f_inline static
void traffic_yellowlight_state(traffic_t* i)
{
    i->yellowlight_countdown--;

    if (i->yellowlight_countdown == 0)
    {
      traffic_lane_select current_lane = i->lane_select;

      i->lane_select = (current_lane + 1) % 3;

      i->light_state[i->lane_select] = LIGHT_GREEN;
      i->light_state[(i->lane_select + 1) % 3] = LIGHT_RED;
      i->light_state[(i->lane_select + 2) % 3] = LIGHT_RED;

      i->traffic_state = STATE_DEFAULT;
    }
    return;
}

f_inline static
void traffic_change_lane(traffic_t* i)
{
    for (u8 count = 0; count < 3; count++)
    {
      i->light_state[count] = LIGHT_YELLOW;
    }

    i->traffic_state = STATE_YELLOWLIGHT_WAIT;
    i->yellowlight_countdown = 4;

    return;
}


f_inline static
void traffic_countdown(traffic_t* i)
{
    i->greenlight_countdown--;

    if (i->greenlight_countdown == 5) {
      traffic_setup_next_greenlight(i);
    }

    if (i->greenlight_countdown == 0) {
      traffic_change_lane(i);

      i->greenlight_countdown = i->greenlight_countdown_snapshot;
      i->greenlight_countdown_snapshot = 0;
    }

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
              i->traffic_state = STATE_EMERGENCY;
              // traffic_emergency_lane_select(i);
            }

            traffic_countdown(i);

            break;


        case STATE_YELLOWLIGHT_WAIT:
            traffic_yellowlight_state(i);
            break;


        case STATE_EMERGENCY:
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
    i->lane_select = 0;

    i->light_state[i->lane_select] = LIGHT_GREEN;
    i->light_state[(i->lane_select + 1) % 3] = LIGHT_RED;
    i->light_state[(i->lane_select + 2) % 3] = LIGHT_RED;

    i->greenlight_countdown = MIN_GREEN_LIGHT_TIMING + (i->traffic_car_body[i->lane_select] / 2);
    i->traffic_car_body[i->lane_select] = 0;

    if (i->greenlight_countdown > MAX_GREEN_LIGHT_TIMING)
      i->greenlight_countdown = MAX_GREEN_LIGHT_TIMING;
}


void traffic_print_state(traffic_t* i)
{
    for (u8 count = 0; count < 3; count++)
    {
        pal_printf("CAR COUNT: %d ", i->traffic_car_body[count]);
    }
    pal_printf("\n");

    pal_printf("GREEN COUNT: %d ", i->greenlight_countdown);
    pal_printf("\n");

    pal_printf("YELLOW COUNT: %d ", i->yellowlight_countdown);
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
