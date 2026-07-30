SUMMARY = "PM5560 Modbus polling and M4 alarm event daemon"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://power-gateway.c \
    file://power-gateway.conf \
    file://power-gateway.service \
"

S = "${WORKDIR}"

DEPENDS = "libmodbus systemd"

inherit pkgconfig systemd

SYSTEMD_SERVICE:${PN} = "power-gateway.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_compile() {
    ${CC} ${CFLAGS} ${CPPFLAGS} power-gateway.c -o power-gateway \
        `${PKG_CONFIG} --cflags --libs libmodbus libsystemd` \
        -pthread ${LDFLAGS}
}

do_install() {
    install -d ${D}${sbindir}
    install -m 0755 power-gateway ${D}${sbindir}/power-gateway

    install -d ${D}${sysconfdir}/power-gateway
    install -m 0644 ${WORKDIR}/power-gateway.conf \
        ${D}${sysconfdir}/power-gateway/power-gateway.conf

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/power-gateway.service \
        ${D}${systemd_system_unitdir}/power-gateway.service
}

