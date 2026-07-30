# meta-power-gateway

Minimal Yocto layer for the Verdin AM62 PM5560 gateway prototype.

## Add the layer

```sh
bitbake-layers add-layer ../layers/meta-power-gateway
bitbake tdx-reference-minimal-image
```

The image append adds `packagegroup-power-gateway`, which installs the polling
daemon, console HMI, M4 RemoteProc loader, GPIO ownership overlay, journald
limits, and nftables rules. For Verdin AM62 it also lists the HMP, alarm GPIO,
and 7-inch DSI overlays in `TEZI_EXTERNAL_KERNEL_DEVICETREE_BOOT`.

Before a hardware image is considered complete:

1. Replace the example Modbus register values in
   `/etc/power-gateway/power-gateway.conf` with addresses from the Schneider
   register map matching the meter firmware.
2. Build and validate the M4F firmware, then enable the disabled packaging
   recipe under `recipes-bsp/m4-alarm-firmware`.
3. Add that package to the packagegroup.
4. Confirm the three overlay names against the exact Toradex BSP release.
5. Verify that SODIMM 206 is not claimed by another carrier-board function.
6. Confirm `/dev/rpmsg_ctrl0` and `/dev/rpmsg0` numbering during bring-up.

`power-gateway` creates the `power-alarm` endpoint with
`RPMSG_CREATE_EPT_IOCTL`, targeting M4 endpoint 14, and sends the protocol
hello before reading events.
