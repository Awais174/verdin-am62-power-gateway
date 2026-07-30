# meta-power-gateway for phyCORE-STM32MP1

Minimal Yocto layer for the STM32MP157C power gateway on phyBOARD-Sargas.

## Hardware Image

Initialize PHYTEC BSP `BSP-Yocto-OpenSTLinux-STM32MP1-PD23.1.0`, then:

```sh
source openstlinux-init-phytec.sh
bitbake-layers add-layer /path/to/phyCORE-STM32MP1/meta-power-gateway
MACHINE=phycore-stm32mp1-3 bitbake st-image-weston
```

The image append installs:

- Modbus polling and alarm-event daemon;
- framebuffer-console HMI;
- journald retention and nftables configuration;
- source-built FreeRTOS M4 firmware;
- STM32 M4 RemoteProc boot service;
- PG0/EXTI0 M4 resource overlay for Arduino D7.

The `st-image-weston` append is conditional on the `stm-st-stm32mp` layer
collection supplied by the PHYTEC OpenSTLinux BSP.

The overlay is installed as
`/boot/overlays/phyboard-sargas-power-alarm.dtbo` and added to PHYTEC's
`overlays.txt` together with
`phyboard-stm32mp1-dsi-rpi-official-display`. Confirm that a saved U-Boot
`overlay` environment variable is not overriding `overlays.txt`.

The `m4-alarm-firmware` recipe fetches the STM32CubeMP1 `v1.6.0` source,
builds the gateway with `gcc-arm-none-eabi-native` and the repository
Makefile, and installs the ELF consumed by the RemoteProc loader.
