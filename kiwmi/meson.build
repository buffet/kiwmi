kiwmi_sources = files(
  'main.c',
  'input.c',
  'output.c',
  'server.c',
  'desktop/output.c',
  'input/cursor.c',
  'input/keyboard.c',
)

kiwmi_deps = [
  wayland_server,
  wlroots,
  xkbcommon,
]

executable(
  'kiwmi',
  kiwmi_sources,
  include_directories: [include],
  dependencies: kiwmi_deps,
  install: true,
)
