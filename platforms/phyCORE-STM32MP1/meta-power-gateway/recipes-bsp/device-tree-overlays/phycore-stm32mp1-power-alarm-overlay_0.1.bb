SUMMARY = "phyBOARD-Sargas PG0 alarm input reservation for Cortex-M4"
LICENSE = "GPL-2.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0-only;md5=801f80980d171dd6425610833a22dbe6"

SRC_URI = "file://phyboard-sargas-power-alarm.dts"
S = "${WORKDIR}"

inherit devicetree

DT_FILES = "phyboard-sargas-power-alarm.dts"

do_install() {
    install -d ${D}/boot/overlays
    overlay=$(find ${B} -name phyboard-sargas-power-alarm.dtbo -print -quit)
    test -n "$overlay"
    install -m 0644 "$overlay" \
        ${D}/boot/overlays/phyboard-sargas-power-alarm.dtbo
}

FILES:${PN} = "/boot/overlays/phyboard-sargas-power-alarm.dtbo"
PACKAGE_ARCH = "${MACHINE_ARCH}"
COMPATIBLE_MACHINE = "^phycore-stm32mp1-3$"
