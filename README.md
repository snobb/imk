IMK
============
Simple file watcher similar to fswatch or inotify-wait that works both in linux and *BSD.

Install
-------
```bash
make release && sudo make install clean
```

to build and install a statically linked version:
```bash
make static && sudo make install clean
```

To build a small static binary, the musl libc can be used as follows:
```bash
$ make static CC=musl-gcc && sudo make install clean
rm -f *.core
rm -f build_host.h
rm -f imk
rm -rf ./obj
musl-gcc -D_DEFAULT_SOURCE -Werror -Wall  -std=c99 -pedantic -static -O3 -o obj/files.o -c files.c
musl-gcc -D_DEFAULT_SOURCE -Werror -Wall  -std=c99 -pedantic -static -O3 -o obj/log.o -c log.c
musl-gcc -D_DEFAULT_SOURCE -Werror -Wall  -std=c99 -pedantic -static -O3 -o obj/main.o -c main.c
musl-gcc -D_DEFAULT_SOURCE -Werror -Wall  -std=c99 -pedantic -static -O3 -o obj/cmd.o -c cmd.c
musl-gcc -D_DEFAULT_SOURCE -Werror -Wall  -std=c99 -pedantic -static -O3 -o obj/cfg.o -c cfg.c
musl-gcc -D_DEFAULT_SOURCE -Werror -Wall  -std=c99 -pedantic -static -O3 -o obj/poll_linux.o -c compat/poll_linux.c
musl-gcc -static -o imk obj/files.o obj/log.o obj/main.o obj/cmd.o obj/cfg.o obj/poll_linux.o
strip imk
install -o root -g root -m 755 imk /usr/local/bin/
rm -f *.core
rm -f build_host.h
rm -f imk
rm -rf ./obj

$ ls -alh /usr/local/bin/imk
-rwxr-xr-x 1 root root 70K May 18 16:49 /usr/local/bin/imk

$ file /usr/local/bin/imk
/usr/local/bin/imk: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, stripped
```

Usage
-----
```bash
$ ./imk -h
usage: imk -c <command> [options] <file/dir ...>

   The options are as follows:
      -c <cmd>   - command to execute when event is triggered
      -d <cmd>   - teardown command to execute when -k timeout occurs (assumes -w)
      -h         - display this text and exit
      -k <ms>    - timeout after which to kill the command subproces (default - do not kill. Assumes -w.)
      -o         - exit after the first iteration
      -r         - if a directory is supplied, add all its sub-directories as well
      -s         - do not run the command inside a shell (eg. /bin/sh -c <cmd>)
      -S <ms>    - number of ms to sleep before reattaching in case of DELETE event (default 300)
      -t <sec>   - number of seconds to skip after the last executed command (default 0)
      -v         - display the version [<version>]
      -w         - spawn a subprocess for command.
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
