test_skeleton = executable('test_skeleton',
  files('test_skeleton.c'),
  dependencies : [wiig_dep],
  install      : false,
)

test('skeleton', test_skeleton, suite : 'unit')

# test_lexer compiles wiig/lexer/lexer.c directly into the test binary
# so the WG_* internal symbols are reachable despite the library's
# gnu_symbol_visibility=hidden setting. This keeps the test self-
# contained regardless of whether the library is built shared, static,
# or both.
# meson.current_source_dir() uses backslashes on Windows, which would
# corrupt the C string literal (\a, \t, \w treated as escape sequences).
# Normalise to forward slashes; fopen accepts both on every platform.
fixture_dir = meson.current_source_dir().replace('\\', '/') + '/fixtures'

test_lexer = executable('test_lexer',
  files(
    'test_lexer.c',
    '../wiig/lexer/lexer.c',
  ),
  include_directories : [wiig_inc, wiig_src_inc],
  c_args              : [
    '-DWG_FIXTURE_DIR="' + fixture_dir + '"',
  ],
  dependencies        : [glib_dep],
  install             : false,
)

test('lexer', test_lexer, suite : 'unit')
