SUMMARY = "Packages for the Verdin AM62 power instrumentation gateway"
LICENSE = "MIT"

inherit packagegroup

RDEPENDS:${PN} = " \
    power-gateway \
    power-gateway-config \
    power-hmi \
    nftables \
"

RDEPENDS:${PN}:append:verdin-am62 = " \
    m4-firmware-loader \
    verdin-am62-power-alarm-overlay \
"

