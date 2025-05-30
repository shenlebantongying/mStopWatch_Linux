project('mStopWatch_Linux', 'cpp',
        version : '1.0.0',
        default_options : ['warning_level=3', 'cpp_std=gnu++23'])

adw_deps = [dependency('libadwaita-1'),dependency('adwaita-icon-theme')]
mStopWatch = executable('mStopWatch', 'mStopWatch.cpp', install : true,dependencies : adw_deps)
