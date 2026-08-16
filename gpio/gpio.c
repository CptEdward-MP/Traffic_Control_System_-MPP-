#include "pal_gpio.h"

static void PAL_GPIO_EnableClock(GPIO_TypeDef *port)
{
    /*
     * STM32F411 GPIO ports A-E and H are on AHB1.
     *
     * RCC AHB1ENR:
     * GPIOAEN = bit 0
     * GPIOBEN = bit 1
     * GPIOCEN = bit 2
     * GPIODEN = bit 3
     * GPIOEEN = bit 4
     * GPIOHEN = bit 7
     */
    if (port == GPIOA)
    {
        RCC->AHB1ENR |= (1U << 0);
    }
    else if (port == GPIOB)
    {
        RCC->AHB1ENR |= (1U << 1);
    }
    else if (port == GPIOC)
    {
        RCC->AHB1ENR |= (1U << 2);
    }
    else if (port == GPIOD)
    {
        RCC->AHB1ENR |= (1U << 3);
    }
    else if (port == GPIOE)
    {
        RCC->AHB1ENR |= (1U << 4);
    }
    else if (port == GPIOH)
    {
        RCC->AHB1ENR |= (1U << 7);
    }
}

void PAL_GPIO_Init(PAL_GPIO_Pin_t *gpio,
                   PAL_GPIO_Mode_t mode,
                   PAL_GPIO_Pull_t pull,
                   PAL_GPIO_Speed_t speed)
{
    uint32_t shift = (uint32_t)gpio->pin * 2U;

    PAL_GPIO_EnableClock(gpio->port);

    /* MODER: clear the 2-bit field first. */
    gpio->port->MODER &= ~(3U << shift);

    if (mode == PAL_GPIO_MODE_OUTPUT)
    {
        gpio->port->MODER |= (1U << shift);

        /* Push-pull output. */
        gpio->port->OTYPER &= ~(1U << gpio->pin);
    }

    /* OSPEEDR: 2 bits per pin. */
    gpio->port->OSPEEDR &= ~(3U << shift);
    gpio->port->OSPEEDR |= ((uint32_t)speed << shift);

    /* PUPDR: 2 bits per pin. */
    gpio->port->PUPDR &= ~(3U << shift);
    gpio->port->PUPDR |= ((uint32_t)pull << shift);
}

void PAL_GPIO_Set(PAL_GPIO_Pin_t *gpio)
{
    /* BSRR bits 0-15 set the corresponding ODR bits. */
    gpio->port->BSRR = (1U << gpio->pin);
}

void PAL_GPIO_Reset(PAL_GPIO_Pin_t *gpio)
{
    /* BSRR bits 16-31 reset the corresponding ODR bits. */
    gpio->port->BSRR = (1U << ((uint32_t)gpio->pin + 16U));
}

void PAL_GPIO_Write(PAL_GPIO_Pin_t *gpio, PAL_GPIO_State_t state)
{
    if (state == PAL_GPIO_HIGH)
    {
        PAL_GPIO_Set(gpio);
    }
    else
    {
        PAL_GPIO_Reset(gpio);
    }
}

void PAL_GPIO_Toggle(PAL_GPIO_Pin_t *gpio)
{
    if ((gpio->port->ODR & (1U << gpio->pin)) != 0U)
    {
        PAL_GPIO_Reset(gpio);
    }
    else
    {
        PAL_GPIO_Set(gpio);
    }
}

PAL_GPIO_State_t PAL_GPIO_Read(PAL_GPIO_Pin_t *gpio)
{
    if ((gpio->port->IDR & (1U << gpio->pin)) != 0U)
    {
        return PAL_GPIO_HIGH;
    }

    return PAL_GPIO_LOW;
}
