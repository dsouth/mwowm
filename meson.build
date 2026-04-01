project('mwowm', 'c')

add_global_arguments('-DWLR_USE_UNSTABLE', language: 'c')

add_project_dependencies(
  dependency('wayland-server'),
  dependency('wlroots-0.20'),
  dependency('xkbcommon'),
  language: 'c'
  )

include = include_directories(['include', 'protocol'])

subdir('src')
subdir('protocol')

full_src_files = src_files + protocol_files

executable('mwowm', 'src/main.c',
  include_directories: include,
  sources: full_src_files)
