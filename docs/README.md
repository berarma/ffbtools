# FFBTools documentation

## Background

In the last few years we've seen more native driving games on GNU/Linux or
compatible via Wine/Proton. For now, Logitech wheels are the only ones with
complete support, but there are some issues and no one seems to be working on
them. What's worse, there's not much people with the knowledge to debug and fix
them.

This project aims to address the problem by providing information and tools to
help improve support and documentation.

## Build

Requirements:

 - A recent kernel.
 - GNU Make.
 - GCC.
 - GCC multilib support. We build 32bits and 64bits versions of the library so
   we need compiler support for cross-compilation. This is commonly provided by
   package `gcc-multilib`.

Run `make` inside the project directory to build the tools.

## The tools

 - [ffbwrap](ffbwrap.md): Script that uses code injection via a wrapper library
   to debug FFB in applications.
 - [ffbplay](ffbplay.md): Console application to test FFB.

## Notes

 - [The Logitech FFB driver](hid-lg4ff.md).

## Resources

 - [Force feedback Linux documentation](https://www.kernel.org/doc/html/latest/input/ff.html).
 - [Force Feedback Protocol for Logitech Gaming Devices](https://opensource.logitech.com/opensource/index.php/Technical_Information).
 - [linuxconsole tools](https://github.com/flosse/linuxconsole): Useful commands to test FFB.
 - [SDL2 Force Feedback Support](https://wiki.libsdl.org/CategoryForceFeedback).
 - [Elias Vanderstuyft's documentation project](https://github.com/Eliasvan/Linux-Force-Feedback)
 - [KLGD (Kernel Library for Gaming Devices)](KLGD.md).

Others:
 - https://lkml.org/lkml/2014/4/26/115
 - https://steamcommunity.com/groups/linuxff/discussions/0/405692224235574471/
 - https://forum.scssoft.com/viewtopic.php?f=109&t=249622
 - https://www.desktopsimulators.com/forum/showthread.php?tid=378
 - https://lkml.org/lkml/2014/5/21/325
