SUMMARY = "Boot the Verdin AM62 M4F firmware with Linux RemoteProc"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://m4-firmware-loader file://m4-firmware-loader.service"
S = "${WORKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "m4-firmware-loader.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/m4-firmware-loader \
        ${D}${sbindir}/m4-firmware-loader

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/m4-firmware-loader.service \
        ${D}${systemd_system_unitdir}/m4-firmware-loader.service
}

RDEPENDS:${PN} = "busybox"

