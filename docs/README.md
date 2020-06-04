# FFBTools documentation

When starting this project I tried to describe the state of the Logitech device
driver in [this document](hid-lg4ff.md). Now, most issues discussed in that
document have been addressed in the new driver
[new-lg4ff](https://github.com/berarma/new-lg4ff), and some were bugs in
Wine/Proton, SDL and Linux that have been already fixed.

## Build

Requirements:

 - A recent kernel.
 - GNU Make.
 - GCC.
 - GCC multilib support for libffbwrapper. We build 32bits and 64bits versions
   of the library so we need compiler support for cross-compilation. This is
   commonly provided by package `gcc-multilib`.

Run `make` inside the project directory to build the tools.

## Testing tools

 - [ffbwrap](ffbwrap.md): Script that uses code injection via a wrapper library
   to debug FFB in applications.
 - [ffbplay](ffbplay.md): Console application to test FFB.

## Other tools

 - [new-lg4ff](https://github.com/berarma/new-lg4ff): improved driver for Logitech GXX and older wheels.
 - [oversteer](https://github.com/berarma/oversteer): configuration tool for Logitech wheels.

## Resources

 - [Force feedback Linux documentation](https://www.kernel.org/doc/html/latest/input/ff.html).
 - [Force Feedback Protocol for Logitech Gaming Devices](https://opensource.logitech.com/opensource/index.php/Technical_Information).
 - [linuxconsole tools](https://github.com/flosse/linuxconsole): Useful commands to test FFB.
 - [SDL2 Force Feedback Support](https://wiki.libsdl.org/CategoryForceFeedback).
 - [Elias Vanderstuyft's documentation project](https://github.com/Eliasvan/Linux-Force-Feedback)
 - [KLGD (Kernel Library for Gaming Devices)](KLGD.md).
 - [ff-memless-next](https://gitorious.org/linux-ff-memless-next/linux-ff-memless-next).
 - [(Racing) Car Steering Forces EXPLAINED!](https://www.youtube.com/watch?v=pCq01LHaIVg).
 - https://lkml.org/lkml/2014/4/26/115
 - https://steamcommunity.com/groups/linuxff/discussions/0/405692224235574471/
 - https://forum.scssoft.com/viewtopic.php?f=109&t=249622
 - https://www.desktopsimulators.com/forum/showthread.php?tid=378
 - https://lkml.org/lkml/2014/5/21/325
