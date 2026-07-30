#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <modbus/modbus.h>
#include <pthread.h>
#include <linux/rpmsg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-journal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_CONFIG "/etc/power-gateway/power-gateway.conf"
#define ALARM_VERSION 1
#define ALARM_TYPE_EDGE 1
#define ALARM_TYPE_HEARTBEAT 2
#define ALARM_HELLO 0xA5

struct alarm_event {
    uint8_t version;
    uint8_t type;
    uint8_t input_level;
    uint8_t reserved;
    uint32_t sequence;
    uint32_t m4_ticks;
    uint32_t overflow_count;
} __attribute__((packed));

_Static_assert(sizeof(struct alarm_event) == 16,
               "Alarm protocol structure must remain 16 bytes");

struct config {
    char meter_id[64];
    char host[64];
    char alarm_device[128];
    char rpmsg_ctrl_device[128];
    char status_file[128];
    int port;
    int unit_id;
    int poll_interval_ms;
    int register_address;
    int register_count;
};

struct runtime {
    struct config config;
    atomic_bool alarm_pending;
    atomic_bool m4_online;
    atomic_bool stop;
    atomic_uint alarm_level;
    atomic_uint alarm_sequence;
    atomic_ullong last_m4_mono_ms;
};

static unsigned long long monotonic_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000ULL +
           (unsigned long long)ts.tv_nsec / 1000000ULL;
}

static void set_string(char *destination, size_t size, const char *value)
{
    snprintf(destination, size, "%s", value);
}

static void default_config(struct config *config)
{
    memset(config, 0, sizeof(*config));
    set_string(config->meter_id, sizeof(config->meter_id),
               "pm5560-main-incomer");
    set_string(config->host, sizeof(config->host), "192.168.20.50");
    set_string(config->alarm_device, sizeof(config->alarm_device),
               "/dev/rpmsg0");
    set_string(config->rpmsg_ctrl_device, sizeof(config->rpmsg_ctrl_device),
               "/dev/rpmsg_ctrl0");
    set_string(config->status_file, sizeof(config->status_file),
               "/run/power-gateway/status");
    config->port = 502;
    config->unit_id = 255;
    config->poll_interval_ms = 1000;
}

static void trim(char *value)
{
    char *end;

    while (*value == ' ' || *value == '\t')
        memmove(value, value + 1, strlen(value));
    end = value + strlen(value);
    while (end > value &&
           (end[-1] == '\r' || end[-1] == '\n' ||
            end[-1] == ' ' || end[-1] == '\t'))
        *--end = '\0';
}

static int load_config(const char *path, struct config *config)
{
    FILE *file = fopen(path, "r");
    char line[256];

    if (!file)
        return -errno;

    while (fgets(line, sizeof(line), file)) {
        char *separator;
        char *key = line;
        char *value;

        trim(key);
        if (key[0] == '\0' || key[0] == '#')
            continue;
        separator = strchr(key, '=');
        if (!separator)
            continue;
        *separator = '\0';
        value = separator + 1;
        trim(key);
        trim(value);

        if (!strcmp(key, "METER_ID"))
            set_string(config->meter_id, sizeof(config->meter_id), value);
        else if (!strcmp(key, "METER_HOST"))
            set_string(config->host, sizeof(config->host), value);
        else if (!strcmp(key, "METER_PORT"))
            config->port = atoi(value);
        else if (!strcmp(key, "METER_UNIT_ID"))
            config->unit_id = atoi(value);
        else if (!strcmp(key, "POLL_INTERVAL_MS"))
            config->poll_interval_ms = atoi(value);
        else if (!strcmp(key, "REGISTER_ADDRESS"))
            config->register_address = atoi(value);
        else if (!strcmp(key, "REGISTER_COUNT"))
            config->register_count = atoi(value);
        else if (!strcmp(key, "ALARM_DEVICE"))
            set_string(config->alarm_device, sizeof(config->alarm_device),
                       value);
        else if (!strcmp(key, "RPMSG_CTRL_DEVICE"))
            set_string(config->rpmsg_ctrl_device,
                       sizeof(config->rpmsg_ctrl_device), value);
        else if (!strcmp(key, "STATUS_FILE"))
            set_string(config->status_file, sizeof(config->status_file),
                       value);
    }

    fclose(file);
    return 0;
}

static void publish_status(const struct runtime *runtime, bool meter_online,
                           const char *detail)
{
    char temporary[180];
    FILE *file;

    snprintf(temporary, sizeof(temporary), "%s.tmp",
             runtime->config.status_file);
    file = fopen(temporary, "w");
    if (!file)
        return;

    fprintf(file, "METER=%s\nM4=%s\nALARM=%s\nDETAIL=%s\n",
            meter_online ? "ONLINE" : "OFFLINE",
            atomic_load(&runtime->m4_online) ? "OK" : "DEGRADED",
            atomic_load(&runtime->alarm_level) ? "ACTIVE" : "CLEAR",
            detail);
    if (fclose(file) == 0)
        rename(temporary, runtime->config.status_file);
}

static int create_rpmsg_endpoint(const struct config *config)
{
    struct rpmsg_endpoint_info endpoint = {
        .name = "power-alarm",
        .src = RPMSG_ADDR_ANY,
        .dst = 14,
    };
    int control;
    int result;
    int attempt;

    if (strncmp(config->alarm_device, "/dev/rpmsg", 10))
        return 0;
    if (access(config->alarm_device, R_OK | W_OK) == 0)
        return 0;

    control = open(config->rpmsg_ctrl_device, O_RDWR | O_CLOEXEC);
    if (control < 0)
        return -errno;
    result = ioctl(control, RPMSG_CREATE_EPT_IOCTL, &endpoint);
    if (result < 0 && errno != EEXIST && errno != EBUSY) {
        result = -errno;
        close(control);
        return result;
    }
    close(control);

    for (attempt = 0; attempt < 100; attempt++) {
        if (access(config->alarm_device, R_OK | W_OK) == 0)
            return 0;
        usleep(10000);
    }
    return -ETIMEDOUT;
}

static void *alarm_thread(void *argument)
{
    struct runtime *runtime = argument;
    uint32_t previous_sequence = 0;

    while (!atomic_load(&runtime->stop)) {
        if (create_rpmsg_endpoint(&runtime->config) < 0) {
            atomic_store(&runtime->m4_online, false);
            usleep(1000 * 1000);
            continue;
        }
        int descriptor = open(runtime->config.alarm_device,
                              O_RDWR | O_CLOEXEC);
        if (descriptor < 0) {
            atomic_store(&runtime->m4_online, false);
            usleep(1000 * 1000);
            continue;
        }

        if (!strncmp(runtime->config.alarm_device, "/dev/rpmsg", 10)) {
            uint8_t hello = ALARM_HELLO;

            if (write(descriptor, &hello, sizeof(hello)) != sizeof(hello)) {
                close(descriptor);
                usleep(1000 * 1000);
                continue;
            }
        }

        while (!atomic_load(&runtime->stop)) {
            struct alarm_event event;
            ssize_t length = read(descriptor, &event, sizeof(event));

            if (length != sizeof(event))
                break;
            if (event.version != ALARM_VERSION)
                continue;

            atomic_store(&runtime->last_m4_mono_ms, monotonic_ms());
            atomic_store(&runtime->m4_online, true);
            if (previous_sequence && event.sequence != previous_sequence + 1)
                sd_journal_send("PRIORITY=4",
                                "MESSAGE=Missing M4 alarm sequence",
                                "EVENT_TYPE=m4-sequence-gap",
                                "METER_ID=%s", runtime->config.meter_id,
                                "M4_SEQUENCE=%u", event.sequence, NULL);
            previous_sequence = event.sequence;

            if (event.type == ALARM_TYPE_EDGE) {
                atomic_store(&runtime->alarm_level, event.input_level != 0);
                atomic_store(&runtime->alarm_sequence, event.sequence);
                atomic_store(&runtime->alarm_pending, true);
                sd_journal_send("PRIORITY=4",
                                "MESSAGE=Meter alarm edge received from M4",
                                "EVENT_TYPE=alarm-edge",
                                "METER_ID=%s", runtime->config.meter_id,
                                "ALARM_STATE=%s",
                                event.input_level ? "active" : "clear",
                                "M4_SEQUENCE=%u", event.sequence,
                                "M4_TICKS=%u", event.m4_ticks,
                                "M4_OVERFLOW_COUNT=%u",
                                event.overflow_count, NULL);
            } else if (event.type != ALARM_TYPE_HEARTBEAT) {
                sd_journal_send("PRIORITY=5",
                                "MESSAGE=Unknown M4 event type",
                                "EVENT_TYPE=m4-protocol-error", NULL);
            }
        }

        close(descriptor);
        atomic_store(&runtime->m4_online, false);
    }
    return NULL;
}

static int connect_meter(const struct config *config, modbus_t **result)
{
    modbus_t *context = modbus_new_tcp(config->host, config->port);
    struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

    if (!context)
        return -ENOMEM;
    modbus_set_slave(context, config->unit_id);
    modbus_set_response_timeout(context, timeout.tv_sec, timeout.tv_usec);
    if (modbus_connect(context) == -1) {
        int error = -errno;
        modbus_free(context);
        return error;
    }
    *result = context;
    return 0;
}

static void wait_for_alarm_or_timeout(const struct runtime *runtime,
                                      int timeout_ms)
{
    const int quantum_ms = 10;
    int elapsed = 0;

    while (elapsed < timeout_ms &&
           !atomic_load(&runtime->alarm_pending)) {
        usleep((useconds_t)quantum_ms * 1000);
        elapsed += quantum_ms;
    }
}

int main(int argc, char **argv)
{
    struct runtime runtime = {0};
    pthread_t receiver;
    modbus_t *modbus = NULL;
    uint16_t registers[125];
    bool meter_online = false;
    const char *config_path = argc > 1 ? argv[1] : DEFAULT_CONFIG;

    default_config(&runtime.config);
    if (load_config(config_path, &runtime.config) != 0) {
        sd_journal_send("PRIORITY=3", "MESSAGE=Cannot load configuration",
                        "CONFIG_FILE=%s", config_path, NULL);
        return EXIT_FAILURE;
    }
    if (runtime.config.register_count < 0 ||
        runtime.config.register_count > 125) {
        sd_journal_send("PRIORITY=3", "MESSAGE=Invalid register count", NULL);
        return EXIT_FAILURE;
    }
    if (pthread_create(&receiver, NULL, alarm_thread, &runtime) != 0)
        return EXIT_FAILURE;

    sd_journal_send("PRIORITY=6", "MESSAGE=Power gateway started",
                    "METER_ID=%s", runtime.config.meter_id, NULL);

    for (;;) {
        bool priority = atomic_exchange(&runtime.alarm_pending, false);
        int count;

        if (atomic_load(&runtime.m4_online) &&
            monotonic_ms() - atomic_load(&runtime.last_m4_mono_ms) > 15000)
            atomic_store(&runtime.m4_online, false);

        if (!modbus && connect_meter(&runtime.config, &modbus) != 0) {
            meter_online = false;
            publish_status(&runtime, meter_online, "Modbus connect failed");
            sleep(1);
            continue;
        }

        if (runtime.config.register_count == 0) {
            publish_status(&runtime, true,
                           "Register profile required before commissioning");
            wait_for_alarm_or_timeout(&runtime,
                                      runtime.config.poll_interval_ms);
            continue;
        }

        count = modbus_read_registers(modbus,
                                      runtime.config.register_address,
                                      runtime.config.register_count, registers);
        if (count != runtime.config.register_count) {
            sd_journal_send("PRIORITY=4", "MESSAGE=Modbus poll failed",
                            "EVENT_TYPE=communication-error",
                            "METER_ID=%s", runtime.config.meter_id,
                            "QUALITY=comm-error", NULL);
            meter_online = false;
            modbus_close(modbus);
            modbus_free(modbus);
            modbus = NULL;
            publish_status(&runtime, meter_online, "Modbus poll failed");
            continue;
        }

        meter_online = true;
        sd_journal_send("PRIORITY=6",
                        "MESSAGE=%s Modbus register block read",
                        priority ? "Priority" : "Scheduled",
                        "EVENT_TYPE=%s",
                        priority ? "alarm-confirmed" : "measurement",
                        "METER_ID=%s", runtime.config.meter_id,
                        "QUALITY=good",
                        "REGISTER_ADDRESS=%d",
                        runtime.config.register_address,
                        "REGISTER_COUNT=%d",
                        runtime.config.register_count, NULL);
        publish_status(&runtime, meter_online,
                       priority ? "Alarm confirmed by priority poll"
                                : "Scheduled poll OK");
        wait_for_alarm_or_timeout(
            &runtime, priority ? 10 : runtime.config.poll_interval_ms);
    }

    atomic_store(&runtime.stop, true);
    pthread_join(receiver, NULL);
    return EXIT_SUCCESS;
}
