SUMMARY = "STM32MP157C FreeRTOS power alarm firmware"
LICENSE = "CLOSED & Apache-2.0 & MIT & BSD-3-Clause"
LIC_FILES_CHKSUM = "file://License.md;md5=532c0d9fc2820ec1304ab8e0f227acc7"

FILESEXTRAPATHS:prepend := "${THISDIR}/../../../m4-firmware:"

SRC_URI = " \
    git://github.com/STMicroelectronics/STM32CubeMP1.git;protocol=https;branch=master \
    file://Makefile;subdir=firmware \
    file://include/power_alarm_protocol.h;subdir=firmware/include \
    file://include/power_alarm_stm32.h;subdir=firmware/include \
    file://src/board_stm32.c;subdir=firmware/src \
    file://src/ipc_openamp.c;subdir=firmware/src \
    file://src/main.c;subdir=firmware/src \
"
SRCREV = "b9a31179d5bf80b3958c3653153bfd4c3a7fc5d5"

S = "${WORKDIR}/git"
B = "${WORKDIR}/build"

DEPENDS = "gcc-arm-none-eabi-native"

M4_CROSS_COMPILE = "${RECIPE_SYSROOT_NATIVE}${datadir}/gcc-arm-none-eabi/bin/arm-none-eabi-"

do_compile[cleandirs] = "${B}"
do_compile() {
    oe_runmake -f ${WORKDIR}/firmware/Makefile \
        CUBE_ROOT=${S} \
        PROJECT_ROOT=${WORKDIR}/firmware \
        BUILD_DIR=${B} \
        CROSS_COMPILE=${M4_CROSS_COMPILE}
}

do_install() {
    install -d ${D}${nonarch_base_libdir}/firmware
    install -m 0644 ${B}/power-alarm-m4.elf \
        ${D}${nonarch_base_libdir}/firmware/power-alarm-m4.elf
}

FILES:${PN} = "${nonarch_base_libdir}/firmware/power-alarm-m4.elf"
PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "^phycore-stm32mp1-3$"

INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_SYSROOT_STRIP = "1"
