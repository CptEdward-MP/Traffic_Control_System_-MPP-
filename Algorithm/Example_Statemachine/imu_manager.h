#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include "imu.h"

typedef enum
{
    STATE_INIT,
    STATE_READ,
    STATE_RESET,
    STATE_WAIT_RECONNECT
} AppState;

typedef enum
{
    ACTION_NONE,
    ACTION_PRINT_DATA,
    ACTION_PRINT_ERR,
    ACTION_WAIT
} Action;

void IMUManager_Init(void);
void IMUManager_Run(void);

AppState IMUManager_GetState(void);
Action IMUManager_GetLastAction(void);
const imu_data_t *IMUManager_GetData(void);

#endif