#include <stdint.h>

#include <drivers/gpio.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/HwiP.h>

#include "power_alarm_ti.h"

static HwiP_Object gpio_hwi;
static void (*application_handler)(void *);
static void *application_argument;

static void gpio_bank_isr(void *argument)
{
    uint32_t bank = GPIO_GET_BANK_INDEX(gPowerAlarmGpioPinNum);
    uint32_t mask = GPIO_GET_BANK_BIT_MASK(gPowerAlarmGpioPinNum);
    uint32_t status = GPIO_getBankIntrStatus(gPowerAlarmGpioBaseAddr, bank);

    (void)argument;
    GPIO_clearBankIntrStatus(gPowerAlarmGpioBaseAddr, bank, status);
    if ((status & mask) != 0U && application_handler != NULL)
        application_handler(application_argument);
}

void board_alarm_gpio_init(void (*handler)(void *), void *argument)
{
    HwiP_Params parameters;
    uint32_t bank = GPIO_GET_BANK_INDEX(gPowerAlarmGpioPinNum);
    int32_t status;

    DebugP_assert(gPowerAlarmGpioPinNum == 1U);
    application_handler = handler;
    application_argument = argument;

    GPIO_setDirMode(gPowerAlarmGpioBaseAddr, gPowerAlarmGpioPinNum,
                    GPIO_DIRECTION_INPUT);
    GPIO_setTrigType(gPowerAlarmGpioBaseAddr, gPowerAlarmGpioPinNum,
                    GPIO_TRIG_TYPE_BOTH_EDGE);
    GPIO_bankIntrEnable(gPowerAlarmGpioBaseAddr, bank);

    HwiP_Params_init(&parameters);
    parameters.intNum = gPowerAlarmGpioBankIntrNum;
    parameters.callback = gpio_bank_isr;
    parameters.args = NULL;
    status = HwiP_construct(&gpio_hwi, &parameters);
    DebugP_assert(status == SystemP_SUCCESS);
}

uint8_t board_alarm_gpio_level(void)
{
    return GPIO_pinRead(gPowerAlarmGpioBaseAddr, gPowerAlarmGpioPinNum) ==
                   GPIO_PIN_HIGH
               ? 1U
               : 0U;
}

