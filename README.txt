kiwmi
=====

A fully programmable Waylnd Compositor.

NOTE: This whole project is a work in project, and is far from working.

Concepts
--------

kiwmi is cool and all, because it doesn't enforce any logic itself.
This means that the user can easily (via some Lua scripting) create their own behaiors, and can do anything they wish!
Of course this means that there is a steep curve to get into it, but for some people it might be worth it.

In the future there might be a base to base your stuff on, but at the moment this doesn't even work.

Dependencies
------------

- https://github.com/swaywm/wlroots[wlroots]
- meson (build)
- ninja (build)
- git (build, optional, to fetch the version)

Building
--------

Make sure all dependencies are installed.

Run:

----
$ meson build
$ ninja -C build
----

Install with:

----
# ninja -C build install
----

Contribution
------------

You want to contribute? Great!

Future requests, bug reports and PRs are always welcome.
Note that pull requests without a valid issue are ignored to decrease the amount of duplicate work.
Also read link:CONTRIBUTING.adoc[CONTRIBUTING.adoc].

If anything is unclear, feel free to contact me.

If you don't program but want to contribute to kiwmi, spread the word about kiwmi and star the repo.
