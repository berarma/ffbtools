# ffbwrap

Runs a command while tracking calls to the FFB subsystem. The calls can be
logged or modified to test applications.

Usage: `bin/ffbwrap [--logger=logfile] [--update-fix] [--direction-fix] [--features-hack] <device> -- <command>`

Arguments:

 - `<device>`: Which device to log calls for. It has to be an event device in
   the `/dev/input/` tree.
 - `<command>`: Command that runs the application we want to log.

One or more of the following options can be used:

  `--logger=<file-prefix>`: Logs all calls to a file with prefix <file-prefix>.
  A timestamp will be added to the file name.

  `--update-fix`: Works around an issue found when updating FFB effect parameters.
  This issue is reported at [ValveSoftware/Proton/issues/2366](https://github.com/ValveSoftware/Proton/issues/2366#issuecomment-539114450) by @jdinalt
  with full debug information and the workaround that we have used here.

  `--direction-fix`: Apply a fix to the direction of the effect. Effects with only
  a vertical component are ignored so this fix sets an horizontal direction to
  all effects.

  `--duration-fix`: Apply a fix to set the duration 0 to 0xFFFF for devices that
  do not interpret 0 as an infinite effect..

  `--features-hack`: Reports all effect types as supported. The effect types not
  supported by the device will later fail but it allows to log all the effects
  the application can generate.

  `--force-inversion`: Invert the force feedback forces. It can be used
  together with `--direction-fix`.

  `--ignore-set-gain`: Ignore any attempt to change the global FF gain.
  
  `--ignore-autocenter`: Ignore any attempt to change autocenter strength.

  `--offset-fix`: Proton sets `offset` and `phase` to incorrect values. This
  option fixes it.

  `--throttling`: Puts a limit to the number of effect commands that can be
  sent to avoid filling the command queue of the device. It helps with issues
  like effect lag and "full queue" messages in the log.

  `--throttling-time`: Changes the throttling timer period to some value in
  milliseconds. The default value is 3ms. Only used when enabling the
  throttling option.

## Examples

Log calls to a file:

  `ffbwrap --logger=/home/user/myapp /dev/input/by-id/usb-Logitech_G29_Driving_Force_Racing_Wheel-event-joystick -- <command>`

Apply the update fix:

  `ffbwrap --update-fix /dev/input/by-id/usb-Logitech_G29_Driving_Force_Racing_Wheel-event-joystick -- <command>`

Report all features and log calls to a file:

  `ffbwrap --logger=/home/user/myapp --features-hack /dev/input/by-id/usb-Logitech_G29_Driving_Force_Racing_Wheel-event-joystick -- <command>`

Apply the update and direction fixes:

  `ffbwrap --update-fix --direction-fix /dev/input/by-id/usb-Logitech_G29_Driving_Force_Racing_Wheel-event-joystick -- <command>`

To use it on Steam games go to game properties, click "Set launch options" and
use the same syntax replacing the <command> by `%command%`.

Example:

  `ffbwrap --update-fix /dev/input/by-id/usb-Logitech_G29_Driving_Force_Racing_Wheel-event-joystick -- %command%`

