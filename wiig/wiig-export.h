/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#ifndef WIIG_EXPORT_H
#define WIIG_EXPORT_H

/*
 * WIIG_PUBLIC annotates every public-API declaration. Public-API consumers
 * include <wiig/wiig.h> which transitively pulls in this header.
 *
 * The library target is built with -DWIIG_BUILDING (set in wiig/meson.build);
 * consumers compile without that define, so the dllexport / dllimport branches
 * resolve correctly on Windows. On ELF, the visibility attribute is honoured
 * because the library target also sets gnu_symbol_visibility=hidden.
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

#endif /* WIIG_EXPORT_H */
