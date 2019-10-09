# FFBTools documentation

## Background

In the last few years we've seen more native driving games on GNU/Linux or
compatible via Wine/Proton. For now, Logitech wheels are the only ones with
complete support, but there are some issues and no one seems to be working on
them. What's worse, there's not much people with the knowledge to debug and fix
them.

This project would like to address the problem by providing easy tools for
creating reports and testing issues. And at the same time help new developers
understand what's going on under the hood.

## The tools

 - [bin/FfbLogIoCalls.sh](FfbLogIoCalls.md): Run an application logging the calls related to FFB to a file.
 - [bin/FfbFixUpdate.sh](FfbFixUpdate.md): Work around an issue reported on
  [ValveSoftware/Proton/issues/2366](https://github.com/ValveSoftware/Proton/issues/2366#issuecomment-539114450)
  about the FFB effects being stopped when updating them.

## Resources

Logitech FFB docs:
 - https://opensource.logitech.com/opensource/index.php/Technical_Information

Kernel related links:
 - https://www.kernel.org/doc/html/latest/input/ff.html
 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/input/ff-memless.c
 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/hid/hid-lg4ff.c
 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/hid/hid-logitech-hidpp.c
 - https://www.kernel.org/doc/Documentation/hid/hidraw.txt
 - https://github.com/edwin-v/linux-hid-lg4ff-next

KLGD (Kernel Library for Gaming Devices):
 - https://gitlab.com/madcatx/LinuxFF-KLGD
 - https://github.com/Eliasvan/Linux-Force-Feedback/wiki/Logitech-gaming-devices-using-KLGD
 - https://github.com/chrisaq/linux-klgd
 - https://github.com/edwin-v/hid-lg4ff-klgd

SDL2 related links:
 - https://wiki.libsdl.org/CategoryForceFeedback
 - https://hg.libsdl.org/SDL/file/d953f28d33e3/src/haptic/linux/SDL_syshaptic.c

Examples of use:
 - https://sourceforge.net/p/linuxconsole/code/ci/master/tree/utils/fftest.c
 - https://sourceforge.net/p/linuxconsole/code/ci/master/tree/utils/ffset.c

Others:
 - https://github.com/Eliasvan/Linux-Force-Feedback/wiki
 - https://lkml.org/lkml/2014/4/26/115
 - https://github.com/ValveSoftware/Proton/issues/2366#issuecomment-529254481
 - https://github.com/ValveSoftware/Proton/issues/2366#issuecomment-529150586
 - https://steamcommunity.com/groups/linuxff/discussions/0/405692224235574471/
 - https://forum.scssoft.com/viewtopic.php?f=109&t=249622
 - https://www.desktopsimulators.com/forum/showthread.php?tid=378
 - https://lkml.org/lkml/2014/5/21/325
