# Power Gateway

Reference implementations for an embedded power-instrumentation gateway
across the Toradex Verdin AM62 and PHYTEC phyCORE-STM32MP1 platforms. Each
platform keeps its BSP integration, coprocessor firmware, hardware allocation,
and product documentation in a self-contained directory.

## Platforms

| Platform | Processor | Real-time core | Status |
|---|---|---|---|
| [Toradex Verdin AM62](platforms/toradex-verdin-am62/README.md) | TI AM62 | Cortex-M4F | Prototype implementation |
| [PHYTEC phyCORE-STM32MP1](platforms/phyCORE-STM32MP1/README.md) | STM32MP157C | Cortex-M4 | Prototype implementation, hardware validation pending |

The Verdin AM62 directory contains the current PRD, Yocto layer, GPIO
allocation, Linux gateway services, and M4F firmware integration.

The phyCORE-STM32MP1 directory targets the PHYTEC module on a phyBOARD-Sargas
carrier. It includes a pinned OpenSTLinux BSP integration, Arduino D7/PG0
alarm-input allocation, STM32CubeMP1 Cortex-M4 firmware,
OpenAMP/RemoteProc boot path, FreeRTOS runtime, Yocto layer, HMI, and PRD.

## Common Product Scope

The gateway polls Ethernet-connected power instrumentation over Modbus TCP,
processes and records measurements through `systemd-journald`, handles
time-sensitive alarm inputs through a real-time coprocessor, and presents
essential status and alarms on a local display.

Platform implementations may use different BSPs, coprocessor toolchains,
interprocessor communication mechanisms, pin assignments, and display stacks.
Hardware-specific assumptions must remain inside the corresponding platform
directory.
