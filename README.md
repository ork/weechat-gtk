weechat-glib
============

A library to interact with weechat's relay feature using glib and gio.

Features
--------

  * High-level commands (see methods)
  * Low-level commands (send a line)
  * zlib decompression
  * GTK test client

![screenshot](http://i.imgur.com/1igxlcf.png)

Types
-----

Currently, the following weechat types are implemented:

  * `arr` : Array of objects
  * `chr` : Signed char
  * `hda` : Hdata content
  * `htb` : Hastable
  * `inf` : Info
  * `inl` : Infolist content
  * `int` : Signed int
  * `lon` : Signed long
  * `ptr` : Pointer
  * `str` : String
  * `tim` : Time

Methods
-------

Currently, the following weechat methods are implemented:

  * hdata
  * info
  * infolist
  * init
  * input
  * nicklist
  * ping
  * quit
  * test

Work in progress:

  * desync
  * sync

References
----------

  * [WeeChat Relay protocol](http://www.weechat.org/files/doc/devel/weechat_relay_protocol.en.html)
  * [GLib](https://developer.gnome.org/glib/stable/)
  * [GIO](https://developer.gnome.org/gio/stable/)

