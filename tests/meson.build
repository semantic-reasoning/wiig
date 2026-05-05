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
test_lexer = executable('test_lexer',
  files(
    'test_lexer.c',
    '../wiig/lexer/lexer.c',
  ),
  include_directories : [wiig_inc, wiig_src_inc],
  c_args              : [
    '-DWG_FIXTURE_DIR="' + meson.current_source_dir() / 'fixtures' + '"',
  ],
  dependencies        : [glib_dep],
  install             : false,
)

test('lexer', test_lexer, suite : 'unit')
