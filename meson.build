project(
    'stulto', 'c',
    version: '0.1.0',
    default_options: ['c_std=gnu99'],
    license: ['GPL-3.0-or-later'],
)

# Naming

st_name = meson.project_name()

# Directories

datadir = get_option('datadir')
st_configdir = datadir / st_name
st_desktopdir = datadir / 'applications'

# Subdirs

subdir('src')
subdir('data')
