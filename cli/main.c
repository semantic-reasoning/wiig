/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#include <stdio.h>

#include <wiig/wiig.h>

int
main(void)
{
    /*
     * Subcommand parsing (fmt / check / hl) lands in subsequent commits.
     * At the skeleton commit the CLI prints the library version so the
     * call site has an observable side effect; this prevents LTO /
     * --gc-sections from dead-stripping the libwiig dependency and
     * silently turning the smoke test into a vacuous pass.
     */
    if (puts(wiig_version_string()) < 0) {
        return 1;
    }
    return 0;
}
