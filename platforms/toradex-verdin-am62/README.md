# Verdin AM62 Power Gateway

Prototype requirements and a minimal Yocto integration layer for a
Schneider PowerLogic PM5560 gateway.

- [Product requirements](docs/verdin-am62-power-instrumentation-gateway-prd.md)
- [`meta-power-gateway`](meta-power-gateway/README.md)
- [M4F firmware project](m4-firmware/README.md)

The design uses Linux for Modbus TCP polling, journaling, and the DSI LCD HMI.
The AM62 Cortex-M4F captures an isolated meter alarm GPIO and relays events to
Linux using RPMessage/RPMsg.
