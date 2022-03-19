Stulto - A Terminal for Fools
=============================

About
-----

Stulto is a simple wrapper around the VTE terminal emulator widget for GTK3.

The main purpose of Stulto is to provide the developer with a simple playground in which to explore and develop GTK/VTE
features. With the exception of a few configuration settings inherited from `stupidterm`, the intent is to minimize the
number of features subject to deprecation or obsoletion once they've been added, but users should expect that features
will be added from time to time. In most cases, they will be made optional when possible.

Note that this project is not currently accepting feature requests, but will consider doing so once it reaches a more
mature state.

Tentative Roadmap
-----------------

### Planned Features
* Port to GTK4
* Setting default tab label from terminal prompt
* Optional support for client-side decorations
* Support for per-tab terminal profiles

### Architectural Enhancements
* Split out source into separate files per domain
* Add support for feature toggling at build time

### Nice-to-Haves
* Configuration via DConf
* Integrated/(semi-)seamless tab pane a la `kermit`/`urxvt-tabbed`
* User-customizable tab labeling

### Not Planned
* Detachable/re-attachable/transferable tabs
* Image/Multimedia Handling ([_the terminal is not a media player_](https://plato.stanford.edu/entries/category-mistakes/))

Installing
----------

Make sure you have VTE with API version 2.91 installed
along with headers, `pkg-config`, `make` and a C compiler.

```sh
$ git clone 'https://github.com/marcxjo/stulto.git'
$ cd stulto
$ make release
$ sudo make prefix=/usr install
```

To build stulto on Arch Linux, you can find a PKGBUILD script in the Git repository at
https://github.com/marcxjo/PKGBUILDs.git (under the subdir `stulto`).

Configuring
-----------

On startup Stulto will look for `stulto/stulto.ini` under your config dir (unless you specify an alternative config
file).

Copy the included example there and edit it to your heart's content.

License
-------

Stulto is a direct fork of [stupidterm](https://github.com/esmil/stupidterm), with inspiration for enhancements
liberally lifted form [kermit](https://github.com/orhun/kermit). Credit is hence gratefully given to
[Emil Renner Berthing](https://github.com/esmil) and [Orhun Parmaksız](https://github.com/orhun) for their respective
projects.

Stulto is licensed under the GNU GPL v3.0.

About the Name/Tagline
----------------------

'Stulto' is an Esperanto word that translates to 'foolishness' in English; in other words, Stulto is so named as a
subtle homage to its predecessor.
