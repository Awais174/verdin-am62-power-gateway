# STM32MP157C Cortex-M4 Alarm Firmware

This FreeRTOS firmware captures the isolated power-meter alarm input on the
phyBOARD-Sargas and sends fixed-size events to Linux through STM32 OpenAMP and
RPMsg.

## Fixed Hardware Allocation

| Attribute | Value |
|---|---|
| Carrier | PHYTEC phyBOARD-Sargas PCB 1517.2 |
| Connector | X3 Arduino header |
| Alarm input | Interface pin 8 (`D7`) |
| Ground | Interface pin 15 (`GND`) |
| SoC signal | `PG0` / `X_DFSDM1_DATIN0` |
| Interrupt | `EXTI0`, both edges |
| Electrical level | Protected 3.3 V input, active high |
| Owner | STM32MP157C Cortex-M4 |

PHYTEC documents Arduino D7 as a direct 3.3 V GPIO or DFSDM input. It defaults
to GPIO, requires no carrier jumper, and is easier to wire for a prototype than
the motor-control connector. Connect the meter through an isolated, protected
3.3 V carrier input; do not connect a field signal directly to the SoC.

## Pinned Build Environment

- STM32CubeMP1 firmware package: `v1.6.0`
- STM32CubeMP1 commit: `b9a31179d5bf80b3958c3653153bfd4c3a7fc5d5`
- Cross compiler: GNU Arm Embedded `11.2-2022.02`
- Yocto provider: `gcc-arm-none-eabi-native`
- RTOS: FreeRTOS from STM32CubeMP1 `v1.6.0`
- Reference project:
  `STM32MP157C-DK2/Applications/OpenAMP/OpenAMP_FreeRTOS_echo`
- Build system: GNU Make invoked by BitBake

The reference project supplies FreeRTOS and its configuration, the STM32MP1
startup code, linker script, resource table, IPCC mailbox transport, OpenAMP
middleware, and HAL files. The repository `Makefile` compiles those pinned
sources together with the gateway sources; STM32CubeIDE is not required.

### ST Reference Material

- [PHYTEC phyBOARD-Sargas connector and pin-assignment manual](https://www.phytec.de/cdocuments/?doc=joCZOg)
- [OpenAMP FreeRTOS echo example at the pinned STM32CubeMP1 revision](https://github.com/STMicroelectronics/STM32CubeMP1/tree/b9a31179d5bf80b3958c3653153bfd4c3a7fc5d5/Projects/STM32MP157C-DK2/Applications/OpenAMP/OpenAMP_FreeRTOS_echo)
- [Reference example README](https://github.com/STMicroelectronics/STM32CubeMP1/blob/b9a31179d5bf80b3958c3653153bfd4c3a7fc5d5/Projects/STM32MP157C-DK2/Applications/OpenAMP/OpenAMP_FreeRTOS_echo/readme.txt)
- [Latest example on the STM32CubeMP1 master branch](https://github.com/STMicroelectronics/STM32CubeMP1/tree/master/Projects/STM32MP157C-DK2/Applications/OpenAMP/OpenAMP_FreeRTOS_echo)
- [ST Cortex-M coprocessor management overview](https://wiki.st.com/stm32mpu/wiki/Coprocessor_management_overview)
- [ST guide to exchanging data buffers with the coprocessor](https://wiki.st.com/stm32mpu/wiki/How_to_exchange_data_buffers_with_the_coprocessor)
- [ST guidance on OpenAMP with FreeRTOS on STM32MP1](https://community.st.com/t5/stm32-mpus-embedded-software-and/openamp-and-freertos-on-stm32mp1/td-p/209830)

The build uses the pinned revision, not the moving `master` branch. ST notes
thread-safety limitations in this STM32MP1 OpenAMP and FreeRTOS integration.
RPMsg receive processing and transmission therefore remain in the single
`power-gateway` task.

From an initialized PHYTEC BSP build environment:

```sh
bitbake m4-alarm-firmware
bitbake st-image-weston
```

The recipe fetches the pinned STM32CubeMP1 commit, builds with the BSP's
`gcc-arm-none-eabi-native` toolchain, and installs
`/lib/firmware/power-alarm-m4.elf`. Linux RemoteProc loads it into M4 memory
at boot; it is not permanently flashed into a separate MCU.

The same Makefile can be used outside BitBake when the pinned Cube tree and
GNU Arm Embedded toolchain are already available:

```sh
make CUBE_ROOT=/opt/src/STM32CubeMP1 \
     CROSS_COMPILE=/opt/gcc-arm-none-eabi/bin/arm-none-eabi-
```

This produces `build/power-alarm-m4.elf` and
`build/power-alarm-m4.bin`.

## Runtime Contract

- M4 endpoint address: `14`
- OpenAMP service: `rpmsg-raw`
- Linux endpoint hello byte: `0xA5`
- Event structure: packed, little-endian, 16 bytes
- Debounce: 10 ms
- Heartbeat: 5 seconds

GPIO and EXTI configuration uses the STM32Cube resource-lock service so Linux
and M4 coordinate access through HSEM 0 and HSEM 1.

## FreeRTOS Design

- The EXTI ISR timestamps the edge and uses `xQueueSendFromISR()` only.
- A 16-entry FreeRTOS queue decouples interrupt capture from debounce and IPC.
- One `power-gateway` task owns OpenAMP initialization, polling, receive
  callbacks, and all RPMsg sends.
- The task polls OpenAMP every 1 ms, validates edges after the 10 ms debounce
  interval, and emits the 5-second heartbeat.

Keeping all OpenAMP operations in one task follows the STM32MP1 reference
example's constraint: its bundled libmetal/OpenAMP configuration is not
generally thread-safe. FreeRTOS synchronization functions are not called from
an OpenAMP callback.
