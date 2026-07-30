#include <stdint.h>

#include "power_alarm_protocol.h"
#include "power_alarm_stm32.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "stm32mp1xx_hal.h"

#define ALARM_QUEUE_DEPTH 16U
#define ALARM_DEBOUNCE_MS 10U
#define HEARTBEAT_MS 5000U
#define GATEWAY_TASK_STACK_WORDS (configMINIMAL_STACK_SIZE * 4U)
#define GATEWAY_TASK_PRIORITY (tskIDLE_PRIORITY + 2U)

struct captured_edge {
    uint32_t ticks;
    uint8_t level;
};

static QueueHandle_t alarm_queue;
IPCC_HandleTypeDef hipcc;
static volatile uint32_t overflow_count;
static uint32_t sequence;

static void ipcc_init(void)
{
    hipcc.Instance = IPCC;
    if (HAL_IPCC_Init(&hipcc) != HAL_OK)
        Error_Handler();
}

static void alarm_gpio_isr(uint32_t ticks, uint8_t level)
{
    const struct captured_edge edge = {
        .ticks = ticks,
        .level = level,
    };
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (xQueueSendFromISR(alarm_queue, &edge,
                          &higher_priority_task_woken) != pdPASS)
        overflow_count++;

    portYIELD_FROM_ISR(higher_priority_task_woken);
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

static void gateway_task(void *argument)
{
    struct captured_edge edge;
    uint32_t last_heartbeat;

    (void)argument;

    ipc_init();
    board_alarm_gpio_init(alarm_gpio_isr);
    last_heartbeat = board_alarm_ticks();

    for (;;) {
        uint32_t now;

        ipc_poll();
        now = board_alarm_ticks();

        while (xQueuePeek(alarm_queue, &edge, 0U) == pdPASS) {
            if ((uint32_t)(now - edge.ticks) < ALARM_DEBOUNCE_MS)
                break;

            (void)xQueueReceive(alarm_queue, &edge, 0U);
            if (board_alarm_gpio_level() == edge.level)
                send_event(POWER_ALARM_EVENT_EDGE, edge.level, edge.ticks);
        }

        if ((uint32_t)(now - last_heartbeat) >= HEARTBEAT_MS) {
            last_heartbeat = now;
            send_event(POWER_ALARM_EVENT_HEARTBEAT,
                       board_alarm_gpio_level(), now);
        }

        vTaskDelay(pdMS_TO_TICKS(1U));
    }
}

int main(void)
{
    HAL_Init();
    if (IS_ENGINEERING_BOOT_MODE())
        Error_Handler();

    ipcc_init();

    alarm_queue = xQueueCreate(ALARM_QUEUE_DEPTH,
                               sizeof(struct captured_edge));
    if (alarm_queue == NULL)
        Error_Handler();

    if (xTaskCreate(gateway_task, "power-gateway",
                    GATEWAY_TASK_STACK_WORDS, NULL,
                    GATEWAY_TASK_PRIORITY, NULL) != pdPASS)
        Error_Handler();

    vTaskStartScheduler();
    Error_Handler();
}

void Error_Handler(void)
{
    __disable_irq();
    for (;;)
        ;
}
