SUMMARY = "Framebuffer-console status HMI for the power gateway"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://power-hmi file://power-hmi.service"
S = "${WORKDIR}"

inherit systemd

SYSTEMD_SERVICE:${PN} = "power-hmi.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 ${WORKDIR}/power-hmi ${D}${sbindir}/power-hmi
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/power-hmi.service \
        ${D}${systemd_system_unitdir}/power-hmi.service
}

RDEPENDS:${PN} = "bash coreutils"

