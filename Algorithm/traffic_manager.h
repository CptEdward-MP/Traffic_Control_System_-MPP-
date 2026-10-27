#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H


#define TRAFFIC_MINIMUM_GREEN 8
#define TRAFFIC_MAXIMUM_GREEN 64

#define TOTAL_NO_GATES 4

#include "typedefs.h"
#include <stdio.h>
#include <stdarg.h>


typedef enum traffic_light_states
{
    LIGHT_RED,
    LIGHT_GREEN,
    LIGHT_YELLOW,

}traffic_light_states;

typedef enum traffic_state
{
    STATE_DEFAULT,
    STATE_YELLOWLIGHT_WAIT,
    STATE_EMERGENCY,
    STATE_HALT,
    STATE_PED_REQUEST,
    STATE_LANE_JUMPERS_WAIT

}traffic_state;

typedef enum traffic_lane_select
{
    LANE0,
    LANE1,
    LANE2

}traffic_lane_select;


/* typedef struct car_jumpers_t */
/* { */
/*     traffic_lane_select jumped_lane; */
/*     traffic_lane_select reached_lane; */
/* }car_jumpers_t; */


typedef struct lane_jumper_t
{
  u8 car_count;
  /* car_jumpers_t jumped_cars[16]; */
}lane_jumper_t;


typedef struct traffic_t
{
    lane_jumper_t lane_jumpers_s;

    u8 traffic_car_body[TOTAL_NO_GATES];

    traffic_light_states light_state[TOTAL_NO_GATES];

    bool traffic_emergency_vehicle;

    bool lane_jumpers;

    traffic_state traffic_state;

    // which lane has green light
    u8 lane_select;
    u8 next_lane_select;

    u8 lane_with_max_peds;

    bool lane2_gate;


    bool pedestrian_request;

    u8 pedestrian_count[TOTAL_NO_GATES];

    u8 yellowlight_countdown;
    /**
     * redlight_countdown[lane_select] does not depend on its countdown
     * It defaults to zero, and relies on this to countdown
     */
    u8 greenlight_countdown;
    u8 greenlight_countdown_snapshot;
    /**
     * How much time the lanes have waited.
     */
    u8 lanes_redlight_waited[TOTAL_NO_GATES];

}traffic_t;


int pal_printf(const char *format, ...);


#endif
