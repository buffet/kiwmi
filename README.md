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

If you plan to use luajit instead, use the following commands instead.

```
$ meson -Dlua-pkg=luajit build
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


## Using

Since kiwmi does not have any logic in itself, all logic must be provided by lua scripts, specifically the init.lua file.
See the lua_docs.md for a ruleset for the lua files. To test a lua command / file, you can use the kiwmi client (kiwmic).
By default, no lua configuration file is supplied. If you *really* want an example, you can look at https://github.com/zena1/kiwmi./blob/master/init.lua  


