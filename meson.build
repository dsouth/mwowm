project('mwowm', 'c')

add_global_arguments('-DWLR_USE_UNSTABLE', language: 'c')

add_project_dependencies(
  dependency('wayland-server'),
  dependency('wlroots-0.19'),
  language: 'c'
  )

include = include_directories(['include'])

subdir('src')

executable('mwowm', 'src/main.c',
  include_directories: include,
  sources: src_files)
