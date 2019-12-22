<h1 align="center">kiwmi</h1>
<p align="center"><i>A fully programmable Wayland Compositor</i></p>
<hr><p align="center">
  <img alt="Stars" src="https://img.shields.io/github/stars/buffet/kiwmi.svg?label=Stars&style=flat" />
  <a href="https://github.com/buffet/kiwmi/issues"><img alt="GitHub issues" src="https://img.shields.io/github/issues/buffet/kiwmi.svg"/></a>
  <a href="https://github.com/buffet/kiwmi/graphs/contributors"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors/buffet/kiwmi"></a>
</p>

kiwmi is a work-in-progress extensive user-configurable Wayland Compositor.
kiwmi specifically does not enforce any logic, allowing for the creation of Lua-scripted behaviors, making arduous tasks such as modal window management become a breeze.
New users should be aware of the  steep learning curve present, however this will be reduced as the project matures.


## Getting Started

The dependencies required are:

- [wlroots](https://github.com/swaywm/wlroots)
- meson (building)
- ninja (building)
- git (build, optional)

### Building

After cloning/downloading the project and ensuring all dependencies are installed, building is as easy as running

```
$ meson build
$ ninja -C build
```

Installing is accomplished with the following command:

```
# ninja -C build install
```


## Contributing

Contributions are welcomed, especially while the project is in a heavy WIP stage.
If you believe you have a valid concern, read the [CONTRIBUTING](https://github.com/buffet/kiwmi/blob/master/CONTRIBUTING.md) document and please file an issue on the [issues page](https://github.com/buffet/kiwmi/issues/new).

For clarifications or suggestions on anything, please don't hesitate to contact me.
