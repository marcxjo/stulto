Stulto - A Terminal for Fools
=============================

About
-----

Stulto is a simple wrapper around the VTE terminal emulator widget for GTK3. It
is forked from [stupidterm](https://github.com/esmil/stupidterm.git).

Stulto is currently not particularly unique on the landscape of VTE terminals.
Its primary distinguishing characteristics include multiple sessions (with
session state indicated in the window title rather than in visible tabs) and
configuration via a simple .ini file. Stulto is, however, opinionated with
respect to its configuration of many VTE settings.

'Stulto' is Esperanto for 'foolishness.'

Tentative Roadmap
-----------------

Stulto is currently in early development; as such, the feature roadmap is
tentative and may change drastically until the design is stabilized. This is
primarily a pedagogical project for the primary maintainer, but the guiding
principle of feature development is for Stulto to become usable as a primary
terminal emulator for most CLI-/TUI-driven workflows.

### Planned Features

Confirmed and prioritized for upcoming development

* GTK CSDs
  * May be implemented as optional
* Terminal profiles per tab

### Long-term Features

These are confirmed for development, but require further design and
architectural planning to implement.

* Port to GTK4
* Tilix-/terminator-like terminal tiling

### Nice-to-Haves

These are not confirmed, but may be implemented in the long term after prior
prioritized features have been completed.

* Configuration via DConf
* User-configurable tab labels
* Integrated/(semi-)seamless tab pane a la `kermit`/`urxvt-tabbed`
* User-customizable tab labeling
* Feature toggling at build time

### Not Planned

Features from other terminal emulators that will explicitly not be implemented
in Stulto

* Detachable/re-attachable/transferable tabs
* Image/Multimedia Handling ([_the terminal is not a media
  player_](https://plato.stanford.edu/entries/category-mistakes/))

### Architectural Enhancements

Updates to the codebase for more streamlined development

These generally do not impact the user, but may be of interest to those
interested in forking the codebase and/or contributing to the project.

* Modular source architecture

Installing
----------

Make sure you have VTE with API version 2.91 installed along with headers,
`pkg-config`, `make` and a C compiler.

```sh
$ git clone 'https://github.com/marcxjo/stulto.git'
$ cd stulto
$ make release
$ sudo make prefix=/usr install
```

To build stulto on Arch Linux, you can find a PKGBUILD script in the Git
repository at https://github.com/marcxjo/PKGBUILDs.git (under the subdir
`stulto`).

Configuring
-----------

On startup Stulto will look for `$config_dir/stulto/stulto.ini` under your
config dir (unless you specify an alternative config file). For most users,
`$config_dir` is usually `$XDG_CONFIG_HOME` or `$HOME/.config`.

Copy the included example there and edit it to your heart's content.

License
-------

Stulto is licensed under the GNU GPL v3.0.

Thanks
----------------------

* [stupidterm](https://github.com/esmil/stupidterm.git)
* [kermit](https://github.com/orhun/kermit.git)
* [tilix](https://github.com/gnunn1/tilix.git)
