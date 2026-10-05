#include "scheduler.h"

#include "time.h"

static scheduler_task_t
    scheduler_tasks[
        SCHEDULER_MAX_TASKS];

static uint8_t
    scheduler_task_count;

void Scheduler_Init(void)
{
    scheduler_task_count = 0;
}

uint8_t Scheduler_RegisterTask(
    scheduler_callback_t callback,
    void *context,
    uint32_t period_ms)
{
    scheduler_task_t *task;

    if (scheduler_task_count >=
        SCHEDULER_MAX_TASKS)
    {
        return
            SCHEDULER_INVALID_TASK_ID;
    }

    task =
        &scheduler_tasks[
            scheduler_task_count];

    task->callback =
        callback;

    task->context =
        context;

    task->period_ms =
        period_ms;

    task->last_run_ms =
    		 PAL_Time_GetMs();
    scheduler_task_count++;

    return
        (uint8_t)(
            scheduler_task_count - 1U);
}

void Scheduler_Run(void)
{
    uint8_t i;

    uint32_t now;

    now =
        PAL_Time_GetMs();

    for (i = 0;
         i < scheduler_task_count;
         i++)
    {
        scheduler_task_t *task =
            &scheduler_tasks[i];

        if ((now -
             task->last_run_ms)
            >= task->period_ms)
        {
            task->callback(
                task->context);

            task->last_run_ms =
                now;
        }
    }
}
