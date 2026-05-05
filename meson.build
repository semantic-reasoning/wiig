project('wiig', 'c',
  version : '0.1.0',
  license : ['GPL-3.0-or-later', 'LicenseRef-Commercial'],
  license_files : ['LICENSE.md'],
  meson_version : '>= 1.1.0',
  default_options : [
    'c_std=c17',
    'warning_level=2',
    'b_ndebug=if-release',
    'default_library=both',
  ],
)

# ----------------------------------------------------------------------------
# Project metadata
# ----------------------------------------------------------------------------
project_description = 'Datalog (.dl) formatter and syntax highlighter for the wirelog dialect.'

# ----------------------------------------------------------------------------
# Dependencies
# ----------------------------------------------------------------------------
# GLib floor matches RHEL 9 / Ubuntu 22.04 / Debian 11 baselines so that
# contributors on those distros can build without backports.
glib_dep = dependency('glib-2.0', version : '>= 2.68', required : true)

# ----------------------------------------------------------------------------
# Include directories
# ----------------------------------------------------------------------------
wiig_inc     = include_directories('.')
wiig_src_inc = include_directories('wiig')

# ----------------------------------------------------------------------------
# Subdirectories
# ----------------------------------------------------------------------------
subdir('wiig')
subdir('cli')

if get_option('tests')
  subdir('tests')
endif

# ----------------------------------------------------------------------------
# pkg-config
# ----------------------------------------------------------------------------
pkg = import('pkgconfig')
pkg.generate(libwiig,
  name        : meson.project_name(),
  description : project_description,
  version     : meson.project_version(),
  filebase    : meson.project_name(),
  subdirs     : meson.project_name(),
  requires    : ['glib-2.0'],
)

# ----------------------------------------------------------------------------
# Install LICENSE.md alongside the library
# ----------------------------------------------------------------------------
install_data('LICENSE.md',
  install_dir : get_option('datadir') / meson.project_name(),
)

# ----------------------------------------------------------------------------
# Summary
# ----------------------------------------------------------------------------
summary({
  'C standard'    : 'c17',
  'GLib'          : glib_dep.version(),
  'Werror'        : get_option('werror'),  # Meson built-in option
  'Tests enabled' : get_option('tests'),
}, section : 'wiig ' + meson.project_version())
