/*
 * pc_display.c
 *
 *  Created on: Oct 6, 2026
 *      Author: Manansh Pandey
 */


#include "pc_display.h"
#include "usb.h"

#include <stdio.h>

void PC_Display_Send(const traffic_t *traffic)
{
    char packet[64];

    snprintf(
        packet,
        sizeof(packet),
        "L0:%u,L1:%u,L2:%u,G:%u,T:%u\r\n",
        traffic->traffic_car_body[0],
        traffic->traffic_car_body[1],
        traffic->traffic_car_body[2],
        traffic->lane_select,
        traffic->greenlight_countdown
    );

    USB_SendString(packet);
}
