IMAGE_INSTALL:append:phycore-stm32mp1-3 = " packagegroup-power-gateway"

ROOTFS_POSTPROCESS_COMMAND:append:phycore-stm32mp1-3 = " enable_power_gateway_overlay; "

enable_power_gateway_overlay() {
    overlays="${IMAGE_ROOTFS}/boot/overlays/overlays.txt"
    required="phyboard-sargas-power-alarm phyboard-stm32mp1-dsi-rpi-official-display"
    install -d "$(dirname "$overlays")"

    if [ ! -f "$overlays" ]; then
        echo "overlay=$required" > "$overlays"
    elif grep -q '^overlay=' "$overlays"; then
        sed -i "/^overlay=/ s/$/ $required/" "$overlays"
    else
        echo "overlay=$required" >> "$overlays"
    fi
}
