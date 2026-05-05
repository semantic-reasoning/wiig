/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

/*
 * token_kind.h - wiig lossless lexer token catalogue.
 *
 * NOT installed. Internal use only.
 *
 * The syntactic kinds (WG_TOKEN_IDENT through WG_TOKEN_QUERY plus EOF
 * and ERROR) mirror wirelog's wl_parser_lexer_token_type_t one-for-one
 * with a WG_TOKEN_* rename. Trivia kinds (WS, NEWLINE, the two line
 * comments, BLOCK_COMMENT) are wiig-specific: wirelog discards trivia
 * because its parser builds an AST; wiig is a formatter and must
 * preserve every byte.
 *
 * Lossless invariant: for any source S of length N,
 *   sum(t.length for t in tokens(S) if t.kind != WG_TOKEN_EOF) == N
 *   concat(t.start[0..t.length] for ditto) == S
 *
 * Position model: line and col are 1-indexed; col counts bytes, not
 * codepoints, matching GCC/Clang diagnostic convention.
 *
 * Block comments: wiig accepts the C-style slash-star form as
 * BLOCK_COMMENT trivia. This is a deliberate extension over wirelog's
 * lexer, which treats slash-star as two operator tokens. Wirelog ships
 * at least one example file (examples/08-delta-queries/access_control.dl)
 * that opens with a block comment; a formatter for the wirelog corpus
 * must preserve those bytes intact rather than format them as operator
 * soup.
 */

#ifndef WG_LEXER_TOKEN_KIND_H
#define WG_LEXER_TOKEN_KIND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    /* Trivia (wiig-specific; wirelog discards these) ---------------------- */
    WG_TOKEN_WS,                    /* run of [ \t\r] (does not cross \n) */
    WG_TOKEN_NEWLINE,               /* exactly one \n */
    WG_TOKEN_LINE_COMMENT_HASH,     /* '#' to but not including next \n */
    WG_TOKEN_LINE_COMMENT_SLASH,    /* '//' to but not including next \n */
    WG_TOKEN_BLOCK_COMMENT,         /* slash-star ... star-slash (extension) */

    /* Literals ------------------------------------------------------------ */
    WG_TOKEN_IDENT,                 /* _?[a-zA-Z][a-zA-Z0-9_]* */
    WG_TOKEN_INTEGER,               /* [0-9]+ */
    WG_TOKEN_STRING,                /* "..." (raw; escapes not decoded) */

    /* Boolean literals ---------------------------------------------------- */
    WG_TOKEN_TRUE,                  /* True */
    WG_TOKEN_FALSE,                 /* False */

    /* Placeholder --------------------------------------------------------- */
    WG_TOKEN_UNDERSCORE,            /* _, __, ___ (no alpha continuation) */

    /* Aggregate keywords (case-insensitive pairs) ------------------------- */
    WG_TOKEN_COUNT,                 /* count / COUNT */
    WG_TOKEN_SUM,                   /* sum / SUM */
    WG_TOKEN_MIN,                   /* min / MIN */
    WG_TOKEN_MAX,                   /* max / MAX */
    WG_TOKEN_AVG,                   /* average / AVG */

    /* Type keywords ------------------------------------------------------- */
    WG_TOKEN_INT32,                 /* int32 */
    WG_TOKEN_INT64,                 /* int64 */
    WG_TOKEN_STRING_TYPE,           /* string (as a type name) */
    WG_TOKEN_SYMBOL_TYPE,           /* symbol */

    /* Punctuation --------------------------------------------------------- */
    WG_TOKEN_LPAREN,                /* ( */
    WG_TOKEN_RPAREN,                /* ) */
    WG_TOKEN_COMMA,                 /* , */
    WG_TOKEN_DOT,                   /* . */
    WG_TOKEN_COLON,                 /* : */
    WG_TOKEN_BANG,                  /* ! */

    /* Operators ----------------------------------------------------------- */
    WG_TOKEN_HORN,                  /* :- */
    WG_TOKEN_EQ,                    /* = */
    WG_TOKEN_NEQ,                   /* != */
    WG_TOKEN_LT,                    /* < */
    WG_TOKEN_GT,                    /* > */
    WG_TOKEN_LTE,                   /* <= */
    WG_TOKEN_GTE,                   /* >= */
    WG_TOKEN_PLUS,                  /* + */
    WG_TOKEN_MINUS,                 /* - */
    WG_TOKEN_STAR,                  /* * */
    WG_TOKEN_SLASH,                 /* / */
    WG_TOKEN_PERCENT,               /* % */

    /* Bitwise operator keywords ------------------------------------------- */
    WG_TOKEN_BAND,                  /* band */
    WG_TOKEN_BOR,                   /* bor */
    WG_TOKEN_BXOR,                  /* bxor */
    WG_TOKEN_BNOT,                  /* bnot */
    WG_TOKEN_BSHL,                  /* bshl */
    WG_TOKEN_BSHR,                  /* bshr */

    /* Generic hash keyword ------------------------------------------------ */
    WG_TOKEN_HASH,                  /* hash */

    /* Cryptographic hash keywords ----------------------------------------- */
    WG_TOKEN_MD5,                   /* md5 */
    WG_TOKEN_SHA1,                  /* sha1 */
    WG_TOKEN_SHA256,                /* sha256 */
    WG_TOKEN_SHA512,                /* sha512 */
    WG_TOKEN_HMAC_SHA256,           /* hmac_sha256 */

    /* UUID keywords ------------------------------------------------------- */
    WG_TOKEN_UUID4,                 /* uuid4 */
    WG_TOKEN_UUID5,                 /* uuid5 */

    /* String function keywords -------------------------------------------- */
    WG_TOKEN_STRLEN,                /* strlen */
    WG_TOKEN_CAT,                   /* cat */
    WG_TOKEN_SUBSTR,                /* substr */
    WG_TOKEN_CONTAINS,              /* contains */
    WG_TOKEN_STR_PREFIX,            /* str_prefix */
    WG_TOKEN_STR_SUFFIX,            /* str_suffix */
    WG_TOKEN_STR_ORD,               /* str_ord */
    WG_TOKEN_TO_UPPER,              /* to_upper */
    WG_TOKEN_TO_LOWER,              /* to_lower */
    WG_TOKEN_STR_REPLACE,           /* str_replace */
    WG_TOKEN_TRIM,                  /* trim */
    WG_TOKEN_TO_STRING,             /* to_string */
    WG_TOKEN_TO_NUMBER,             /* to_number */

    /* Directives (dot-prefixed) ------------------------------------------- */
    WG_TOKEN_DECL,                  /* .decl */
    WG_TOKEN_INPUT,                 /* .input */
    WG_TOKEN_OUTPUT,                /* .output */
    WG_TOKEN_PRINTSIZE,             /* .printsize */
    WG_TOKEN_PLAN,                  /* .plan */
    WG_TOKEN_QUERY,                 /* .query */

    /* Special ------------------------------------------------------------- */
    WG_TOKEN_EOF,                   /* end of input; length 0 */
    WG_TOKEN_ERROR,                 /* lexer error; length spans bad byte(s) */
} wg_lexer_token_kind_t;

typedef struct {
    wg_lexer_token_kind_t kind;
    const char           *start;        /* pointer into caller-owned buffer */
    uint32_t              length;       /* token length in bytes */
    uint32_t              byte_offset;  /* offset from start of source buffer */
    uint32_t              line;         /* 1-indexed */
    uint32_t              col;          /* 1-indexed; byte-counted */
} wg_lexer_token_t;

#ifdef __cplusplus
}
#endif

#endif /* WG_LEXER_TOKEN_KIND_H */
