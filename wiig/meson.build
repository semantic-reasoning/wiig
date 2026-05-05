# ----------------------------------------------------------------------------
# Public headers (installed under ${includedir}/wiig/)
# ----------------------------------------------------------------------------
wiig_public_headers = files(
  'wiig.h',
  'wiig-types.h',
  'wiig-export.h',
)

# ----------------------------------------------------------------------------
# Library sources
# ----------------------------------------------------------------------------
wiig_sources = files(
  'wg-version.c',
)

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
