/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

#include "wiig/lexer/lexer.h"

#include <string.h>

/* ======================================================================== */
/* Cursor primitives                                                        */
/* ======================================================================== */

static inline bool
at_end(const wg_lexer_t *lex)
{
    return lex->current >= lex->end;
}

static inline char
peek(const wg_lexer_t *lex)
{
    return at_end(lex) ? '\0' : *lex->current;
}

static inline char
peek_at(const wg_lexer_t *lex, size_t offset)
{
    const char *p = lex->current + offset;
    return p >= lex->end ? '\0' : *p;
}

static inline void
advance(wg_lexer_t *lex)
{
    if (at_end(lex)) {
        return;
    }
    char c = *lex->current++;
    if (c == '\n') {
        lex->line++;
        lex->col = 1;
    } else {
        lex->col++;
    }
}

static inline bool
match(wg_lexer_t *lex, char c)
{
    if (at_end(lex) || *lex->current != c) {
        return false;
    }
    advance(lex);
    return true;
}

static inline bool
is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool
is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static inline bool
is_alnum_underscore(char c)
{
    return is_alpha(c) || is_digit(c) || c == '_';
}

/* ======================================================================== */
/* Token construction                                                       */
/* ======================================================================== */

static wg_lexer_token_t
make_token(const wg_lexer_t *lex, wg_lexer_token_kind_t kind)
{
    wg_lexer_token_t tok;
    tok.kind        = kind;
    tok.start       = lex->start;
    tok.length      = (uint32_t)(lex->current - lex->start);
    tok.byte_offset = (uint32_t)(lex->start - lex->source);
    tok.line        = lex->start_line;
    tok.col         = lex->start_col;
    return tok;
}

/* ======================================================================== */
/* Trivia scanners                                                          */
/* ======================================================================== */

/* Run of [ \t\r] (does not cross \n; \r is part of WS, not NEWLINE). */
static wg_lexer_token_t
scan_ws(wg_lexer_t *lex)
{
    while (!at_end(lex)) {
        char c = peek(lex);
        if (c == ' ' || c == '\t' || c == '\r') {
            advance(lex);
        } else {
            break;
        }
    }
    return make_token(lex, WG_TOKEN_WS);
}

/* Exactly one '\n'. The '\n' was already consumed by the caller. */
static wg_lexer_token_t
scan_newline(wg_lexer_t *lex)
{
    return make_token(lex, WG_TOKEN_NEWLINE);
}

/* '#' to but not including the next '\n'. The '#' was already consumed. */
static wg_lexer_token_t
scan_line_comment_hash(wg_lexer_t *lex)
{
    while (!at_end(lex) && peek(lex) != '\n') {
        advance(lex);
    }
    return make_token(lex, WG_TOKEN_LINE_COMMENT_HASH);
}

/* '//' to but not including the next '\n'. Both slashes already consumed. */
static wg_lexer_token_t
scan_line_comment_slash(wg_lexer_t *lex)
{
    while (!at_end(lex) && peek(lex) != '\n') {
        advance(lex);
    }
    return make_token(lex, WG_TOKEN_LINE_COMMENT_SLASH);
}

/* Slash-star ... star-slash. The opening slash-star was already consumed. */
static wg_lexer_token_t
scan_block_comment(wg_lexer_t *lex)
{
    while (!at_end(lex)) {
        if (peek(lex) == '*' && peek_at(lex, 1) == '/') {
            advance(lex); /* * */
            advance(lex); /* / */
            return make_token(lex, WG_TOKEN_BLOCK_COMMENT);
        }
        advance(lex);
    }
    /* Unterminated block comment: emit BLOCK_COMMENT spanning to EOF.
     * The bytes are still preserved losslessly; the parser may flag it. */
    return make_token(lex, WG_TOKEN_BLOCK_COMMENT);
}

/* ======================================================================== */
/* String / integer scanners                                                */
/* ======================================================================== */

/* "..." byte-faithful, no escape decoding. Mirrors wirelog. */
static wg_lexer_token_t
scan_string(wg_lexer_t *lex)
{
    /* Opening quote already consumed. */
    while (!at_end(lex) && peek(lex) != '"') {
        if (peek(lex) == '\n') {
            /* Unterminated on this line: emit ERROR for what we have. */
            return make_token(lex, WG_TOKEN_ERROR);
        }
        advance(lex);
    }
    if (at_end(lex)) {
        return make_token(lex, WG_TOKEN_ERROR);
    }
    advance(lex); /* closing quote */
    return make_token(lex, WG_TOKEN_STRING);
}

static wg_lexer_token_t
scan_integer(wg_lexer_t *lex)
{
    while (!at_end(lex) && is_digit(peek(lex))) {
        advance(lex);
    }
    return make_token(lex, WG_TOKEN_INTEGER);
}

/* ======================================================================== */
/* Keyword classification                                                   */
/* ======================================================================== */

typedef struct {
    const char           *text;
    uint32_t              length;
    wg_lexer_token_kind_t kind;
} wg_keyword_t;

/* Keep this table sorted by first character then length to keep lookup
 * predictable; correctness, not perf, is the goal at this commit. */
static const wg_keyword_t WG_KEYWORDS[] = {
    /* aggregates - lowercase */
    { "average",      7,  WG_TOKEN_AVG },
    { "count",        5,  WG_TOKEN_COUNT },
    { "max",          3,  WG_TOKEN_MAX },
    { "min",          3,  WG_TOKEN_MIN },
    { "sum",          3,  WG_TOKEN_SUM },
    /* aggregates - uppercase */
    { "AVG",          3,  WG_TOKEN_AVG },
    { "COUNT",        5,  WG_TOKEN_COUNT },
    { "MAX",          3,  WG_TOKEN_MAX },
    { "MIN",          3,  WG_TOKEN_MIN },
    { "SUM",          3,  WG_TOKEN_SUM },
    /* booleans */
    { "True",         4,  WG_TOKEN_TRUE },
    { "False",        5,  WG_TOKEN_FALSE },
    /* types */
    { "int32",        5,  WG_TOKEN_INT32 },
    { "int64",        5,  WG_TOKEN_INT64 },
    { "string",       6,  WG_TOKEN_STRING_TYPE },
    { "symbol",       6,  WG_TOKEN_SYMBOL_TYPE },
    /* bitwise */
    { "band",         4,  WG_TOKEN_BAND },
    { "bor",          3,  WG_TOKEN_BOR },
    { "bxor",         4,  WG_TOKEN_BXOR },
    { "bnot",         4,  WG_TOKEN_BNOT },
    { "bshl",         4,  WG_TOKEN_BSHL },
    { "bshr",         4,  WG_TOKEN_BSHR },
    /* hashes */
    { "hash",         4,  WG_TOKEN_HASH },
    { "md5",          3,  WG_TOKEN_MD5 },
    { "sha1",         4,  WG_TOKEN_SHA1 },
    { "sha256",       6,  WG_TOKEN_SHA256 },
    { "sha512",       6,  WG_TOKEN_SHA512 },
    { "hmac_sha256", 11, WG_TOKEN_HMAC_SHA256 },
    /* uuid */
    { "uuid4",        5,  WG_TOKEN_UUID4 },
    { "uuid5",        5,  WG_TOKEN_UUID5 },
    /* string fns */
    { "strlen",       6,  WG_TOKEN_STRLEN },
    { "cat",          3,  WG_TOKEN_CAT },
    { "substr",       6,  WG_TOKEN_SUBSTR },
    { "contains",     8,  WG_TOKEN_CONTAINS },
    { "str_prefix",  10, WG_TOKEN_STR_PREFIX },
    { "str_suffix",  10, WG_TOKEN_STR_SUFFIX },
    { "str_ord",      7,  WG_TOKEN_STR_ORD },
    { "to_upper",     8,  WG_TOKEN_TO_UPPER },
    { "to_lower",     8,  WG_TOKEN_TO_LOWER },
    { "str_replace", 11, WG_TOKEN_STR_REPLACE },
    { "trim",         4,  WG_TOKEN_TRIM },
    { "to_string",    9,  WG_TOKEN_TO_STRING },
    { "to_number",    9,  WG_TOKEN_TO_NUMBER },
};

static wg_lexer_token_kind_t
classify_identifier(const char *text, uint32_t length)
{
    for (size_t i = 0; i < sizeof(WG_KEYWORDS) / sizeof(WG_KEYWORDS[0]); i++) {
        const wg_keyword_t *kw = &WG_KEYWORDS[i];
        if (kw->length == length && memcmp(kw->text, text, length) == 0) {
            return kw->kind;
        }
    }
    return WG_TOKEN_IDENT;
}

/* ======================================================================== */
/* Identifier / underscore                                                  */
/* ======================================================================== */

/* On entry, lex->start points at an alpha or underscore. Wirelog's
 * identifier rule is `_?[a-zA-Z][a-zA-Z0-9_]*`. The first byte (alpha
 * or single leading underscore) was already consumed by the caller;
 * this scans the rest. */
static wg_lexer_token_t
scan_identifier(wg_lexer_t *lex)
{
    while (!at_end(lex) && is_alnum_underscore(peek(lex))) {
        advance(lex);
    }
    uint32_t            length = (uint32_t)(lex->current - lex->start);
    wg_lexer_token_kind_t kind = classify_identifier(lex->start, length);
    return make_token(lex, kind);
}

/* On entry, the cursor is at the first character (which must be '_').
 * Decide between WG_TOKEN_UNDERSCORE (one or more `_` followed by EOF
 * or non-alpha) and WG_TOKEN_IDENT (`_+[a-zA-Z]...`). */
static wg_lexer_token_t
scan_underscore_or_identifier(wg_lexer_t *lex)
{
    /* Consume the run of underscores. */
    while (!at_end(lex) && peek(lex) == '_') {
        advance(lex);
    }
    if (!at_end(lex) && is_alpha(peek(lex))) {
        /* `_+[a-zA-Z]` is an identifier. Continue scanning the tail. */
        return scan_identifier(lex);
    }
    /* Run of underscores with no alpha tail: wildcard. */
    return make_token(lex, WG_TOKEN_UNDERSCORE);
}

/* ======================================================================== */
/* Directive                                                                */
/* ======================================================================== */

/* On entry, the leading '.' was just consumed and the next byte is alpha.
 * Try to match one of the six directives. On miss, roll the cursor back
 * to just after the dot (so the next call lexes the alpha tail as IDENT)
 * and return DOT. */
static wg_lexer_token_t
scan_directive_or_dot(wg_lexer_t *lex)
{
    /* Snapshot for rollback. */
    const char *save_current = lex->current;
    uint32_t    save_line    = lex->line;
    uint32_t    save_col     = lex->col;

    while (!at_end(lex) && is_alnum_underscore(peek(lex))) {
        advance(lex);
    }

    uint32_t length = (uint32_t)(lex->current - lex->start); /* includes '.' */
    const char *text = lex->start;

    struct { const char *t; uint32_t n; wg_lexer_token_kind_t k; } table[] = {
        { ".decl",      5, WG_TOKEN_DECL },
        { ".input",     6, WG_TOKEN_INPUT },
        { ".output",    7, WG_TOKEN_OUTPUT },
        { ".printsize", 10, WG_TOKEN_PRINTSIZE },
        { ".plan",      5, WG_TOKEN_PLAN },
        { ".query",     6, WG_TOKEN_QUERY },
    };
    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (table[i].n == length && memcmp(table[i].t, text, length) == 0) {
            return make_token(lex, table[i].k);
        }
    }

    /* Unknown ".xxx" — roll back, emit just DOT. The next call will
     * lex the alpha tail as IDENT. */
    lex->current = save_current;
    lex->line    = save_line;
    lex->col     = save_col;
    return make_token(lex, WG_TOKEN_DOT);
}

/* ======================================================================== */
/* Main scanner                                                             */
/* ======================================================================== */

static wg_lexer_token_t
scan_token(wg_lexer_t *lex)
{
    if (at_end(lex)) {
        wg_lexer_token_t tok;
        tok.kind        = WG_TOKEN_EOF;
        tok.start       = lex->current;
        tok.length      = 0;
        tok.byte_offset = (uint32_t)(lex->current - lex->source);
        tok.line        = lex->line;
        tok.col         = lex->col;
        return tok;
    }

    lex->start      = lex->current;
    lex->start_line = lex->line;
    lex->start_col  = lex->col;

    char c = peek(lex);
    advance(lex);

    /* Trivia */
    if (c == ' ' || c == '\t' || c == '\r') {
        return scan_ws(lex);
    }
    if (c == '\n') {
        return scan_newline(lex);
    }
    if (c == '#') {
        return scan_line_comment_hash(lex);
    }
    if (c == '/') {
        if (peek(lex) == '/') {
            advance(lex);
            return scan_line_comment_slash(lex);
        }
        if (peek(lex) == '*') {
            advance(lex);
            return scan_block_comment(lex);
        }
        return make_token(lex, WG_TOKEN_SLASH);
    }

    /* Identifiers / keywords / underscore wildcard */
    if (c == '_') {
        return scan_underscore_or_identifier(lex);
    }
    if (is_alpha(c)) {
        return scan_identifier(lex);
    }

    /* Integer */
    if (is_digit(c)) {
        return scan_integer(lex);
    }

    /* String */
    if (c == '"') {
        return scan_string(lex);
    }

    /* Directive or dot */
    if (c == '.') {
        if (!at_end(lex) && is_alpha(peek(lex))) {
            return scan_directive_or_dot(lex);
        }
        return make_token(lex, WG_TOKEN_DOT);
    }

    /* Punctuation and operators */
    switch (c) {
    case '(':  return make_token(lex, WG_TOKEN_LPAREN);
    case ')':  return make_token(lex, WG_TOKEN_RPAREN);
    case ',':  return make_token(lex, WG_TOKEN_COMMA);
    case '+':  return make_token(lex, WG_TOKEN_PLUS);
    case '-':  return make_token(lex, WG_TOKEN_MINUS);
    case '*':  return make_token(lex, WG_TOKEN_STAR);
    case '%':  return make_token(lex, WG_TOKEN_PERCENT);
    case ':':
        return match(lex, '-')
             ? make_token(lex, WG_TOKEN_HORN)
             : make_token(lex, WG_TOKEN_COLON);
    case '!':
        return match(lex, '=')
             ? make_token(lex, WG_TOKEN_NEQ)
             : make_token(lex, WG_TOKEN_BANG);
    case '<':
        return match(lex, '=')
             ? make_token(lex, WG_TOKEN_LTE)
             : make_token(lex, WG_TOKEN_LT);
    case '>':
        return match(lex, '=')
             ? make_token(lex, WG_TOKEN_GTE)
             : make_token(lex, WG_TOKEN_GT);
    case '=':
        return make_token(lex, WG_TOKEN_EQ);
    default:
        break;
    }

    /* Unrecognised byte: ERROR spanning the single byte; cursor already
     * advanced so the next call resumes one byte further on. */
    return make_token(lex, WG_TOKEN_ERROR);
}

/* ======================================================================== */
/* Public API                                                               */
/* ======================================================================== */

void
wg_lexer_init(wg_lexer_t *lexer, const char *source, size_t len)
{
    lexer->source     = source;
    lexer->end        = source + len;
    lexer->current    = source;
    lexer->start      = source;
    lexer->line       = 1;
    lexer->col        = 1;
    lexer->start_line = 1;
    lexer->start_col  = 1;
}

wg_lexer_token_t
wg_lexer_next(wg_lexer_t *lexer)
{
    return scan_token(lexer);
}

wg_lexer_token_t
wg_lexer_peek(wg_lexer_t *lexer)
{
    /* Save and restore. Simpler than a one-token lookahead buffer; the
     * formatter peeks rarely enough that the cost is irrelevant. */
    wg_lexer_t saved = *lexer;
    wg_lexer_token_t tok = scan_token(lexer);
    *lexer = saved;
    return tok;
}

bool
wg_lexer_is_trivia(wg_lexer_token_kind_t kind)
{
    return kind == WG_TOKEN_WS
        || kind == WG_TOKEN_NEWLINE
        || kind == WG_TOKEN_LINE_COMMENT_HASH
        || kind == WG_TOKEN_LINE_COMMENT_SLASH
        || kind == WG_TOKEN_BLOCK_COMMENT;
}

const char *
wg_lexer_token_kind_str(wg_lexer_token_kind_t kind)
{
    switch (kind) {
    case WG_TOKEN_WS:                  return "WS";
    case WG_TOKEN_NEWLINE:             return "NEWLINE";
    case WG_TOKEN_LINE_COMMENT_HASH:   return "LINE_COMMENT_HASH";
    case WG_TOKEN_LINE_COMMENT_SLASH:  return "LINE_COMMENT_SLASH";
    case WG_TOKEN_BLOCK_COMMENT:       return "BLOCK_COMMENT";
    case WG_TOKEN_IDENT:               return "IDENT";
    case WG_TOKEN_INTEGER:             return "INTEGER";
    case WG_TOKEN_STRING:              return "STRING";
    case WG_TOKEN_TRUE:                return "TRUE";
    case WG_TOKEN_FALSE:               return "FALSE";
    case WG_TOKEN_UNDERSCORE:          return "UNDERSCORE";
    case WG_TOKEN_COUNT:               return "COUNT";
    case WG_TOKEN_SUM:                 return "SUM";
    case WG_TOKEN_MIN:                 return "MIN";
    case WG_TOKEN_MAX:                 return "MAX";
    case WG_TOKEN_AVG:                 return "AVG";
    case WG_TOKEN_INT32:               return "INT32";
    case WG_TOKEN_INT64:               return "INT64";
    case WG_TOKEN_STRING_TYPE:         return "STRING_TYPE";
    case WG_TOKEN_SYMBOL_TYPE:         return "SYMBOL_TYPE";
    case WG_TOKEN_LPAREN:              return "LPAREN";
    case WG_TOKEN_RPAREN:              return "RPAREN";
    case WG_TOKEN_COMMA:               return "COMMA";
    case WG_TOKEN_DOT:                 return "DOT";
    case WG_TOKEN_COLON:               return "COLON";
    case WG_TOKEN_BANG:                return "BANG";
    case WG_TOKEN_HORN:                return "HORN";
    case WG_TOKEN_EQ:                  return "EQ";
    case WG_TOKEN_NEQ:                 return "NEQ";
    case WG_TOKEN_LT:                  return "LT";
    case WG_TOKEN_GT:                  return "GT";
    case WG_TOKEN_LTE:                 return "LTE";
    case WG_TOKEN_GTE:                 return "GTE";
    case WG_TOKEN_PLUS:                return "PLUS";
    case WG_TOKEN_MINUS:               return "MINUS";
    case WG_TOKEN_STAR:                return "STAR";
    case WG_TOKEN_SLASH:               return "SLASH";
    case WG_TOKEN_PERCENT:             return "PERCENT";
    case WG_TOKEN_BAND:                return "BAND";
    case WG_TOKEN_BOR:                 return "BOR";
    case WG_TOKEN_BXOR:                return "BXOR";
    case WG_TOKEN_BNOT:                return "BNOT";
    case WG_TOKEN_BSHL:                return "BSHL";
    case WG_TOKEN_BSHR:                return "BSHR";
    case WG_TOKEN_HASH:                return "HASH";
    case WG_TOKEN_MD5:                 return "MD5";
    case WG_TOKEN_SHA1:                return "SHA1";
    case WG_TOKEN_SHA256:              return "SHA256";
    case WG_TOKEN_SHA512:              return "SHA512";
    case WG_TOKEN_HMAC_SHA256:         return "HMAC_SHA256";
    case WG_TOKEN_UUID4:               return "UUID4";
    case WG_TOKEN_UUID5:               return "UUID5";
    case WG_TOKEN_STRLEN:              return "STRLEN";
    case WG_TOKEN_CAT:                 return "CAT";
    case WG_TOKEN_SUBSTR:              return "SUBSTR";
    case WG_TOKEN_CONTAINS:            return "CONTAINS";
    case WG_TOKEN_STR_PREFIX:          return "STR_PREFIX";
    case WG_TOKEN_STR_SUFFIX:          return "STR_SUFFIX";
    case WG_TOKEN_STR_ORD:             return "STR_ORD";
    case WG_TOKEN_TO_UPPER:            return "TO_UPPER";
    case WG_TOKEN_TO_LOWER:            return "TO_LOWER";
    case WG_TOKEN_STR_REPLACE:         return "STR_REPLACE";
    case WG_TOKEN_TRIM:                return "TRIM";
    case WG_TOKEN_TO_STRING:           return "TO_STRING";
    case WG_TOKEN_TO_NUMBER:           return "TO_NUMBER";
    case WG_TOKEN_DECL:                return "DECL";
    case WG_TOKEN_INPUT:               return "INPUT";
    case WG_TOKEN_OUTPUT:              return "OUTPUT";
    case WG_TOKEN_PRINTSIZE:           return "PRINTSIZE";
    case WG_TOKEN_PLAN:                return "PLAN";
    case WG_TOKEN_QUERY:               return "QUERY";
    case WG_TOKEN_EOF:                 return "EOF";
    case WG_TOKEN_ERROR:               return "ERROR";
    }
    return "UNKNOWN";
}
