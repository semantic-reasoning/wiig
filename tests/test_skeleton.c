/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#include <string.h>

#include <wiig/wiig.h>

/*
 * Explicit branch + non-zero exit (mirroring cli/main.c) so the test
 * stays meaningful under -DNDEBUG; b_ndebug=if-release would otherwise
 * strip assert() in release builds and turn this into a vacuous pass.
 */
int
main(void)
{
    const char *v = wiig_version_string();
    if (v == NULL || v[0] == '\0') {
        return 1;
    }
    return 0;
}
