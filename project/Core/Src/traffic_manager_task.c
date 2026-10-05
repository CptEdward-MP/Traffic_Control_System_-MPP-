/*
 * traffic_manager_task.c
 *
 *  Created on: Oct 5, 2026
 *      Author: Manansh Pandey
 */

#include "traffic_hw.h"
#include "traffic_manager_task.h"

void TrafficManagerTask(void *context)
{
    traffic_t *traffic =
        (traffic_t *)context;

    traffic_state_update(traffic);

    traffic_update_physical_leds(traffic);
}
