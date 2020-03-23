/*
 * Copyright 2020, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

#include <autoconf.h>

#include <assert.h>
#include <stdbool.h>
#include <sel4/sel4.h>
#include <camkes.h>
#include <camkes/io.h>
#include <platsupport/reset.h>
#include <utils/util.h>

#include "plat.h"

/*
 * Gotchas:
 *  - there is no access control at the moment, this means that clients can
 *    perform a denial-of-service attack on other clients by asserting the reset
 *    line on devices
 */

/* reset_sys for interacting with the reset lines */
static reset_sys_t reset_sys;
/* io_ops structure to access IO resources with */
static ps_io_ops_t io_ops;

int the_reset_assert_reset(reset_id_t id)
{
    int error = reset_server_lock();
    ZF_LOGF_IF(error, "Failed to lock the reset server");

    int ret = reset_sys_assert(&reset_sys, id);

    error = reset_server_unlock();
    ZF_LOGF_IF(error, "Failed to unlock the reset server");

    return ret;
}

int the_reset_deassert_reset(reset_id_t id)
{
    int error = reset_server_lock();
    ZF_LOGF_IF(error, "Failed to lock the reset server");

    int ret = reset_sys_deassert(&reset_sys, id);

    error = reset_server_unlock();
    ZF_LOGF_IF(error, "Failed to unlock the reset server");

    return ret;
}

void post_init()
{
    int error = reset_server_lock();
    ZF_LOGF_IF(error, "Failed to lock the reset server");

    error = camkes_io_ops(&io_ops);
    ZF_LOGF_IF(error, "Failed to get camkes_io_ops");

    /* Do platform-specific initialisation for the reset subsystem */
    if (plat_init) {
        error = plat_init(&io_ops);
        ZF_LOGF_IF(error, "Failed to perform the platform initialisation");
    }

    error = reset_sys_init(&io_ops, NULL, &reset_sys);
    ZF_LOGF_IF(error, "Failed to initialise the reset subsystem with the BPMP");

    error = reset_server_unlock();
    ZF_LOGF_IF(error, "Failed to unlock the reset server");
}
