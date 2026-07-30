SUMMARY = "Packages for the phyCORE-STM32MP1 power instrumentation gateway"
LICENSE = "MIT"

inherit packagegroup

RDEPENDS:${PN} = " \
    power-gateway \
    power-gateway-config \
    power-hmi \
    nftables \
"

RDEPENDS:${PN}:append:phycore-stm32mp1-3 = " \
    m4-alarm-firmware \
    stm32-m4-firmware-loader \
    phycore-stm32mp1-power-alarm-overlay \
    phytec-dt-overlays-stm32mp \
"
