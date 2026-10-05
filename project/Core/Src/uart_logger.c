/*
 * uart_logger.c
 *
 *  Created on: Oct 4, 2026
 *      Author: Manansh Pandey
 */


#include "uart_logger.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;

void UART_SendText(const char *text)
{
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)text,
        strlen(text),
        HAL_MAX_DELAY
    );
}

void UART_SendInt(int32_t value)
{
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%ld", (long)value);

    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)buffer,
        strlen(buffer),
        HAL_MAX_DELAY
    );
}
