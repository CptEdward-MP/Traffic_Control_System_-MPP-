/*
 * sensor.c
 *
 *  Created on: Oct 5, 2026
 *      Author: Manansh Pandey
 */


#include "sensor.h"
#include "pal_gpio.h"

#define NUM_SENSORS 3

static PAL_GPIO_Pin_t sensor_pins[NUM_SENSORS] =
{
    {GPIOA, 0},
    {GPIOA, 1},
    {GPIOA, 2}
};

void Sensor_Init(void)
{
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
    {
        PAL_GPIO_Init(
            &sensor_pins[i],
            PAL_GPIO_MODE_INPUT,
            PAL_GPIO_PULLDOWN,
            PAL_GPIO_SPEED_LOW
        );
    }
}

uint8_t Sensor_Read(uint8_t sensor)
{
    if (sensor >= NUM_SENSORS)
        return 0U;

    return (uint8_t)PAL_GPIO_Read(
        &sensor_pins[sensor]
    );
}
