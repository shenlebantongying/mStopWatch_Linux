project(
    'mStopWatch_Linux',
    'cpp',
    version: '1.0.0',
    default_options: ['warning_level=3', 'cpp_std=gnu++23'],
)

adw_deps = [dependency('libadwaita-1')]

executable(
    'mStopWatch',
    'mStopWatch.cpp',
    install: true,
    dependencies: adw_deps,
)

configure_file(
    copy: true,
    input: 'org.slbtty.mStopWatch.desktop.in',
    output: '@BASENAME@',
    install: true,
    install_dir: join_paths(get_option('datadir'), 'applications'),
)
