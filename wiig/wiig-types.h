/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#ifndef WIIG_TYPES_H
#define WIIG_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ======================================================================== */
/* Version Information                                                      */
/* ======================================================================== */

#define WIIG_VERSION_MAJOR 0
#define WIIG_VERSION_MINOR 1
#define WIIG_VERSION_PATCH 0

#define WIIG_VERSION                                  \
        (WIIG_VERSION_MAJOR * 10000                   \
         + WIIG_VERSION_MINOR * 100                   \
         + WIIG_VERSION_PATCH)

/*
 * Future commits add opaque handles (wiig_doc_t, wiig_formatter_t,
 * wiig_highlighter_t) and enumerations as the lexer / CST / format /
 * highlight subsystems land. This header is intentionally minimal at the
 * skeleton commit.
 */

#ifdef __cplusplus
}
#endif

#endif /* WIIG_TYPES_H */
