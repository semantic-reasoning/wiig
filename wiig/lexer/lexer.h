/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

/*
 * lexer.h - wiig internal lossless lexer interface.
 *
 * NOT installed. Internal use only. No symbol exported from libwiig.
 *
 * Lifecycle:
 *   wg_lexer_t lex;
 *   wg_lexer_init(&lex, source, len);
 *   for (;;) {
 *       wg_lexer_token_t tok = wg_lexer_next(&lex);
 *       if (tok.kind == WG_TOKEN_EOF) break;
 *       ...
 *   }
 *
 * Buffer ownership: the lexer holds a non-owning pointer to `source`.
 * The buffer must outlive every token (tokens point into it). The
 * buffer does NOT need to be NUL-terminated; the lexer respects `len`.
 *
 * Threading: a single wg_lexer_t is not thread-safe. Independent
 * instances on disjoint buffers may run concurrently; the lexer uses
 * no globals or TLS.
 *
 * Error recovery: on an unexpected byte the lexer emits WG_TOKEN_ERROR
 * with length covering the offending byte(s) and resumes at the next
 * byte. The lossless concat invariant still holds across ERROR tokens.
 */

#ifndef WG_LEXER_LEXER_H
#define WG_LEXER_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "wiig/lexer/token_kind.h"

typedef struct {
    const char *source;       /* buffer start (immutable for the lifetime) */
    const char *end;          /* one past last byte */
    const char *current;      /* read cursor */
    const char *start;        /* start of in-progress token */
    uint32_t    line;
    uint32_t    col;
    uint32_t    start_line;
    uint32_t    start_col;
} wg_lexer_t;

/**
 * wg_lexer_init: bind the lexer to a caller-owned source buffer.
 * `source` may contain any bytes (including embedded NULs); the lexer
 * treats embedded NULs as ERROR bytes, not as end-of-input.
 */
void
wg_lexer_init(wg_lexer_t *lexer, const char *source, size_t len);

/**
 * wg_lexer_next: scan and return the next token. After the last byte
 * of source, every call returns WG_TOKEN_EOF (with length 0 and start
 * pointing one past the buffer). Trivia (whitespace, newlines,
 * comments) is interleaved with syntactic tokens in source order.
 */
wg_lexer_token_t
wg_lexer_next(wg_lexer_t *lexer);

/**
 * wg_lexer_peek: return the token wg_lexer_next would return without
 * advancing. Calling peek twice in a row returns the same token.
 */
wg_lexer_token_t
wg_lexer_peek(wg_lexer_t *lexer);

/**
 * wg_lexer_token_kind_str: human-readable name for a token kind.
 * The returned pointer is static; do not free.
 */
const char *
wg_lexer_token_kind_str(wg_lexer_token_kind_t kind);

/**
 * wg_lexer_is_trivia: true iff `kind` is a trivia kind (whitespace,
 * newline, or any comment form). Useful for parsers that want to skip
 * over trivia without enumerating the kinds explicitly.
 */
bool
wg_lexer_is_trivia(wg_lexer_token_kind_t kind);

#ifdef __cplusplus
}
#endif

#endif /* WG_LEXER_LEXER_H */
