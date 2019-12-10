wayland_protocols_dir = wayland_protocols.get_pkgconfig_variable('pkgdatadir')
wayland_scanner_bin   = wayland_scanner.get_pkgconfig_variable('wayland_scanner')

protocols_server = [
  wayland_protocols_dir / 'stable/xdg-shell/xdg-shell.xml',
]

protocols_server_src = []
protocols_server_inc = []

foreach protocol : protocols_server
  protocols_server_src += custom_target(
    protocol.underscorify() + '_server_c',
    input: protocol,
    output: '@BASENAME@-protocol.c',
    command: [wayland_scanner_bin, 'private-code', '@INPUT@', '@OUTPUT@'],
  )

  protocols_server_inc += custom_target(
    protocol.underscorify() + '_server_h',
    input: protocol,
    output: '@BASENAME@-protocol.h',
    command: [wayland_scanner_bin, 'server-header', '@INPUT@', '@OUTPUT@'],
  )
endforeach

protocols_server_lib = static_library(
  'protocols_server',
  [protocols_server_src, protocols_server_inc],
)

protocols_server = declare_dependency(
  link_with: protocols_server_lib,
  sources: protocols_server_inc,
)
