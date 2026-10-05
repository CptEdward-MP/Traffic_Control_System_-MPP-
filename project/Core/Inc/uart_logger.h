/*
 * uart_logger.h
 *
 *  Created on: Oct 4, 2026
 *      Author: Manansh Pandey
 */

#ifndef UART_LOGGER_H
#define UART_LOGGER_H

#include <stdint.h>

void UART_SendText(const char *text);
void UART_SendInt(int32_t value);

#endif
