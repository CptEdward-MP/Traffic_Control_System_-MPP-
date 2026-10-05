/*
 * vehicle.c
 *
 *  Created on: Oct 5, 2026
 *      Author: Manansh Pandey
 */


#include "vehicle_task.h"
#include "sensor.h"

#define NUM_LANES 3

static uint8_t previous_sensor_state[NUM_LANES];

void VehicleTask(void *context)
{
    traffic_t *traffic = (traffic_t *)context;

    for (uint8_t lane = 0; lane < NUM_LANES; lane++)
    {
        uint8_t current_state =
        		Sensor_Read(lane);

        /*
         * LOW -> HIGH
         * One vehicle detected.
         */
        if ((current_state == 1U) &&
            (previous_sensor_state[lane] == 0U))
        {
            traffic->traffic_car_body[lane]++;
        }

        previous_sensor_state[lane] =
            current_state;
    }
}
