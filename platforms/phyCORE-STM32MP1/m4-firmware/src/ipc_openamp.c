#include "power_alarm_protocol.h"
#include "power_alarm_stm32.h"

#include <stddef.h>

#include "openamp.h"
#include "openamp/open_amp.h"

static struct rpmsg_endpoint alarm_endpoint;
static volatile uint32_t linux_endpoint = RPMSG_ADDR_ANY;
static volatile int linux_ready;

static int receive_callback(struct rpmsg_endpoint *endpoint, void *data,
                            size_t length, uint32_t source, void *private_data)
{
    const uint8_t *message = data;

    (void)endpoint;
    (void)private_data;
    if (length == 1U && message[0] == POWER_ALARM_HELLO) {
        linux_endpoint = source;
        linux_ready = 1;
    }
    return RPMSG_SUCCESS;
}

static void unbind_callback(struct rpmsg_endpoint *endpoint)
{
    (void)endpoint;
    linux_ready = 0;
    linux_endpoint = RPMSG_ADDR_ANY;
}

void ipc_init(void)
{
    if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL) != HAL_OK)
        Error_Handler();

    if (OPENAMP_create_endpoint(&alarm_endpoint,
                                POWER_ALARM_RPMSG_SERVICE,
                                POWER_ALARM_M4_ENDPOINT,
                                receive_callback,
                                unbind_callback) != 0)
        Error_Handler();
}

void ipc_poll(void)
{
    (void)OPENAMP_check_for_message();
}

int ipc_send(const void *data, uint32_t length)
{
    if (!linux_ready)
        return -1;

    return rpmsg_sendto(&alarm_endpoint, data, length, linux_endpoint);
}
