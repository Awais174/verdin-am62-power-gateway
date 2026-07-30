#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "power_alarm_protocol.h"
#include "ti_board_open_close.h"
#include "ti_drivers_open_close.h"

#define ALARM_QUEUE_DEPTH 32U
#define ALARM_DEBOUNCE_MS 10U
#define HEARTBEAT_MS 5000U

struct captured_edge {
    uint32_t ticks;
    uint8_t level;
};

static QueueHandle_t alarm_queue;
static volatile uint32_t sequence;
static volatile uint32_t overflow_count;

/*
 * These functions are implemented by the board-specific adapter created from
 * the matching TI MCU+ SDK GPIO and RPMessage examples.
 */
extern void board_alarm_gpio_init(void (*handler)(void *), void *argument);
extern uint8_t board_alarm_gpio_level(void);
extern void ipc_init(const char *endpoint_name);
extern int ipc_send(const void *data, uint32_t length);

void alarm_gpio_isr(void *argument)
{
    struct captured_edge edge = {
        .ticks = (uint32_t)xTaskGetTickCountFromISR(),
        .level = board_alarm_gpio_level(),
    };
    BaseType_t wake = pdFALSE;

    (void)argument;
    if (xQueueSendFromISR(alarm_queue, &edge, &wake) != pdPASS)
        overflow_count++;
    portYIELD_FROM_ISR(wake);
}

static void send_event(uint8_t type, uint8_t level, uint32_t ticks)
{
    struct power_alarm_event_v1 event = {
        .version = POWER_ALARM_PROTOCOL_VERSION,
        .type = type,
        .input_level = level,
        .sequence = ++sequence,
        .m4_ticks = ticks,
        .overflow_count = overflow_count,
    };

    (void)ipc_send(&event, sizeof(event));
}

static void alarm_task(void *argument)
{
    struct captured_edge edge;

    (void)argument;
    for (;;) {
        if (xQueueReceive(alarm_queue, &edge, portMAX_DELAY) != pdPASS)
            continue;

        vTaskDelay(pdMS_TO_TICKS(ALARM_DEBOUNCE_MS));
        if (board_alarm_gpio_level() != edge.level)
            continue;

        send_event(POWER_ALARM_EVENT_EDGE, edge.level, edge.ticks);
    }
}

static void heartbeat_task(void *argument)
{
    (void)argument;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_MS));
        send_event(POWER_ALARM_EVENT_HEARTBEAT,
                   board_alarm_gpio_level(),
                   (uint32_t)xTaskGetTickCount());
    }
}

int main(void)
{
    System_init();
    Drivers_open();
    Board_driversOpen();

    alarm_queue = xQueueCreate(ALARM_QUEUE_DEPTH, sizeof(struct captured_edge));
    configASSERT(alarm_queue != NULL);

    ipc_init(POWER_ALARM_ENDPOINT_NAME);
    board_alarm_gpio_init(alarm_gpio_isr, NULL);

    configASSERT(xTaskCreate(alarm_task, "alarm", 1024U, NULL,
                             tskIDLE_PRIORITY + 3U, NULL) == pdPASS);
    configASSERT(xTaskCreate(heartbeat_task, "heartbeat", 1024U, NULL,
                             tskIDLE_PRIORITY + 1U, NULL) == pdPASS);

    vTaskStartScheduler();
    for (;;)
        ;
}

