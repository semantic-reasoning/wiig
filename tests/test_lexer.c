/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) CleverPlant. Commercial: inquiry@cleverplant.com */

/*
 * test_lexer.c - unit tests for the wiig lossless lexer.
 *
 * The lexer is internal (not exported from libwiig); this test target
 * compiles wiig/lexer/lexer.c directly into the binary so the WG_*
 * symbols are link-time visible despite gnu_symbol_visibility=hidden.
 *
 * Each test_X function returns 0 on pass and 1 on failure, mirroring
 * the explicit-branch pattern in tests/test_skeleton.c. main() runs
 * them in sequence and returns the first non-zero result.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wiig/lexer/lexer.h"

#ifndef WG_FIXTURE_DIR
#define WG_FIXTURE_DIR "tests/fixtures"
#endif

/* ======================================================================== */
/* Helpers                                                                  */
/* ======================================================================== */

static int g_failures;

static void
fail(const char *test, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "FAIL %s: ", test);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    g_failures++;
}

static void
init_lex(wg_lexer_t *lex, const char *src)
{
    wg_lexer_init(lex, src, strlen(src));
}

/* Read a fixture file into a heap buffer. Caller frees. */
static char *
read_fixture(const char *name, size_t *out_len)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", WG_FIXTURE_DIR, name);
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "FAIL: cannot open fixture %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    fread(buf, 1, (size_t)sz, f);
    buf[sz] = '\0';
    fclose(f);
    *out_len = (size_t)sz;
    return buf;
}

typedef struct {
    wg_lexer_token_kind_t kind;
    const char *text;   /* expected start[0..length]; NULL = don't check */
} expect_t;

static int
expect_stream(const char *test, const char *src,
              const expect_t *exp, size_t n)
{
    wg_lexer_t lex;
    init_lex(&lex, src);
    int rc = 0;
    for (size_t i = 0; i < n; i++) {
        wg_lexer_token_t t = wg_lexer_next(&lex);
        if (t.kind != exp[i].kind) {
            fail(test, "tok %zu: expected %s, got %s",
                 i,
                 wg_lexer_token_kind_str(exp[i].kind),
                 wg_lexer_token_kind_str(t.kind));
            rc = 1;
            break;
        }
        if (exp[i].text) {
            uint32_t want_len = (uint32_t)strlen(exp[i].text);
            if (t.length != want_len ||
                memcmp(t.start, exp[i].text, want_len) != 0) {
                fail(test, "tok %zu (%s): expected text %.*s, got %.*s",
                     i, wg_lexer_token_kind_str(t.kind),
                     (int)want_len, exp[i].text,
                     (int)t.length, t.start);
                rc = 1;
                break;
            }
        }
    }
    return rc;
}

/* ======================================================================== */
/* Tests                                                                    */
/* ======================================================================== */

static int
test_empty_input(void)
{
    wg_lexer_t lex;
    init_lex(&lex, "");
    wg_lexer_token_t t = wg_lexer_next(&lex);
    if (t.kind != WG_TOKEN_EOF || t.length != 0 ||
        t.line != 1 || t.col != 1) {
        fail("empty_input", "got kind=%s len=%u line=%u col=%u",
             wg_lexer_token_kind_str(t.kind),
             t.length, t.line, t.col);
        return 1;
    }
    /* Idempotent at EOF. */
    t = wg_lexer_next(&lex);
    return t.kind == WG_TOKEN_EOF ? 0 : 1;
}

static int
test_single_tokens(void)
{
    static const struct { const char *src; wg_lexer_token_kind_t kind; } cases[] = {
        { "(",  WG_TOKEN_LPAREN },
        { ")",  WG_TOKEN_RPAREN },
        { ",",  WG_TOKEN_COMMA },
        { ":",  WG_TOKEN_COLON },
        { "!",  WG_TOKEN_BANG },
        { "+",  WG_TOKEN_PLUS },
        { "-",  WG_TOKEN_MINUS },
        { "*",  WG_TOKEN_STAR },
        { "/",  WG_TOKEN_SLASH },
        { "%",  WG_TOKEN_PERCENT },
        { "=",  WG_TOKEN_EQ },
        { "<",  WG_TOKEN_LT },
        { ">",  WG_TOKEN_GT },
        { ":-", WG_TOKEN_HORN },
        { "!=", WG_TOKEN_NEQ },
        { "<=", WG_TOKEN_LTE },
        { ">=", WG_TOKEN_GTE },
        { ".",  WG_TOKEN_DOT },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        wg_lexer_t lex;
        init_lex(&lex, cases[i].src);
        wg_lexer_token_t t = wg_lexer_next(&lex);
        if (t.kind != cases[i].kind) {
            fail("single_tokens", "src=%s kind=%s",
                 cases[i].src, wg_lexer_token_kind_str(t.kind));
            return 1;
        }
    }
    return 0;
}

static int
test_keywords(void)
{
    static const struct { const char *src; wg_lexer_token_kind_t kind; } cases[] = {
        { "True",       WG_TOKEN_TRUE },
        { "False",      WG_TOKEN_FALSE },
        { "count",      WG_TOKEN_COUNT },
        { "COUNT",      WG_TOKEN_COUNT },
        { "average",    WG_TOKEN_AVG },
        { "AVG",        WG_TOKEN_AVG },
        { "int32",      WG_TOKEN_INT32 },
        { "int64",      WG_TOKEN_INT64 },
        { "string",     WG_TOKEN_STRING_TYPE },
        { "symbol",     WG_TOKEN_SYMBOL_TYPE },
        { "band",       WG_TOKEN_BAND },
        { "bnot",       WG_TOKEN_BNOT },
        { "hash",       WG_TOKEN_HASH },
        { "sha256",     WG_TOKEN_SHA256 },
        { "hmac_sha256",WG_TOKEN_HMAC_SHA256 },
        { "uuid4",      WG_TOKEN_UUID4 },
        { "strlen",     WG_TOKEN_STRLEN },
        { "str_prefix", WG_TOKEN_STR_PREFIX },
        { "to_string",  WG_TOKEN_TO_STRING },
        /* Identifier that is NOT a keyword. */
        { "edge",       WG_TOKEN_IDENT },
        { "MyRel",      WG_TOKEN_IDENT },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        wg_lexer_t lex;
        init_lex(&lex, cases[i].src);
        wg_lexer_token_t t = wg_lexer_next(&lex);
        if (t.kind != cases[i].kind) {
            fail("keywords", "src=%s expected %s got %s",
                 cases[i].src,
                 wg_lexer_token_kind_str(cases[i].kind),
                 wg_lexer_token_kind_str(t.kind));
            return 1;
        }
    }
    return 0;
}

static int
test_directives(void)
{
    static const struct { const char *src; wg_lexer_token_kind_t kind; } cases[] = {
        { ".decl",      WG_TOKEN_DECL },
        { ".input",     WG_TOKEN_INPUT },
        { ".output",    WG_TOKEN_OUTPUT },
        { ".printsize", WG_TOKEN_PRINTSIZE },
        { ".plan",      WG_TOKEN_PLAN },
        { ".query",     WG_TOKEN_QUERY },
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        wg_lexer_t lex;
        init_lex(&lex, cases[i].src);
        wg_lexer_token_t t = wg_lexer_next(&lex);
        if (t.kind != cases[i].kind) {
            fail("directives", "src=%s got %s",
                 cases[i].src, wg_lexer_token_kind_str(t.kind));
            return 1;
        }
    }
    return 0;
}

static int
test_unknown_directive_rollback(void)
{
    /* `.foobar` must lex as DOT then IDENT(foobar). */
    expect_t e[] = {
        { WG_TOKEN_DOT,   "." },
        { WG_TOKEN_IDENT, "foobar" },
        { WG_TOKEN_EOF,   NULL },
    };
    return expect_stream("unknown_directive_rollback",
                         ".foobar", e, sizeof(e) / sizeof(e[0]));
}

static int
test_underscore_disambiguation(void)
{
    expect_t e1[] = { { WG_TOKEN_UNDERSCORE, "_" },   { WG_TOKEN_EOF, NULL } };
    expect_t e2[] = { { WG_TOKEN_UNDERSCORE, "__" },  { WG_TOKEN_EOF, NULL } };
    expect_t e3[] = { { WG_TOKEN_UNDERSCORE, "___" }, { WG_TOKEN_EOF, NULL } };
    expect_t e4[] = { { WG_TOKEN_IDENT,      "_x" },  { WG_TOKEN_EOF, NULL } };
    expect_t e5[] = { { WG_TOKEN_IDENT,      "__x" }, { WG_TOKEN_EOF, NULL } };
    expect_t e6[] = { { WG_TOKEN_IDENT,      "_____foo" },
                      { WG_TOKEN_EOF, NULL } };
    int rc = 0;
    rc |= expect_stream("underscore _",   "_",        e1, 2);
    rc |= expect_stream("underscore __",  "__",       e2, 2);
    rc |= expect_stream("underscore ___", "___",      e3, 2);
    rc |= expect_stream("underscore _x",  "_x",       e4, 2);
    rc |= expect_stream("underscore __x", "__x",      e5, 2);
    rc |= expect_stream("underscore _5_foo", "_____foo", e6, 2);
    return rc;
}

static int
test_integer_and_string(void)
{
    expect_t e[] = {
        { WG_TOKEN_INTEGER, "9223372036854775807" },
        { WG_TOKEN_WS,      " " },
        { WG_TOKEN_STRING,  "\"hello\"" },
        { WG_TOKEN_EOF,     NULL },
    };
    return expect_stream("integer_and_string",
                         "9223372036854775807 \"hello\"",
                         e, sizeof(e) / sizeof(e[0]));
}

static int
test_comments(void)
{
    /* Hash comment, slash comment, block comment, all preserved as trivia. */
    const char *src = "# h\n// s\n/* b */";
    expect_t e[] = {
        { WG_TOKEN_LINE_COMMENT_HASH,  "# h" },
        { WG_TOKEN_NEWLINE,            "\n" },
        { WG_TOKEN_LINE_COMMENT_SLASH, "// s" },
        { WG_TOKEN_NEWLINE,            "\n" },
        { WG_TOKEN_BLOCK_COMMENT,      "/* b */" },
        { WG_TOKEN_EOF,                NULL },
    };
    return expect_stream("comments", src, e, sizeof(e) / sizeof(e[0]));
}

static int
test_block_comment_multiline(void)
{
    const char *src = "/* line1\nline2\nline3 */x";
    wg_lexer_t lex;
    init_lex(&lex, src);
    wg_lexer_token_t t = wg_lexer_next(&lex);
    if (t.kind != WG_TOKEN_BLOCK_COMMENT) {
        fail("block_multiline", "first kind=%s",
             wg_lexer_token_kind_str(t.kind));
        return 1;
    }
    /* After the block comment, line counter must be 3, col after the
     * closing slash must be the byte after '/' on line 3. */
    t = wg_lexer_next(&lex);
    if (t.kind != WG_TOKEN_IDENT || t.line != 3) {
        fail("block_multiline", "after-comment kind=%s line=%u",
             wg_lexer_token_kind_str(t.kind), t.line);
        return 1;
    }
    return 0;
}

static int
test_crlf(void)
{
    /* \r is WS, \n is NEWLINE. \r\n -> WS(\r) + NEWLINE(\n). */
    const char *src = "a\r\nb";
    expect_t e[] = {
        { WG_TOKEN_IDENT,   "a" },
        { WG_TOKEN_WS,      "\r" },
        { WG_TOKEN_NEWLINE, "\n" },
        { WG_TOKEN_IDENT,   "b" },
        { WG_TOKEN_EOF,     NULL },
    };
    int rc = expect_stream("crlf", src, e, sizeof(e) / sizeof(e[0]));
    if (rc) return rc;
    /* Line counter on `b` must be 2. */
    wg_lexer_t lex;
    init_lex(&lex, src);
    (void)wg_lexer_next(&lex);  /* a */
    (void)wg_lexer_next(&lex);  /* WS */
    (void)wg_lexer_next(&lex);  /* NEWLINE */
    wg_lexer_token_t t = wg_lexer_next(&lex);
    return t.line == 2 ? 0 : 1;
}

static int
test_error_recovery(void)
{
    /* `@@@` must produce three ERROR tokens then EOF, no infinite loop. */
    expect_t e[] = {
        { WG_TOKEN_ERROR, "@" },
        { WG_TOKEN_ERROR, "@" },
        { WG_TOKEN_ERROR, "@" },
        { WG_TOKEN_EOF,   NULL },
    };
    return expect_stream("error_recovery", "@@@", e,
                         sizeof(e) / sizeof(e[0]));
}

static int
test_peek(void)
{
    wg_lexer_t lex;
    init_lex(&lex, "x y");
    wg_lexer_token_t p1 = wg_lexer_peek(&lex);
    wg_lexer_token_t p2 = wg_lexer_peek(&lex);
    wg_lexer_token_t n1 = wg_lexer_next(&lex);
    if (p1.kind != WG_TOKEN_IDENT || p2.kind != WG_TOKEN_IDENT ||
        n1.kind != WG_TOKEN_IDENT ||
        p1.start != n1.start || p2.start != n1.start) {
        fail("peek", "peek/next mismatch");
        return 1;
    }
    /* After advance, peek shows the WS (the next token in source order). */
    wg_lexer_token_t p3 = wg_lexer_peek(&lex);
    if (p3.kind != WG_TOKEN_WS) {
        fail("peek", "post-next peek kind=%s",
             wg_lexer_token_kind_str(p3.kind));
        return 1;
    }
    return 0;
}

/* The signature lossless invariant: concatenating every non-EOF
 * token's start[..length] reproduces the source byte-for-byte. */
static int
roundtrip(const char *test, const char *src, size_t len)
{
    wg_lexer_t lex;
    wg_lexer_init(&lex, src, len);
    char *out = malloc(len + 1);
    if (!out) {
        fail(test, "alloc");
        return 1;
    }
    size_t out_len = 0;
    for (;;) {
        wg_lexer_token_t t = wg_lexer_next(&lex);
        if (t.kind == WG_TOKEN_EOF) {
            if (t.length != 0) {
                fail(test, "EOF length %u", t.length);
                free(out);
                return 1;
            }
            break;
        }
        memcpy(out + out_len, t.start, t.length);
        out_len += t.length;
    }
    int rc = (out_len == len && memcmp(out, src, len) == 0) ? 0 : 1;
    if (rc) {
        fail(test, "expected %zu bytes, got %zu", len, out_len);
    }
    free(out);
    return rc;
}

static int
test_roundtrip_fixtures(void)
{
    static const char *fixtures[] = {
        "simple.dl",
        "graph_reachability.dl",
        "access_control.dl",
        "edge.dl",
    };
    int rc = 0;
    for (size_t i = 0; i < sizeof(fixtures) / sizeof(fixtures[0]); i++) {
        size_t len = 0;
        char *buf = read_fixture(fixtures[i], &len);
        if (!buf) {
            return 1;
        }
        rc |= roundtrip(fixtures[i], buf, len);
        free(buf);
    }
    return rc;
}

static int
test_eof_position(void)
{
    const char *src = "abc";
    wg_lexer_t lex;
    init_lex(&lex, src);
    (void)wg_lexer_next(&lex);
    wg_lexer_token_t eof = wg_lexer_next(&lex);
    if (eof.kind != WG_TOKEN_EOF || eof.length != 0 ||
        eof.start != src + 3) {
        fail("eof_position", "kind=%s len=%u start_off=%ld",
             wg_lexer_token_kind_str(eof.kind),
             eof.length, (long)(eof.start - src));
        return 1;
    }
    return 0;
}

static int
test_trailing_newline_distinction(void)
{
    /* "x"  vs "x\n" must produce different streams. */
    char a[] = { 'x' };
    char b[] = { 'x', '\n' };
    if (roundtrip("no_trailing_nl", a, sizeof(a))) return 1;
    if (roundtrip("with_trailing_nl", b, sizeof(b))) return 1;
    /* Verify the streams differ: 'x' produces 1 non-EOF token; 'x\n'
     * produces 2 (IDENT + NEWLINE). */
    wg_lexer_t la, lb;
    wg_lexer_init(&la, a, sizeof(a));
    wg_lexer_init(&lb, b, sizeof(b));
    int n_a = 0, n_b = 0;
    while (wg_lexer_next(&la).kind != WG_TOKEN_EOF) n_a++;
    while (wg_lexer_next(&lb).kind != WG_TOKEN_EOF) n_b++;
    if (n_a != 1 || n_b != 2) {
        fail("trailing_nl", "n_a=%d n_b=%d", n_a, n_b);
        return 1;
    }
    return 0;
}

/* ======================================================================== */
/* Main                                                                     */
/* ======================================================================== */

int
main(void)
{
    int (*tests[])(void) = {
        test_empty_input,
        test_single_tokens,
        test_keywords,
        test_directives,
        test_unknown_directive_rollback,
        test_underscore_disambiguation,
        test_integer_and_string,
        test_comments,
        test_block_comment_multiline,
        test_crlf,
        test_error_recovery,
        test_peek,
        test_eof_position,
        test_trailing_newline_distinction,
        test_roundtrip_fixtures,
    };
    int rc = 0;
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        rc |= tests[i]();
    }
    if (g_failures > 0) {
        fprintf(stderr, "test_lexer: %d failure(s)\n", g_failures);
        return 1;
    }
    return rc;
}
