weechat-glib
============

A library to interact with weechat's relay feature using glib and gio.

Types
-----

Currently, the following weechat types are implemented:

  * `arr` : Array of objects
  * `chr` : Signed char
  * `hda` : Hdata content
  * `inf` : Info
  * `inl` : Infolist content
  * `int` : Signed int
  * `lon` : Signed long
  * `ptr` : Pointer
  * `str` : String
  * `tim` : Time

Work in progress:

  * `htb` : Hastable

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

