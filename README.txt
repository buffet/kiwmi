kiwmi
=====

kiwmi is a Wayland compositor that is programmed by the user.
This means that the sky is the limit:
You can implement any tiling or floating logic you want; even stuff like modal window management becomes easy.

This means there is a steep entrance barrier, but in the future there will be base configurations to start with.

NOTE: This whole project is a work in project, and not usable yet.

Installation
------------

Make sure all dependencies are installed.

Run:

  $ meson build
  $ ninja -C build

Install with:

  # ninja -C build install

Dependencies
------------

- wlroots (https://github.com/swaywm/wlroots)
- meson (build)
- ninja (build)
- git (build, optional, to fetch the version)

Contribution
------------

You want to contribute? Great!

Future requests, bug reports and PRs are always welcome.
Note that pull requests without a valid issue are ignored to decrease the amount of duplicate work.
Also read CONTRIBUTING.txt.

If anything is unclear, feel free to contact me.

If you don't program but want to contribute to kiwmi, spread the word about kiwmi and star the repo.
