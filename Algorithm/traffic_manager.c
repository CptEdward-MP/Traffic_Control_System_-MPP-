#include "traffic_manager.h"
#include "log.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MIN_GREEN_LIGHT_TIMING 10
#define MAX_GREEN_LIGHT_TIMING 30
#define YELLOWLIGHT_COUNTDOWN 4
#define MAX_RED_WAIT_TIME 60
#define PEDESTRIAN_TIMING 20
#define TEMPORARY_HALT 20


static const u8 zeroes[TOTAL_NO_GATES] = {0, 0, 0, 0};



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

void find_max_peds(traffic_t* i)
{
    u8 max_index = 0;

    u8* arr = i->pedestrian_count;

    for (u8 count = 0; count < TOTAL_NO_GATES; count++)
    {
        if (arr[count] > arr[max_index]) {
            max_index = count;
        }
    }

    i->lane_with_max_peds = max_index;
}


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

    for (u8 lanes = 0; lanes < TOTAL_NO_GATES; lanes++) {
        i->lanes_redlight_waited[lanes] = 0;
        i->pedestrian_count[lanes] = 0;
    }

    i->pedestrian_request = false;

    i->lane_jumpers = false;

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


static f_inline
bool better_lane(traffic_t *i, u8 a, traffic_lane_select b)
{
    bool a_starved = i->lanes_redlight_waited[a] >= MAX_RED_WAIT_TIME;
    bool b_starved = i->lanes_redlight_waited[b] >= MAX_RED_WAIT_TIME;

    if (a_starved != b_starved)
        return a_starved;

    if (i->traffic_car_body[a] != i->traffic_car_body[b])
        return i->traffic_car_body[a] > i->traffic_car_body[b];

    return i->lanes_redlight_waited[a] > i->lanes_redlight_waited[b];
}


static f_inline
traffic_lane_select traffic_select_next_lane(traffic_t *i)
{
    traffic_lane_select best_lane = (traffic_lane_select)i->lane_select;
    bool found = false;

    for (u8 lane = 0; lane < TOTAL_NO_GATES; lane++)
    {
        if (lane == i->lane_select || i->traffic_car_body[lane] == 0)
            continue;

        if (!found ||
            better_lane(i, lane, best_lane))
        {
            best_lane = (traffic_lane_select)lane;
            found = true;
        }
    }

    return best_lane;
}


f_inline static
void traffic_yellowlight_state(traffic_t* i)
{
    i->yellowlight_countdown--;

    if (i->yellowlight_countdown == 0)
    {
        i->traffic_car_body[i->lane_select] = 0;

        i->lane_select = i->next_lane_select;
        i->lanes_redlight_waited[i->lane_select] = 0;

        //lane2 gate.
        i->lane2_gate = (i->lane_select == LANE2);

        i->light_state[i->lane_select] = LIGHT_GREEN;

        for (u8 lane = 0; lane < TOTAL_NO_GATES; lane++)
        {
            if (lane != i->lane_select)
                i->light_state[lane] = LIGHT_RED;
        }

        i->traffic_state = STATE_DEFAULT;
    }

    return;
}

f_inline static
void traffic_change_lane(traffic_t* i)
{
    i->light_state[i->lane_select] = LIGHT_YELLOW;
    i->light_state[i->next_lane_select] = LIGHT_YELLOW;

    i->light_state[TOTAL_NO_GATES - i->lane_select - i->next_lane_select] = LIGHT_RED;

    i->traffic_state = STATE_YELLOWLIGHT_WAIT;

    if (i->pedestrian_request == true)
    {
        // set max_peds
        find_max_peds(i);
        // used as pedestrian countdown
        i->greenlight_countdown = i->pedestrian_count[i->lane_with_max_peds] / 4;

        if (i->greenlight_countdown < MIN_GREEN_LIGHT_TIMING)
          i->greenlight_countdown = MIN_GREEN_LIGHT_TIMING;

        if (i->greenlight_countdown > MAX_GREEN_LIGHT_TIMING)
          i->greenlight_countdown = MAX_GREEN_LIGHT_TIMING;


        for (u8 count = 0; count < TOTAL_NO_GATES; count++)
            i->light_state[count] = LIGHT_RED;

        i->traffic_state = STATE_PED_REQUEST;
    }


    i->yellowlight_countdown = YELLOWLIGHT_COUNTDOWN;
}


f_inline static
void traffic_countdown(traffic_t* i)
{
    i->greenlight_countdown--;

    i->lanes_redlight_waited[(i->lane_select + 1) % TOTAL_NO_GATES]++;
    i->lanes_redlight_waited[(i->lane_select + 2) % TOTAL_NO_GATES]++;

    if (i->greenlight_countdown == 5)
    {
        i->next_lane_select = traffic_select_next_lane(i);
        traffic_setup_next_greenlight(i, i->next_lane_select);
    }

    u8 lane_jump_sensor = car_jumper_sensor();

    if (lane_jump_sensor < TOTAL_NO_GATES)
    {
        i->greenlight_countdown = TEMPORARY_HALT;

        for (u8 count = 0; count < TOTAL_NO_GATES; count++)
        {
            i->light_state[count] = LIGHT_RED;
        }

        i->lane_jumpers = true;
    }


    if (i->greenlight_countdown == 0) {

        /**
         * SETS YELLOW LIGHT
         */
        traffic_change_lane(i);

        i->greenlight_countdown = i->greenlight_countdown_snapshot;
        i->greenlight_countdown_snapshot = 0;
    }

    return;
}



f_inline static
void traffic_emergency_lane_select(traffic_t* i)
{
    i->traffic_state = STATE_EMERGENCY;

    // set lane2's gate to false
    i->lane2_gate = false;

    for (u8 count = 0; count < TOTAL_NO_GATES; count++)
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
            }


            if (i->lane_jumpers == true)
            {
                i->lane_jumpers_s.car_count++;
                i->traffic_state = STATE_LANE_JUMPERS_WAIT;
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

        case STATE_PED_REQUEST:
          i->pedestrian_request = false;

          i->greenlight_countdown--;

          if (i->greenlight_countdown == 0)
          {
              i->traffic_state = STATE_DEFAULT;
              traffic_init_state(i);
          }
          break;


        case STATE_LANE_JUMPERS_WAIT:
          i->lane_jumpers = false;
          i->lane_jumpers_s.car_count--;

          i->greenlight_countdown--;

          if (i->greenlight_countdown == 0)
          {
              i->traffic_state = STATE_DEFAULT;
              traffic_init_state(i);
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

    u8 initialize_car_count[TOTAL_NO_GATES] = {34, 23, 12, 43};

    traffic_ENTRY_init_state(&main_state, initialize_car_count);

    input_init();

    time_t last_update = time(NULL);

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

        time_t now = time(NULL);

        if (now != last_update)
        {
            traffic_state_update(&main_state);
            traffic_print_state(&main_state);

            last_update = now;

        }


        /* usleep(100000); */
    }

    return 0;
}
