Stulto - A Simple VTE Terminal
==============================

About
-----

Stulto is a simple wrapper around the VTE terminal emulator widget for GTK3. It
is forked from [stupidterm](https://github.com/esmil/stupidterm.git).

Stulto's distinguishing characteristics include multiple sessions (with session
state indicated in the window title rather than in visible tabs) and
configuration via a simple .ini file. Stulto is, however, opinionated with
respect to its configuration of many VTE settings.

Stulto enables GTK CSD by default. The headerbar can be disabled by setting the
environment variable STULTO_DISABLE_HEADERBAR.

To use Stulto with CSD permanently disabled, add the following to your shell
configuration (e.g., $HOME/.bashrc):

```
export STULTO_DISABLE_HEADERBAR=1
```

'Stulto' is Esperanto for 'foolishness.'

Tentative Roadmap
-----------------

Stulto's feature roadmap is tentative and subject to change until it sees public
release. Once the first release is published, the current feature set will be
considered stable, and future features will be implemented based on user input.

### Planned Features

Confirmed and prioritized for upcoming development

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

To get started, copy the included example config file and edit to your heart's
content.

License
-------

Stulto is licensed under the GNU GPL v3.0.

Thanks
----------------------

* [stupidterm](https://github.com/esmil/stupidterm.git)
* [kermit](https://github.com/orhun/kermit.git)
* [tilix](https://github.com/gnunn1/tilix.git)
