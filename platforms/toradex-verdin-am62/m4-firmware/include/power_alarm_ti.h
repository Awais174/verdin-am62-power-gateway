#ifndef POWER_ALARM_TI_H
#define POWER_ALARM_TI_H

#include <stdint.h>

/*
 * The matching SysConfig project supplies these values. For Verdin AM62 they
 * must resolve to MCU_GPIO0, pin 1 and the M4F-routed bank-0 interrupt.
 */
extern uint32_t gPowerAlarmGpioBaseAddr;
extern uint32_t gPowerAlarmGpioPinNum;
extern uint32_t gPowerAlarmGpioBankIntrNum;

#endif

