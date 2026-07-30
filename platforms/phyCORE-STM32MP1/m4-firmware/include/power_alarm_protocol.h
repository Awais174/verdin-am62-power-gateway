#ifndef POWER_ALARM_PROTOCOL_H
#define POWER_ALARM_PROTOCOL_H

#include <stdint.h>

#define POWER_ALARM_PROTOCOL_VERSION 1U
#define POWER_ALARM_ENDPOINT_NAME "power-alarm"
#define POWER_ALARM_RPMSG_SERVICE "rpmsg-raw"
#define POWER_ALARM_M4_ENDPOINT 14U
#define POWER_ALARM_HELLO 0xA5U

enum power_alarm_event_type {
    POWER_ALARM_EVENT_EDGE = 1,
    POWER_ALARM_EVENT_HEARTBEAT = 2,
    POWER_ALARM_EVENT_OVERFLOW = 3,
};

struct power_alarm_event_v1 {
    uint8_t version;
    uint8_t type;
    uint8_t input_level;
    uint8_t reserved;
    uint32_t sequence;
    uint32_t m4_ticks;
    uint32_t overflow_count;
} __attribute__((packed));

_Static_assert(sizeof(struct power_alarm_event_v1) == 16,
               "RPMessage protocol size changed");

#endif
