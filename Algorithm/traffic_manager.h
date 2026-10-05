#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H


#define TRAFFIC_MINIMUM_GREEN 8
#define TRAFFIC_MAXIMUM_GREEN 64


#if defined(_MSC_VER)
    #define f_inline __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define f_inline inline __attribute__((always_inline))
#else
    #define f_inline inline
#endif


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


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
    STATE_EMERGENCY

}traffic_state;

typedef enum traffic_lane_select
{
    LANE0,
    LANE1,
    LANE2

}traffic_lane_select;

typedef struct traffic_t
{
    u8 traffic_car_body[3];

    traffic_light_states light_state[3];

    /**
     * Stores 3 booleans. 0bXXXXXBBB.
     * Each bit represents in what lane contains emergency vehicle
     * 0bXXXXX[lane0][lane1][lane2]
     * Stores in big endian
     */
    u8 traffic_emergency_vehicle;

    traffic_state traffic_state;

    // which lane has green light
    traffic_lane_select lane_select;
    bool lane2_gate;

    u8 yellowlight_countdown;
    /**
     * redlight_countdown[lane_select] does not depend on its countdown
     * It defaults to zero, and relies on this to countdown
     */
    u8 greenlight_countdown;
    u8 greenlight_countdown_snapshot;

}traffic_t;


int pal_printf(const char *format, ...);


#endif
