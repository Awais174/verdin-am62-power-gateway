# Cortex-M4 Firmware Recipe

`m4-alarm-firmware_0.1.bb` builds the FreeRTOS firmware from source during the
Yocto build. It:

1. fetches the STM32CubeMP1 `v1.6.0` source at the pinned ST commit;
2. depends on the BSP's `gcc-arm-none-eabi-native` toolchain;
3. calls `m4-firmware/Makefile` with the BitBake work and sysroot paths;
4. installs `power-alarm-m4.elf` under `/lib/firmware`.

The packagegroup includes this recipe for `phycore-stm32mp1-3`, so either
command builds the firmware without a separate staging step:

```sh
bitbake m4-alarm-firmware
bitbake st-image-weston
```

The RemoteProc loader starts only when the installed ELF is present.
