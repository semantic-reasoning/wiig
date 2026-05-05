/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#ifndef WIIG_H
#define WIIG_H

/*
 * wiig.h is the umbrella header for the public wiig API.
 *
 * Downstream users include only this header:
 *
 *     #include <wiig/wiig.h>
 *
 * Sub-headers (wiig-export.h, wiig-types.h) are installed for compatibility
 * with packagers but should not be included directly by consumers.
 */

#include <wiig/wiig-export.h>
#include <wiig/wiig-types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================== */
/* Version                                                                  */
/* ======================================================================== */

/**
 * wiig_version_string:
 *
 * Returns the wiig library version as a NUL-terminated string in the form
 * "MAJOR.MINOR.PATCH". The returned pointer is owned by the library and
 * must not be freed.
 *
 * Returns: (transfer none): version string.
 */
WIIG_PUBLIC const char *
wiig_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* WIIG_H */
