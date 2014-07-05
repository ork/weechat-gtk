weechat-glib
============

A library to interact with weechat's relay feature using glib and gio.

Usage
-----

Currently, only a test program is available giving the following output:

    -> Incoming packet
    Length: 26
    Compression: False
    Body size: 21
    Id: _pong
    str-> hello
    Remain: 0
    
    -> Incoming packet
    Length: 181
    Compression: False
    Body size: 176
    Id: (null)
    chr-> A
    Remain: 168
    int-> 123456
    Remain: 161
    int-> -123456
    Remain: 154
    lon-> 1234567890
    Remain: 140
    lon-> -1234567890
    Remain: 125
    str-> a string
    Remain: 110
    str-> 
    Remain: 103
    str-> (null)
    Remain: 96
    buf-> buffer
    Remain: 83
    buf-> (null)
    Remain: 76
    ptr-> 0x1234abcd
    Remain: 64
    ptr-> 0x0
    Remain: 59
    tim-> 1321993456
    Remain: 45
    arr -> of 2 x str: [ abc, de,]
    Remain: 22
    arr -> of 3 x int: [ 123, 456, 789,]
    Remain: 0

