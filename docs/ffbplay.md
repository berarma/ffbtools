# ffbplay

Manage and play FFB effects from the console for testing purposes.

Usage: `bin/ffbplay -d <device> [-i] [-t] [file]`

There are two possible ways to use this tool. The interactive mode, invoked
with the `-i` option, and the replay mode, invoked when passing a FFB log file.

## Interactive mode

Use the menus to manage the effects in real-time.

## Replay mode

It will play the effects from the provided log file. There are samples in the
`tests` directory. More samples can be written using a text editor.

Use only with log files produced with a current version of the code. Log files
produced with older versions may not work.

Use the `-t` option for tracing the log lines as they're read.

