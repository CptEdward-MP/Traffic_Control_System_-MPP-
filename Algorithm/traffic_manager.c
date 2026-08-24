#include "traffic_manager.h"
#include <stdio.h>


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
void traffic_greenlight_countdown(traffic_t* i)
{
    if (i->traffic_car_body[i->lane_select] == 0)
    {
        i->greenlight_countdown = 0;
        return;
    }

    if (i->greenlight_countdown == 0) return;

    i->greenlight_countdown--;
    i->traffic_car_body[i->lane_select]--;

    return;
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

            traffic_greenlight_countdown(i);

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

            break;

        case STATE_YELLOWLIGHT_WAIT:

            if (i->yellowlight_countdown == 0)
            {
                i->traffic_state = STATE_DEFAULT;
                i->light_state[i->lane_select] = LIGHT_GREEN;
            }

            i->yellowlight_countdown--;
            break;
    }

    return;
}

int main(void)
{
  printf("Hello World!\n");
  return 0;
}
