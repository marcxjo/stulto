project(
    'stulto', 'c',
    version: '0.1.0',
    default_options: ['c_std=gnu99'],
    license: ['GPL-3.0-or-later'],
)

compiler = meson.get_compiler('c')

vte_dep = dependency('vte-2.91')

datadir = get_option('datadir')
desktopdatadir = datadir / 'applications'

subdir('src')
subdir('data')
