IMK
============
Simple file watcher similar to fswatch or inotify-wait that works both in linux and *BSD.


Usage:
------
```bash
$ ./imk -h
usage: ./imk -c <command> -d <command> [-h] [-k <ms>] [-o] [-r] [-s] [-S <ms>] [-t <sec>] [-v] [-w] <file ...> <dir>

   The options are as follows:
      -c <cmd>   - command to execute when event is triggered
      -d <cmd>   - teardown command to execute when -k timeout occurs
      -h         - display this text and exit
      -k <ms>    - timeout after which to kill the command subproces (default - do not kill)
      -o         - exit after the first iteration
      -r         - if a directory is supplied, add all its sub-directories as well
      -s         - run the command inside a shell (eg. /bin/sh -c <cmd>)
      -S <ms>    - number of ms to sleep before reattaching in case of DELETE event (default 300)
      -t <sec>   - number of seconds to skip after the last executed command (default 0)
      -v         - display the version [<version>]
      -w         - do not spawn a subprocess for command (not compatible with -k and -d)
      <file/dir ...> - list of files or folders to monitor

   Please use quotes around the command and teardown command if it is composed of multiple words
```

To monitor all .c files and run make run the following:

```bash
$ ./imk -c 'make release' *.c
:: [20:00:00] start monitoring: cmd[make release] files[log.c main.c]
```

If any of the monitored files are modified, the command will be executed.

Known Issues:
-------------
 - when monitoring a directory, the command may be triggered more then once for every change of the contained files.
 - imk may not detect a change if the monitored file was edited with VIM with swapfiles enabled.
