Stulto - A Simple VTE Terminal
==============================

No longer maintained
---

Stulto was a simple platform to enable learning about C, GTK, and VTE in
particular. It has a lot of design issues that, for an app this simple, are
simultaneously not worth the trouble and almost too simple to bother with. I
intend to start fresh with a GTK4 app that builds on the intent behind Stulto,
with additional features and a workflow more suited to users who enjoy the
feature set of VTE but seek a more flexible and slimmed-down experience
than the current popular offerings.

About
-----

Stulto is a simple wrapper around the VTE terminal emulator widget for GTK3. It
is forked from [stupidterm](https://github.com/esmil/stupidterm.git).

Stulto's distinguishing characteristics include multiple sessions (with session
state indicated in the window title rather than in visible tabs) and
configuration via a simple .ini file. Stulto is, however, opinionated with
respect to its configuration of many VTE settings.

Stulto enables GTK CSD by default. The headerbar can be disabled by setting the
environment variable `STULTO_DISABLE_HEADERBAR`.

To use Stulto with CSD permanently disabled, add the following to your shell
configuration (e.g., $HOME/.bashrc):

```
export STULTO_DISABLE_HEADERBAR=1
```

'Stulto' is Esperanto for 'foolishness.'

Keybindings
-----------

Stulto supports the following keybindings:

| Key Combination | Action                           |
| --------------- |--------------------------------- |
| Ctrl+Shift+-    | Decrease terminal font size      |
| Ctrl+Shift+=    | Increase terminal font size      |
| Ctrl+Shift+0    | Reset terminal font size         |
| Ctrl+Shift+c    | Copy selected text               |
| Ctrl+Shift+p    | Paste at cursor position         |
| Ctrl+Shift+t    | Add terminal session             |
| Ctrl+Shift+PgUp | Select previous terminal session |
| Ctrl+Shift+PgDn | Select next terminal session     |

In CSD mode, Stulto provides a toolbar with buttons for adding and navigating
between terminal sessions.

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

* User-configurable tab labels
* Integrated/(semi-)seamless tab pane a la `kermit`/`urxvt-tabbed`
* User-customizable tab labeling
* Feature toggling at build time

Installing
----------

Make sure you have VTE with API version 2.91 installed along with headers,
`pkg-config`, `meson`, `ninja`, and a C compiler.

```sh
$ git clone 'https://github.com/marcxjo/stulto.git'
$ cd stulto
$ meson setup build
$ meson install -C build
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

Development
-----------

Stulto is currently a one-dev project, but contributions are welcome and
encouraged (though note that not all features are guaranteed to be
accepted for adoption). Despite that Stulto is not currently a
collaborative project, pull requests used extensively to signal breaking
or otherwise significant changes and document the reasoning for the
benefit of interested users or contributors.

License
-------

Stulto is licensed under the GNU GPL v3.0.

Thanks
----------------------

* [stupidterm](https://github.com/esmil/stupidterm.git)
* [kermit](https://github.com/orhun/kermit.git)
* [tilix](https://github.com/gnunn1/tilix.git)
