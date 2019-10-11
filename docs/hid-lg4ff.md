# The Logitech FFB driver

First and foremost, we have to thank Simon Wood for the work done on this
driver. His pioneer work has brought us where we are today and now we can talk
about improving on his work.

Logitech has released official documentation for their wheels making it
possible to fully support their features. The current Linux driver
(`hid-lg4ff`) doesn't support some of these features for reasons still unclear
to me, and that's most probably because of my lack of better knowledge. That's
why I'm trying to expand it and help others do the same.

I'd be more than pleased to be corrected on any information written here.

Some of the limitations imposed by the driver, not the hardware are:

 - Only one effect type available (constant force) while the hardware supports a
   good number of effects, including periodic and conditional.
 - Only one type of effect can be played simultaneously while the hardware
   supports 4 different effects playing at the same time. This limitation isn't
   really important for now because of the previous one.

The `hid-lg4ff` driver has been implemented as an extension to the `ff-memless`
driver.  This driver seems to be written for joysticks that typically support
constant force and the rumble effect in its simplest form. It can emulate an
effect envelope and even emulate periodic effects using the rumble effect.

But the Logitech wheels are not exactly like this. They don't support the
rumble effect, but they support a lot more effects, including periodic, and can
play up to 4 effects at the same time. These features are ignored by the
`ff-memless` base driver.

This is true for most Logitech wheels supporting FFB except the G920 driving
wheel. This device uses the HID++ protocol and an improved on-board controller
that can handle a play queue including effect envelopes by itself. Thus, this
device should be already fully supported on Linux.

To work around these limitations I think the most obvious step would be to
write a modified driver that extends on the more generic `ff-core` driver using
all features supported by the hardware. And then implement effect envelope
emulation like the `ff-memless` driver does. Maybe this should be broken down
as a `ff-no-so-memless` driver (excuse the name) and a new `hid-lg5ff` driver
that extends on it.

There might be other issues that should be addressed, like what happens when an
application sends effects faster that the device can handle, but for now I
think these are not as important.

It has been reported that the driver can stop an effect when trying to update
it. This might be a bug in the driver or maybe a consequence of using
`ff-memless`, we should figure out.

Related kernel files:

 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/input/ff-core.c
 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/input/ff-memless.c
 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/hid/hid-lg.c
 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/hid/hid-lg4ff.c
 - https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/hid/hid-logitech-hidpp.c

There's already been an attempt to improve the driver but it never got accepted
into the kernel. It has a github project:

 - https://github.com/edwin-v/linux-hid-lg4ff-next

It's similar to my proposal but not quiet the same. I have yet to fully
understand how they differ.
