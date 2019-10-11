# FfbFixUpdate.sh

Using the script to run a command and work around failed update FFB calls:
  ```bin/FfbFixUpdate.sh <device> -- <command>```

Arguments:

 - `<device>`: Which device to intercept calls for. It has to be an event device in
   the `/dev/input/` tree.
 - `<command>`: Command that runs the application we want to try the fix.

To use it on Steam games go to game properties, click "Set launch options", enter this command:
  ```/home/user/ffbtools/bin/FfbFixUpdate.sh <device> [<logprefix>] -- %command%```

Thanks to the [user jdinalt for the debugging and idea](https://github.com/ValveSoftware/Proton/issues/2366#issuecomment-528539637).
