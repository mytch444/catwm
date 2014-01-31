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
 * 29.01.14 -> Removed floating as I have yet to actually use it.
 * 11.01.14 -> Added Xinerama support, floating windows and some other things.
 * 07.01.14 -> Added key strings and border space.

catwm
 
 * 05.07.19 -> v0.3. Multiple desktops and correct some bugs // is it ment to be 19?
 * 30.06.10 -> v0.2. Back again \o/
 * 15.03.10 -> v0.2. The wm is functional -> I only use this wm!
 * 24.02.10 -> First release, v0.1. In this release 0.1, the wm is almost functional

Modes
-----

The default is the normal mode which has one window as master and the rest to the right
of it (like below). The with of the master area can be changed and is only for that 
desktop (so different desktops have different widths).

    --------------
    |        |___|
    |        |___|
    | Master |___|
    |        |___|
    |        |___|
    --------------

and fullscreen mode which has each window taking up the full screen and can be cycled
through the same way as changing windows on tiling ("C-p o" but default).

Installation
------------

You will need Xlib and <https://github.com/mytch444/showmessage> (Or disable messages),
then:

    $ vim config.h # And change to your liking.
    $ make
    $ make install
    $ make clean

Bugs
----
 * make doesn't recompile when config.h (or any other file) is changed without cleaning 
   first. Not major but annoying.

Todo
----
 * Find a new name? So far I have come up with idlcwm (I don't like catwm). Thats about the best.
   So this could take some time to fix.
 * Add another tiling mode that works more like emacs. I like emacs.
 
If you have some particular request, just send me an e-mail, and I will see for it!
