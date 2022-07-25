project(
  'kiwmi',
  'c',
  license: 'MPL-2.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
  ],
)

add_project_arguments(
  [
    '-DWLR_USE_UNSTABLE',
    '-D_POSIX_C_SOURCE=200809L',
  ],
  language: 'c',
)

git               = find_program('git', required: false)
lua               = dependency(get_option('lua-pkg'))
pixman            = dependency('pixman-1')
wayland_client    = dependency('wayland-client')
wayland_protocols = dependency('wayland-protocols')
wayland_scanner   = dependency('wayland-scanner')
wayland_server    = dependency('wayland-server')
wlroots           = dependency('wlroots')
xkbcommon         = dependency('xkbcommon')

include = include_directories('include')

version = get_option('kiwmi-version')
if version != ''
  version = '"@0@"'.format(version)
else
  if not git.found()
    error('git is required to make the version string')
  endif

  git_commit_hash = run_command([git.path(), 'describe', '--always', '--tags'], check: false)
  git_branch = run_command([git.path(), 'rev-parse', '--abbrev-ref', 'HEAD'], check: false)
  if git_commit_hash.returncode() != 0 or git_branch.returncode() != 0
    error('need the git directory to make the version string')
  endif

  version = '"@0@ (" __DATE__ ", branch \'@1@\')"'.format(
    git_commit_hash.stdout().strip(),
    git_branch.stdout().strip(),
  )
endif
add_project_arguments('-DKIWMI_VERSION=@0@'.format(version), language: 'c')

compiler = meson.get_compiler('c')
if compiler.get_id() == 'gcc' or compiler.get_id() == 'clang'
  add_project_arguments('-DUNUSED(x)=UNUSED_ ## x __attribute__((__unused__))', language: 'c')
else
  add_project_arguments('-DUNUSED(x)=UNUSED_ ## x', language: 'c')
endif

install_data(
  'kiwmi.desktop',
  install_dir: join_paths(get_option('datadir'), 'wayland-sessions')
)

subdir('protocols')
subdir('kiwmi')
subdir('kiwmic')
