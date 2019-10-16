kiwmi_sources = files(
  'main.c',
  'server.c',
  'desktop/desktop.c',
  'desktop/output.c',
  'input/cursor.c',
  'input/input.c',
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
