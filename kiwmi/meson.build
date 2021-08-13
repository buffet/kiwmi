kiwmi_sources = files(
  'main.c',
  'server.c',
  'color.c',
  'desktop/desktop.c',
  'desktop/layer_shell.c',
  'desktop/output.c',
  'desktop/view.c',
  'desktop/xdg_shell.c',
  'input/cursor.c',
  'input/input.c',
  'input/keyboard.c',
  'input/seat.c',
  'luak/ipc.c',
  'luak/kiwmi_cursor.c',
  'luak/kiwmi_keyboard.c',
  'luak/kiwmi_lua_callback.c',
  'luak/kiwmi_output.c',
  'luak/kiwmi_renderer.c',
  'luak/kiwmi_server.c',
  'luak/kiwmi_view.c',
  'luak/lua_compat.c',
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
