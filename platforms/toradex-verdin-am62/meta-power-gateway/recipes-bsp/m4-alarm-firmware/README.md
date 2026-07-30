# M4F firmware packaging

`m4-alarm-firmware_0.2.bb.disabled` is a packaging template, not an active
recipe. Build the firmware under `m4-firmware` with AM62x MCU+ SDK
`12.00.00.27`, TI ARM Clang `4.0.1.LTS`, and SysConfig `1.26.2`.

After hardware validation:

1. Place the binary at
   `files/am62-mcu-m4f0_0-fw`.
2. Rename the recipe to `m4-alarm-firmware_0.2.bb`.
3. Add `m4-alarm-firmware` to `packagegroup-power-gateway`.
4. Record the firmware and MCU+ SDK versions in release metadata.

The active image remains buildable without a proprietary TI binary. On the
hardware image, `m4-firmware-loader.service` starts only when
`/lib/firmware/am62-mcu-m4f0_0-fw` exists.
