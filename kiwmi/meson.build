kiwmi_sources = files(
  'main.c',
  'server.c',
  'desktop/desktop.c',
  'desktop/output.c',
  'desktop/view.c',
  'desktop/xdg_shell.c',
  'input/cursor.c',
  'input/input.c',
  'input/keyboard.c',
  'luak/kiwmi_lua_callback.c',
  'luak/kiwmi_server.c',
  'luak/kiwmi_view.c',
  'luak/luak.c',
)

kiwmi_deps = [
  lua,
  protocols_server,
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
