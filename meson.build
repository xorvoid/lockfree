project('lockfree', 'c')

##################################
## Common flags

flags_c = [
  '-std=c11',
  #'-fvisibility=hidden',
]

flags_cpp = [
  '-std=c++14',
  '-fno-exceptions', '-fno-rtti',
  #'-fvisibility=hidden', '-fvisibility-inlines-hidden',
]

flags_common = [
  '-fdiagnostics-color=always',
  '-D_GNU_SOURCE',
  '-Dtypeof=__typeof__',
  '-I' + meson.source_root() + '/src',
  '-g', '-fPIC',
  '-m64',
  '-ffast-math', '-fno-associative-math', '-fno-reciprocal-math',
  '-fno-strict-aliasing',
]

flags_warn = [
  ## Warning enables
  '-Wall', '-Werror', '-Wextra',

  ## Warning disables: annoying
  '-Wno-unused-function', '-Wno-unused-parameter',

  ## Warning disables: Complains about 'Type t[1] = {{0}}'
  '-Wno-missing-field-initializers',

  ## Warning disables: Complains about 'if (val < 0 || val >= LIMIT)' when val is unsigned
  '-Wno-type-limits',

  ## Warning disables: Complains about macros that expand to empty if-else bodies
  '-Wno-empty-body',
]

flags_release = [
  ## Optimization
  '-O2',

  ## Warning disables:
  '-Wno-unused-variable', '-Wno-unused-but-set-variable',

  ## Warning disables: Seriously, no comments in comments?
  '-Wno-comments',
]

flags_debug = [
]

flags_special = [
]

link_flags_common = [
  '-fdiagnostics-color=always',
  #'-fvisibility=hidden', '-fvisibility-inlines-hidden',
  '-lpthread', '-m64'
]

add_global_arguments(flags_c   + flags_common + flags_warn + flags_special, language : 'c')
add_global_arguments(flags_cpp + flags_common + flags_warn + flags_special, language : 'cpp')

add_global_link_arguments(link_flags_common, language : 'c')
add_global_link_arguments(link_flags_common, language : 'cpp')

## Release
add_global_arguments(flags_release, language : 'c')
add_global_arguments(flags_release, language : 'cpp')

## Debug
# add_global_arguments(flags_debug, language : 'c')
# add_global_arguments(flags_debug, language : 'cpp')

##################################
## Definitions
subdir('src')
