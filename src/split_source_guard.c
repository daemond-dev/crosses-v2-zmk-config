/*
 * zmkfirmware/zmk#3522: on a host disconnect the BLE split central queues a battery event with
 * source (uint8_t)-EINVAL, and the handler writes peripheral_battery_levels[234]. Drop events
 * whose source is out of range before they reach it. Remove once fixed upstream.
 */

#include <errno.h>

#include <zmk/split/central.h>
#include <zmk/split/transport/central.h>

/* Fails the build if ZMK changes the handler's signature. */
__typeof__(zmk_split_transport_central_peripheral_event_handler)
    __real_zmk_split_transport_central_peripheral_event_handler,
    __wrap_zmk_split_transport_central_peripheral_event_handler;

int __wrap_zmk_split_transport_central_peripheral_event_handler(
    const struct zmk_split_transport_central *transport, uint8_t source,
    struct zmk_split_transport_peripheral_event ev) {
    if (source >= ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT) {
        return -EINVAL;
    }

    return __real_zmk_split_transport_central_peripheral_event_handler(transport, source, ev);
}
