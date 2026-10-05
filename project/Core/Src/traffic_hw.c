#include "traffic_hw.h"
#include "pal_gpio.h"



typedef struct {
    PAL_GPIO_Pin_t red;
    PAL_GPIO_Pin_t green;
} lane_led_map_t;

static lane_led_map_t lane_leds[3] = {

    // LANE 0: Red=PB5, Green=PB15
    { .red = {GPIOB, 5}, .green = {GPIOB, 15} },

    // LANE 1: Red=PB4, Green=PB13
    { .red = {GPIOB, 4}, .green = {GPIOB, 13} },

    // LANE 2: Red=PA15, Green=PB12
    { .red = {GPIOA, 15}, .green = {GPIOB, 12} }
};


void traffic_hw_init(void)
{
    for (u8 lane = 0; lane < 3; lane++)
    {
        PAL_GPIO_Init(&lane_leds[lane].red,    PAL_GPIO_MODE_OUTPUT, PAL_GPIO_NOPULL, PAL_GPIO_SPEED_LOW);
        //PAL_GPIO_Init(&lane_leds[lane].yellow, PAL_GPIO_MODE_OUTPUT, PAL_GPIO_NOPULL, PAL_GPIO_SPEED_LOW);
        PAL_GPIO_Init(&lane_leds[lane].green,  PAL_GPIO_MODE_OUTPUT, PAL_GPIO_NOPULL, PAL_GPIO_SPEED_LOW);
    }
}

void traffic_update_physical_leds(const traffic_t *i)
{
    for (u8 lane = 0; lane < 3; lane++)
    {
        switch (i->light_state[lane])
        {
            case LIGHT_GREEN:
                PAL_GPIO_Write(&lane_leds[lane].green,  PAL_GPIO_HIGH);
                //PAL_GPIO_Write(&lane_leds[lane].yellow, PAL_GPIO_LOW);
                PAL_GPIO_Write(&lane_leds[lane].red,    PAL_GPIO_LOW);
                break;

            case LIGHT_YELLOW:
                PAL_GPIO_Write(&lane_leds[lane].green,  PAL_GPIO_LOW);
                //PAL_GPIO_Write(&lane_leds[lane].yellow, PAL_GPIO_HIGH);
                PAL_GPIO_Write(&lane_leds[lane].red,    PAL_GPIO_LOW);
                break;

            case LIGHT_RED:
            default:
                PAL_GPIO_Write(&lane_leds[lane].green,  PAL_GPIO_LOW);
                //PAL_GPIO_Write(&lane_leds[lane].yellow, PAL_GPIO_LOW);
                PAL_GPIO_Write(&lane_leds[lane].red,    PAL_GPIO_HIGH);
                break;
        }
    }

}


