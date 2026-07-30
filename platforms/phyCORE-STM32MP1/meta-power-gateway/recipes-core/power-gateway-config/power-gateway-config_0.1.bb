SUMMARY = "System configuration for the power gateway"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://journald-power-gateway.conf file://power-gateway.nft"
S = "${WORKDIR}"

do_install() {
    install -d ${D}${sysconfdir}/systemd/journald.conf.d
    install -m 0644 ${WORKDIR}/journald-power-gateway.conf \
        ${D}${sysconfdir}/systemd/journald.conf.d/power-gateway.conf

    install -d ${D}${sysconfdir}/nftables
    install -m 0644 ${WORKDIR}/power-gateway.nft \
        ${D}${sysconfdir}/nftables/power-gateway.nft
}

FILES:${PN} += "${sysconfdir}/systemd/journald.conf.d \
                ${sysconfdir}/nftables"
