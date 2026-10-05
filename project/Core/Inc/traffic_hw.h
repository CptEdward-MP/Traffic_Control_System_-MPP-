/*
 * traffic_hw.h
 *
 *  Created on: Oct 4, 2026
 *      Author: Bharath V
 */

#ifndef INC_TRAFFIC_HW_H_
#define INC_TRAFFIC_HW_H_
#include "traffic_manager.h"

void traffic_hw_init(void);
void traffic_update_physical_leds(const traffic_t *i);


#endif /* INC_TRAFFIC_HW_H_ */
