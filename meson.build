project(
    'stulto', 'c',
    version: '0.1.0',
    default_options: ['c_std=gnu99'],
    license: ['GPL-3.0-or-later'],
)

st_name = meson.project_name()

datadir = get_option('datadir')
st_configdir = datadir / st_name
st_desktopdir = datadir / 'applications'

subdir('src')
subdir('data')
