#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>
#include <task.h>

#include <drivers/ipc_rpmsg.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>

#include "power_alarm_protocol.h"

static RPMessage_Object endpoint;
static volatile bool client_ready;
static volatile uint16_t linux_core_id;
static volatile uint16_t linux_endpoint;

static void ipc_receiver_task(void *argument)
{
    RPMessage_CreateParams parameters;
    uint8_t message[8];
    int32_t status;

    (void)argument;
    status = RPMessage_waitForLinuxReady(SystemP_WAIT_FOREVER);
    DebugP_assert(status == SystemP_SUCCESS);

    RPMessage_CreateParams_init(&parameters);
    parameters.localEndPt = POWER_ALARM_M4_ENDPOINT;
    status = RPMessage_construct(&endpoint, &parameters);
    DebugP_assert(status == SystemP_SUCCESS);

    status = RPMessage_announce(CSL_CORE_ID_A53SS0_0,
                                POWER_ALARM_M4_ENDPOINT,
                                POWER_ALARM_RPMSG_SERVICE);
    DebugP_assert(status == SystemP_SUCCESS);

    for (;;) {
        uint16_t length = sizeof(message);
        uint16_t remote_core;
        uint16_t remote_endpoint;

        status = RPMessage_recv(&endpoint, message, &length, &remote_core,
                                &remote_endpoint, SystemP_WAIT_FOREVER);
        if (status == SystemP_SUCCESS && length == 1U &&
            message[0] == POWER_ALARM_HELLO) {
            linux_core_id = remote_core;
            linux_endpoint = remote_endpoint;
            client_ready = true;
        }
    }
}

void ipc_init(const char *endpoint_name)
{
    (void)endpoint_name;
    DebugP_assert(xTaskCreate(ipc_receiver_task, "rpmsg-rx", 2048U, NULL,
                              tskIDLE_PRIORITY + 2U, NULL) == pdPASS);
}

int ipc_send(const void *data, uint32_t length)
{
    if (!client_ready || length > UINT16_MAX)
        return SystemP_FAILURE;

    return RPMessage_send((void *)data, (uint16_t)length, linux_core_id,
                          linux_endpoint, POWER_ALARM_M4_ENDPOINT,
                          SystemP_WAIT_FOREVER);
}
