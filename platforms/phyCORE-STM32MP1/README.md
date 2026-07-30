# PHYTEC phyCORE-STM32MP1 Power Gateway

Power-instrumentation gateway prototype for a PHYTEC phyCORE-STM32MP1 module
with STM32MP157C on the phyBOARD-Sargas carrier.

- [Product requirements](docs/phycore-stm32mp1-power-gateway-prd.md)
- [Cortex-M4 firmware](m4-firmware/README.md)
- [Yocto layer](meta-power-gateway/README.md)

## Selected Platform

| Component | Selection |
|---|---|
| SOM | PHYTEC phyCORE-STM32MP1, STM32MP157C |
| Carrier | phyBOARD-Sargas PCB 1517.2 |
| Linux BSP | PHYTEC OpenSTLinux PD23.1.0, Yocto Kirkstone |
| Yocto machine | `phycore-stm32mp1-3` |
| Meter | Schneider PowerLogic PM5560 |
| Alarm input | X3 Arduino D7, `PG0/EXTI0`, 3.3 V |
| Real-time core | STM32MP157C Cortex-M4 at up to 209 MHz |
| M4 runtime | FreeRTOS from STM32CubeMP1 v1.6.0 |
| IPC | STM32 IPCC + OpenAMP/RPMsg |
| Display | Raspberry Pi official 7-inch DSI display |

Linux performs Modbus TCP polling, processing, journaling, and HMI updates.
FreeRTOS on the Cortex-M4 captures both edges of the isolated meter alarm
input and sends events to Linux without relying on the normal polling
interval.

## Status

The source, recipes, boot loader, overlay, and Makefile-based M4 build are
implemented. BitBake compiles and installs the M4 ELF as part of the PHYTEC
image. Execution on the selected module and carrier revision remains a
hardware-validation requirement.
