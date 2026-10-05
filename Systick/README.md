PAL_Time — Simple Documentation
1. What is PAL_Time?

PAL_Time provides simple time functions to the application.

It allows us to wait, get the current time, and check elapsed time without knowing how the timing works internally.

2. Initialize
PAL_Time_Init();

Called once when the program starts.

Starts the timing system.

3. Delay
PAL_Time_DelayMs(500);

Waits for approximately 500 milliseconds.

For example, for an LED:

PAL_GPIO_Set(&LED1);
PAL_Time_DelayMs(500);
PAL_GPIO_Reset(&LED1);

DelayMs = wait for some time.

4. Get Current Time
uint32_t now = PAL_Time_GetMs();

Gives the current time in milliseconds.

GetMs = find out what the current time is.

5. Check Elapsed Time

Save the starting time:

uint32_t start = PAL_Time_GetMs();

Later:

uint32_t elapsed = PAL_Time_GetMs() - start;

elapsed tells us how much time has passed.

This can be used for periodic tasks:

if ((PAL_Time_GetMs() - last_time) >= 100U)
{
    last_time = PAL_Time_GetMs();

    /* Do something */
}

This makes the task run approximately every 100 ms.

6. Main Functions
Function	Meaning
PAL_Time_Init()	Start timing
PAL_Time_DelayMs()	Wait
PAL_Time_GetMs()	Get current time
Simple flow
PAL_Time_Init()
       ↓
Start timing
       ↓
GetMs() → Know the time
       ↓
DelayMs() → Wait
       ↓
Use elapsed time for periodic tasks

PAL_Time gives the application a simple way to work with time without dealing with the underlying timing implementation.
