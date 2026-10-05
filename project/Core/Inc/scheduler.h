#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define SCHEDULER_MAX_TASKS        16U
#define SCHEDULER_INVALID_TASK_ID  255U

typedef void (*scheduler_callback_t)(
    void *context);

typedef struct
{
    scheduler_callback_t callback;

    void *context;

    uint32_t period_ms;

    uint32_t last_run_ms;

} scheduler_task_t;

void Scheduler_Init(void);

uint8_t Scheduler_RegisterTask(
    scheduler_callback_t callback,
    void *context,
    uint32_t period_ms);

void Scheduler_Run(void);

#endif