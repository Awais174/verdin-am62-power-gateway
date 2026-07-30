SUMMARY = "Reserve the Verdin AM62 MCU GPIO interrupt domain for M4F"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://verdin-am62_power-alarm_overlay.dts"
S = "${WORKDIR}"

DEPENDS = "dtc-native"

inherit deploy

COMPATIBLE_MACHINE = "^verdin-am62$"

do_compile() {
    dtc -@ -I dts -O dtb \
        -o verdin-am62_power-alarm_overlay.dtbo \
        ${WORKDIR}/verdin-am62_power-alarm_overlay.dts
}

do_install() {
    install -d ${D}/boot/overlays
    install -m 0644 verdin-am62_power-alarm_overlay.dtbo \
        ${D}/boot/overlays/
}

do_deploy() {
    install -d ${DEPLOYDIR}
    install -m 0644 verdin-am62_power-alarm_overlay.dtbo ${DEPLOYDIR}/
}

addtask deploy after do_compile before do_build

FILES:${PN} = "/boot/overlays/verdin-am62_power-alarm_overlay.dtbo"

