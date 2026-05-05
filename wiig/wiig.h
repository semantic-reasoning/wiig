/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#ifndef WIIG_H
#define WIIG_H

/*
 * wiig.h — single public umbrella header for libwiig.
 *
 * Consumers include only this header:
 *
 *     #include <wiig/wiig.h>
 *
 * No other wiig header is part of the public surface, and consumers
 * MUST NOT include any other wiig file directly. Internal sub-headers
 * (visibility macros, internal types, lexer / CST / parser / format /
 * highlight) are not installed.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================================== */
/* Symbol visibility                                                        */
/* ======================================================================== */

/*
 * WIIG_PUBLIC annotates every exported declaration. The library target is
 * built with -DWIIG_BUILDING and gnu_symbol_visibility=hidden; consumers
 * compile without WIIG_BUILDING so dllimport / default-visibility resolve
 * correctly.
 */
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef WIIG_BUILDING
    #define WIIG_PUBLIC __declspec(dllexport)
  #else
    #define WIIG_PUBLIC __declspec(dllimport)
  #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
  #define WIIG_PUBLIC __attribute__((visibility("default")))
#else
  #define WIIG_PUBLIC
#endif

/* ======================================================================== */
/* Version                                                                  */
/* ======================================================================== */

#define WIIG_VERSION_MAJOR 0
#define WIIG_VERSION_MINOR 1
#define WIIG_VERSION_PATCH 0

#define WIIG_VERSION                                  \
        (WIIG_VERSION_MAJOR * 10000                   \
         + WIIG_VERSION_MINOR * 100                   \
         + WIIG_VERSION_PATCH)

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
