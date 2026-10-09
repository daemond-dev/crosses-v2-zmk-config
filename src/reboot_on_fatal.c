/*
 * Reboot on a fatal error instead of halting until a manual reset. This Zephyr has no
 * CONFIG_RESET_ON_FATAL_ERROR; overrides the weak handler in kernel/fatal.c.
 */

#include <zephyr/kernel.h>
#include <zephyr/fatal.h>
#include <zephyr/sys/reboot.h>

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf) {
    ARG_UNUSED(reason);
    ARG_UNUSED(esf);

    sys_reboot(SYS_REBOOT_WARM);
}
