# FfbFixUpdate.sh

Using the script to run a command and work around failed update FFB calls:
  ```bin/FfbFixUpdate.sh <device> -- <command>```

Arguments:

 - `<device>`: Which device to intercept calls for. It has to be an event device in
   the `/dev/input/` tree.
 - `<command>`: Command that runs the application we want to try the fix.

