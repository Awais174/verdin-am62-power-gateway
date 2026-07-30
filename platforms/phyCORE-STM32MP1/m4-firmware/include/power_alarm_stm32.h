#ifndef POWER_ALARM_STM32_H
#define POWER_ALARM_STM32_H

#include <stdint.h>

typedef void (*power_alarm_irq_handler_t)(uint32_t ticks, uint8_t level);

void board_alarm_gpio_init(power_alarm_irq_handler_t handler);
uint8_t board_alarm_gpio_level(void);
uint32_t board_alarm_ticks(void);

void ipc_init(void);
void ipc_poll(void);
int ipc_send(const void *data, uint32_t length);
void Error_Handler(void);

#endif
