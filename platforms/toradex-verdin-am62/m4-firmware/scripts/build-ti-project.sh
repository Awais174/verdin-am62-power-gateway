#!/bin/sh

set -eu

: "${SDK_INSTALL_PATH:?Set SDK_INSTALL_PATH to MCU+ SDK 12.00.00.27}"

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
firmware_dir=$(CDPATH= cd -- "$script_dir/.." && pwd)
project="$SDK_INSTALL_PATH/examples/drivers/ipc/ipc_rpmsg_echo_linux/am62x-sk/m4fss0-0_freertos/ti-arm-clang"
generated="$project/generated/ti_drivers_config.h"

if [ ! -f "$project/makefile" ]; then
    echo "TI project makefile not found: $project/makefile" >&2
    exit 1
fi
if [ ! -f "$generated" ]; then
    echo "Run SysConfig generation first; missing $generated" >&2
    exit 1
fi
for symbol in CONFIG_GPIO0_BASE_ADDR CONFIG_GPIO0_PIN CONFIG_GPIO0_INTR; do
    if ! grep -q "$symbol" "$generated"; then
        echo "SysConfig output is missing $symbol" >&2
        exit 1
    fi
done

make -s -C "$project"

output=$(find "$project" -maxdepth 1 -type f -name '*.release.out' |
    head -n 1)
if [ -z "$output" ]; then
    echo "No release RemoteProc ELF was produced" >&2
    exit 1
fi

mkdir -p "$firmware_dir/build"
cp "$output" "$firmware_dir/build/am62-mcu-m4f0_0-fw"
echo "Firmware: $firmware_dir/build/am62-mcu-m4f0_0-fw"

