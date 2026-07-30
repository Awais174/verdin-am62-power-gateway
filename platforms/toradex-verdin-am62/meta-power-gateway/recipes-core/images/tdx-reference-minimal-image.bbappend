IMAGE_INSTALL:append = " packagegroup-power-gateway"

do_rootfs[depends] += "${@'verdin-am62-power-alarm-overlay:do_deploy' if d.getVar('MACHINE') == 'verdin-am62' else ''}"

TEZI_EXTERNAL_KERNEL_DEVICETREE_BOOT:append:verdin-am62 = " \
    verdin-am62_hmp_overlay.dtbo \
    verdin-am62_power-alarm_overlay.dtbo \
    verdin-am62_panel-cap-touch-7inch-dsi_overlay.dtbo \
"
