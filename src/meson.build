stulto_sources = [
    'exit-status.c',
    'stulto-application.c',
    'stulto-exec-data.c',
    'stulto-header-bar.c',
    'stulto-main-window.c',
    'stulto-session-manager.c',
    'stulto-session.c',
    'stulto-terminal-profile.c',
    'stulto-terminal.c',
    'stulto.c',
]

# Compiler dependencies

vte_dep = dependency('vte-2.91')

executable(
    'stulto', stulto_sources,
    dependencies: vte_dep,
    install: true
)
