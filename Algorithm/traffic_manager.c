#include "log.h"
#include "traffic_manager.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define MIN_GREEN_LIGHT_TIMING 10
#define MAX_GREEN_LIGHT_TIMING 30
#define YELLOWLIGHT_COUNTDOWN 4

#define TOTAL_NO_GATES 3

static const u8 zeroes[3] = {0, 0, 0};

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


void traffic_init_state(traffic_t* i)
{
    i->traffic_state = STATE_DEFAULT;

    traffic_lane_select max_cars_lane = LANE0;
    max_cars_lane = i->traffic_car_body[0] > i->traffic_car_body[1]           ?
        (i->traffic_car_body[0] > i->traffic_car_body[2] ? LANE0 : LANE2)     :
        (i->traffic_car_body[1] > i->traffic_car_body[2] ? LANE1 : LANE2);

    i->lane_select = max_cars_lane;
    i->lane2_gate = false;

    i->traffic_emergency_vehicle = false;

    i->light_state[i->lane_select] = LIGHT_GREEN;
    i->light_state[(i->lane_select + 1) % TOTAL_NO_GATES] = LIGHT_RED;
    i->light_state[(i->lane_select + 2) % TOTAL_NO_GATES] = LIGHT_RED;

    i->greenlight_countdown = MIN_GREEN_LIGHT_TIMING + (i->traffic_car_body[i->lane_select] / 2);
    i->traffic_car_body[i->lane_select] = 0;

    if (i->greenlight_countdown > MAX_GREEN_LIGHT_TIMING)
        i->greenlight_countdown = MAX_GREEN_LIGHT_TIMING;

    i->yellowlight_countdown = 0;


    return;
}



f_inline static
void traffic_setup_next_greenlight(traffic_t* i, traffic_lane_select lane_select)
{
    i->greenlight_countdown_snapshot = MIN_GREEN_LIGHT_TIMING + (i->traffic_car_body[lane_select] / 2);

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
        i->traffic_car_body[i->lane_select] = 0;

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
        traffic_lane_select next_lane = (i->lane_select + 1) % TOTAL_NO_GATES;
        traffic_setup_next_greenlight(i, next_lane);
    }

    if (i->greenlight_countdown == 0) {

        /**
         * SETS YELLOW LIGHT
         */
        traffic_change_lane(i, &(i->lane_select));

        i->greenlight_countdown = i->greenlight_countdown_snapshot;
        i->greenlight_countdown_snapshot = 0;
    }

    return;
}



f_inline static
void traffic_emergency_lane_select(traffic_t* i)
{
    i->traffic_state = false;

    i->traffic_state = STATE_EMERGENCY;

    // set lane2's gate to false
    i->lane2_gate = false;

    for (u8 count = 0; count < 3; count++)
    {
        i->light_state[count] = LIGHT_RED;
    }

    i->greenlight_countdown_snapshot = MIN_GREEN_LIGHT_TIMING;

    return;
}

f_inline static
void traffic_emergency_countdown(traffic_t* i)
{
    i->greenlight_countdown--;

    if (i->greenlight_countdown == 0)
    {
        traffic_init_state(i);
    }

    return;
}


f_inline static
void traffic_state_update(traffic_t* i)
{
    switch(i->traffic_state)
    {
        case STATE_DEFAULT:
            if (i->traffic_emergency_vehicle == true)
            {
                traffic_emergency_lane_select(i);
                break;
            }

            traffic_countdown(i);

            if (memcmp(i->traffic_car_body, zeroes, sizeof(zeroes)) == 0)
            {
                i->traffic_state = STATE_HALT;
                break;
            }

            break;


        case STATE_YELLOWLIGHT_WAIT:
            traffic_yellowlight_state(i);
            break;


        case STATE_EMERGENCY:
            traffic_emergency_countdown(i);
            break;


        case STATE_HALT:
            pal_printf("Aargh! Out of cars\n");

            if (memcmp(i->traffic_car_body, zeroes, sizeof(zeroes)) != 0)
            {
                i->traffic_state = STATE_DEFAULT;
                traffic_init_state(i);
                break;
            }

            break;
    }

    return;
}


void traffic_ENTRY_init_state(traffic_t* i, u8 lane[])
{
    for (u8 count = 0; count < TOTAL_NO_GATES; count++)
    {
        i->traffic_car_body[count] = lane[count];
    }

    traffic_init_state(i);

    return;
}


int main(void)
{
    traffic_t main_state;

    u8 initialize_car_count[3] = {3, 3, 3};

    // setup emergency
    main_state.traffic_emergency_vehicle = 0x6;

    traffic_ENTRY_init_state(&main_state, initialize_car_count);

    input_init();

    while (1)
    {
        int key = input_get_key();

        if (key == 'a' || key == 'A')
            main_state.traffic_car_body[0] += 1;

        if (key == 's' || key == 'S')
            main_state.traffic_car_body[1] += 1;

        if (key == 'd' || key == 'D')
            main_state.traffic_car_body[2] += 1;

        if (key == 'w' || key == 'W')
            main_state.traffic_emergency_vehicle = true;

        traffic_state_update(&main_state);
        traffic_print_state(&main_state);

        /* usleep(100000); */
        sleep(1);
    }

    return 0;
}
