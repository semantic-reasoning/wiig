/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#include <wiig/wiig.h>

#define WG_STRINGIFY_(x) #x
#define WG_STRINGIFY(x)  WG_STRINGIFY_(x)

#define WG_VERSION_STRING                       \
        WG_STRINGIFY(WIIG_VERSION_MAJOR) "."    \
        WG_STRINGIFY(WIIG_VERSION_MINOR) "."    \
        WG_STRINGIFY(WIIG_VERSION_PATCH)

const char *
wiig_version_string(void)
{
    return WG_VERSION_STRING;
}
