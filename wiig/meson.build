# ----------------------------------------------------------------------------
# Public headers (installed under ${includedir}/wiig/)
# ----------------------------------------------------------------------------
# wiig.h is the single umbrella header. Consumers include only
# `<wiig/wiig.h>`; no other wiig header is part of the public surface.
wiig_public_headers = files(
  'wiig.h',
)

# ----------------------------------------------------------------------------
# Library sources
# ----------------------------------------------------------------------------
wiig_sources = files(
  'wg-version.c',
)

# Internal subsystems extend wiig_sources via subdir(). None of the
# headers under these subdirs are installed; the umbrella header in
# wiig_public_headers is the entire public surface.
subdir('lexer')

# ----------------------------------------------------------------------------
# Library
# ----------------------------------------------------------------------------
libwiig = library('wiig',
  wiig_sources,
  include_directories    : [wiig_inc, wiig_src_inc],
  c_args                 : ['-DWIIG_BUILDING'],
  gnu_symbol_visibility  : 'hidden',
  dependencies           : [glib_dep],
  version                : meson.project_version(),
  soversion              : '0',
  install                : true,
)

install_headers(wiig_public_headers, subdir : 'wiig')

# Public consumer dependency (used by the CLI, the test harness, and any
# future in-tree consumers).
wiig_dep = declare_dependency(
  link_with           : libwiig,
  include_directories : [wiig_inc, wiig_src_inc],
  dependencies        : [glib_dep],
)
