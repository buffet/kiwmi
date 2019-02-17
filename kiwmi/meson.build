kiwmi_sources = files(
  'main.c',
  'server.c',
)

kiwmi_deps = [
  wayland_server,
  wlroots,
]

executable(
  'kiwmi',
  kiwmi_sources,
  include_directories: [include],
  dependencies: kiwmi_deps,
  install: true,
)
