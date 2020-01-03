kiwmic_sources = files(
  'main.c',
)

kiwmic_deps = [
  protocols_client,
  wayland_client,
]

executable(
  'kiwmic',
  kiwmic_sources,
  include_directories: [include],
  dependencies: kiwmic_deps,
  install: true,
)
