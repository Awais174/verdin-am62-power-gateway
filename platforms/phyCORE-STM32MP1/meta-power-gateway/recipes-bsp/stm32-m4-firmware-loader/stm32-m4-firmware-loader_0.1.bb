SUMMARY = "RemoteProc boot loader for STM32MP1 power alarm firmware"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://stm32-m4-firmware-loader \
           file://stm32-m4-firmware-loader.service"

S = "${WORKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "stm32-m4-firmware-loader.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/stm32-m4-firmware-loader \
        ${D}${sbindir}/stm32-m4-firmware-loader

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/stm32-m4-firmware-loader.service \
        ${D}${systemd_system_unitdir}/stm32-m4-firmware-loader.service
}

COMPATIBLE_MACHINE = "^phycore-stm32mp1-3$"
