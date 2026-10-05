#include "log.h"
#include "traffic_manager.h"

#include <stdio.h>
#include <unistd.h>

#define MIN_GREEN_LIGHT_TIMING 10
#define MAX_GREEN_LIGHT_TIMING 30
#define YELLOWLIGHT_COUNTDOWN 4

#define TOTAL_NO_GATES 3

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
void traffic_setup_next_greenlight(traffic_t* i, traffic_lane_select* lane_select)
{
    i->greenlight_countdown_snapshot = MIN_GREEN_LIGHT_TIMING + (i->traffic_car_body[i->lane_select] / 2);
    i->traffic_car_body[*lane_select] = 0;

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

        // clear lane2 gate
        i->lane2_gate = false;

        i->lane_select = (current_lane + 1) % TOTAL_NO_GATES;

        if (i->lane_select == LANE2) i->lane2_gate = true;

        i->light_state[i->lane_select] = LIGHT_GREEN;
        i->light_state[(i->lane_select + 1) % TOTAL_NO_GATES] = LIGHT_RED;
        i->light_state[(i->lane_select + 2) % TOTAL_NO_GATES] = LIGHT_RED;

        i->traffic_state = STATE_DEFAULT;
    }
    return;
}

f_inline static
void traffic_change_lane(traffic_t* i, traffic_lane_select* lane_select)
{
    i->light_state[*lane_select] = LIGHT_YELLOW;
    i->light_state[(*lane_select + 1) % TOTAL_NO_GATES] = LIGHT_YELLOW;

    i->traffic_state = STATE_YELLOWLIGHT_WAIT;

    i->yellowlight_countdown = YELLOWLIGHT_COUNTDOWN;

    return;
}


f_inline static
void traffic_countdown(traffic_t* i)
{
    i->greenlight_countdown--;

    if (i->greenlight_countdown == 5) {
        traffic_setup_next_greenlight(i, &(i->lane_select));
    }

    if (i->greenlight_countdown == 0) {
        traffic_change_lane(i, &(i->lane_select));

        i->greenlight_countdown = i->greenlight_countdown_snapshot;
        i->greenlight_countdown_snapshot = 0;
    }

    return;
}



f_inline static
void traffic_emergency_lane_select(traffic_t* i)
{
    u8 lane0 = i->traffic_emergency_vehicle & 0x4;
    u8 lane1 = i->traffic_emergency_vehicle & 0x2;
    u8 lane2 = i->traffic_emergency_vehicle & 0x1;

    /**
     * temporarily set the state back to default.
     * Only if it enters the if it enters the if statement
     */
    i->traffic_state = STATE_DEFAULT;

    if ((lane0 != 0) || (lane1 != 0) || (lane2 != 0))
    {

        i->traffic_state = STATE_EMERGENCY;

        /**
         * If more than one lane has emergency enabled, pick in this order :-
         * lane0 > lane1 > lane2
         */
        traffic_lane_select pick_the_greatest_lane = lane0 > lane1  ?
            (lane0 > lane2 ? LANE0 : LANE2)                         :
            (lane1 > lane2 ? LANE1 : LANE2);

        // disable the emergency bit
        i->traffic_emergency_vehicle &= ~(1 << (2 - pick_the_greatest_lane));

        // set lane2's gate to false
        i->lane2_gate = false;
        // change lane
        i->lane_select = pick_the_greatest_lane;
        // if the emergency lane is gate2, set it up
        if (i->lane_select == LANE2) i->lane2_gate = true;

        i->light_state[i->lane_select] = LIGHT_GREEN;
        i->light_state[(i->lane_select + 1) % TOTAL_NO_GATES] = LIGHT_RED;
        i->light_state[(i->lane_select + 2) % TOTAL_NO_GATES] = LIGHT_RED;

        i->greenlight_countdown = MIN_GREEN_LIGHT_TIMING + (i->traffic_car_body[i->lane_select] / 2);

        i->traffic_car_body[i->lane_select] = 0;

        if (i->greenlight_countdown_snapshot > MAX_GREEN_LIGHT_TIMING)
            i->greenlight_countdown_snapshot = MAX_GREEN_LIGHT_TIMING;

        return;

    }

    return;
}

f_inline static
void traffic_emergency_countdown(traffic_t* i)
{
    i->greenlight_countdown--;

    if (i->greenlight_countdown == 0)
    {
        traffic_emergency_lane_select(i);
    }
}


f_inline static
void traffic_state_update(traffic_t* i)
{
    switch(i->traffic_state)
    {
        case STATE_DEFAULT:
            if (i->traffic_emergency_vehicle != 0)
            {
              i->traffic_state = STATE_EMERGENCY;
              traffic_emergency_lane_select(i);
            }
            traffic_countdown(i);
            break;


        case STATE_YELLOWLIGHT_WAIT:
            traffic_yellowlight_state(i);
            break;


        case STATE_EMERGENCY:
            traffic_emergency_countdown(i);
            break;
    }

    return;
}


void traffic_init_state(traffic_t* i, u8 lane[])
{
    for (u8 count = 0; count < TOTAL_NO_GATES; count++)
    {
        i->traffic_car_body[count] = lane[count];
    }

    i->traffic_state = STATE_DEFAULT;
    i->lane_select = 0;
    i->lane2_gate = false;

    i->light_state[i->lane_select] = LIGHT_GREEN;
    i->light_state[(i->lane_select + 1) % TOTAL_NO_GATES] = LIGHT_RED;
    i->light_state[(i->lane_select + 2) % TOTAL_NO_GATES] = LIGHT_RED;

    i->greenlight_countdown = MIN_GREEN_LIGHT_TIMING + (i->traffic_car_body[i->lane_select] / 2);
    i->traffic_car_body[i->lane_select] = 0;

    if (i->greenlight_countdown > MAX_GREEN_LIGHT_TIMING)
      i->greenlight_countdown = MAX_GREEN_LIGHT_TIMING;
}




int main(void)
{
    traffic_t main_state;

    u8 initialize_car_count[3] = {17, 20, 25};

    // setup emergency
    main_state.traffic_emergency_vehicle = 0x6;

    traffic_init_state(&main_state, initialize_car_count);

    while(1)
    {
        traffic_print_state(&main_state);
        traffic_state_update(&main_state);
        sleep(1);
    }

    return 0;
}
