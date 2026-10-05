/*
 * sensor.h
 *
 *  Created on: Oct 5, 2026
 *      Author: Manansh Pandey
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

void Sensor_Init(void);

uint8_t Sensor_Read(uint8_t sensor);

#endif
