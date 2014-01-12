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

 * 11.01.14 -> Added Xinerama support, floating windows and some other things.
 * 07.01.14 -> Added key strings and border space.

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

fullscreen mode and floating.

Installation
------------

Need Xlib and <https://github.com/mytch444/showmessage>, then:

  $ emacs config.h
  $ make
  $ make install
  $ make clean

Bugs
----
 * Some programs seem to crash X when they are closed on one computer while working fine
   on another. Only thing that I can see that could cause it is different graphics cards
   or 32 vs 64 bit. The programs are chromium and lxappearance that I have noted. I have
   a fix that helps with some of it but I'm pretty sure its just covering up the problem.
 * make doesn't recompile when config.h is changed. No major but annoying.

Todo
----
 * Find a new name?
 * Add another tiling mode that works more like emacs. I like emacs.
 
If you have some particular request, just send me an e-mail, and I will see for it!

