# Verdin AM62 M4F power-alarm firmware

This is the real-hardware firmware source for the AM62 Cortex-M4F. It captures
both edges from the PM5560 D1 alarm contact on Verdin SODIMM 206, debounces the
signal under FreeRTOS, and sends the versioned event structure to Linux through
TI RPMessage.

## Pinned development tools

| Component | Selection |
|---|---|
| MCU+ SDK | AM62x `12.00.00.27` |
| Compiler | TI ARM Clang `4.0.1.LTS` (`tiarmclang`) |
| SysConfig | `1.26.2` |
| Board variant | `am62x-sk` |
| Core/OS | `m4fss0-0_freertos` |
| Reference project | `examples/drivers/ipc/ipc_rpmsg_echo_linux` |

The exact versions are intentional. Firmware, generated SysConfig files, Linux
device tree, and memory carveouts must be released as one tested set.

## Fixed pin allocation

The authoritative allocation is in
[`config/power-alarm-pin.json`](config/power-alarm-pin.json):

- Verdin X1/SODIMM pin 206 (`GPIO_1`);
- AM62 ball B8, pad `MCU_SPI0_CS1`;
- mux mode 7, function `MCU_GPIO0_1`;
- MCU GPIO controller 0, pin 1, bank 0;
- both-edge interrupt routed only to M4F;
- 1.8 V input after external isolation and conditioning.

## Create the MCU+ SDK project

Install the pinned MCU+ SDK, TI ARM Clang, and SysConfig. Then stage this source
over a local copy of TI's Linux RPMessage example:

```sh
export SDK_INSTALL_PATH=$HOME/ti/mcu_plus_sdk_am62x_12_00_00_27
./scripts/stage-ti-project.sh
```

Open SysConfig:

```sh
make -s -C \
  "$SDK_INSTALL_PATH/examples/drivers/ipc/ipc_rpmsg_echo_linux/am62x-sk/m4fss0-0_freertos/ti-arm-clang" \
  syscfg-gui
```

Keep the reference project's Linux IPC/resource-table and M4 DDR/MPU settings.
Add one GPIO instance named `CONFIG_GPIO0` with these selections:

| SysConfig field | Value |
|---|---|
| Domain/instance | MCU domain, `MCU_GPIO0` |
| SoC pad | `MCU_SPI0_CS1` / B8 |
| GPIO pin | 1 |
| Direction | input |
| Trigger | rising and falling edges |
| Interrupt destination | M4F, bank 0 |
| Pull | defined by the validated carrier input circuit |

Generate files and check that `ti_drivers_config.h` contains
`CONFIG_GPIO0_BASE_ADDR`, `CONFIG_GPIO0_PIN`, and `CONFIG_GPIO0_INTR`.

Build and collect the RemoteProc ELF:

```sh
./scripts/build-ti-project.sh
```

The result is copied to:

```text
build/am62-mcu-m4f0_0-fw
```

Copy that validated file into the Yocto binary recipe as described in
`meta-power-gateway/recipes-bsp/m4-alarm-firmware/README.md`.

## Source behavior

- `board_ti.c` uses TI GPIO and HwiP APIs and clears bank interrupt status.
- `main.c` keeps the ISR bounded and uses an ISR-safe FreeRTOS queue.
- `ipc_ti.c` announces the standard `rpmsg_chrdev` service at endpoint 14.
- Linux sends `POWER_ALARM_HELLO`; the M4F records the Linux endpoint and then
  sends edge and heartbeat events.
- The RPMessage resource table and non-cacheable DDR mappings remain generated
  from TI's Linux RPMessage reference project.

Use Verdin AM62 hardware to validate GPIO, RemoteProc, RPMsg, and interrupt
latency.
