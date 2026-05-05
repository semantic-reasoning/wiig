/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#include <assert.h>
#include <string.h>

#include <wiig/wiig.h>

int
main(void)
{
    const char *v = wiig_version_string();
    assert(v != NULL);
    assert(strlen(v) > 0);
    return 0;
}
