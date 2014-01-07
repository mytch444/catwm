CATWM
=====

     /\___/\
    ( o   o )  Made by cat...
    (  =^=  )
    (        )            ... for cat!
    (         )
    (          ))))))________________ Cute And Tiny Window Manager

Summary
-------

catwm is a very simple and lightweight tiling window manager.
This fork currently allows for space between windows (like bspwm)
and strings of key combinations (C-b o) like emacs.

Status
------
This fork

 * 07.01.14 -> v0.1 Added key strings and border space.

catwm
 
 * 05.07.19 -> v0.3. Multiple desktops and correct some bugs // is it ment to be 19?
 * 30.06.10 -> v0.2. Back again \o/
 * 15.03.10 -> v0.2. The wm is functional -> I only use this wm!
 * 24.02.10 -> First release, v0.1. In this release 0.1, the wm is almost functional

Modes
-----

It allow the "normal" modes of tiling window managers:

    --------------
    |        |___|
    |        |___|
    | Master |___|
    |        |___|
    |        |___|
    --------------

and fullscreen mode

Installation
------------

Need Xlib and <https://github.com/mytch444/showmessage>, then:
    $ emacs config.h
    $ make
    $ make install
    $ make clean

Bugs
----
 * submap does not work with mod keys.

Todo
----
 * Make submap work with mod's rather than think that shift is a key.
 * Add modes (not sure what you would call them) like stumpwm for resizing master area.
 * Switch to XCB // umm

If you have some particular request, just send me an e-mail, and I will see for it!

