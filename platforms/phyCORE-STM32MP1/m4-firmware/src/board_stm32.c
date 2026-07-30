#include "power_alarm_stm32.h"

#include "FreeRTOS.h"
#include "lock_resource.h"
#include "stm32mp1xx_hal.h"

#define POWER_ALARM_PIN GPIO_PIN_0
#define POWER_ALARM_PORT GPIOG

static power_alarm_irq_handler_t alarm_handler;

void board_alarm_gpio_init(power_alarm_irq_handler_t handler)
{
    GPIO_InitTypeDef gpio = {
        .Pin = POWER_ALARM_PIN,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };

    alarm_handler = handler;

    __HAL_RCC_HSEM_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    PERIPH_LOCK(EXTI);
    PERIPH_LOCK(POWER_ALARM_PORT);
    HAL_GPIO_Init(POWER_ALARM_PORT, &gpio);
    PERIPH_UNLOCK(POWER_ALARM_PORT);
    PERIPH_UNLOCK(EXTI);

    HAL_NVIC_SetPriority(EXTI0_IRQn,
                         configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0U);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

uint8_t board_alarm_gpio_level(void)
{
    return HAL_GPIO_ReadPin(POWER_ALARM_PORT, POWER_ALARM_PIN) == GPIO_PIN_SET;
}

uint32_t board_alarm_ticks(void)
{
    return HAL_GetTick();
}

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(POWER_ALARM_PIN);
}

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == POWER_ALARM_PIN && alarm_handler != NULL)
        alarm_handler(HAL_GetTick(), board_alarm_gpio_level());
}
