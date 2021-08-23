# klaculate
Software for [klaculate](http://klaculate.com/).

---
## Runtime Dependencies
* FastCGI: `pkg install fcgi-devkit` ([`www/fcgi`](https://freshports.org/www/fcgi/))

***Please Note:*** Dependencies that are part of the base OS are not listed here.

---
## Build Dependencies
* CMake: `pkg install cmake` ([`devel/cmake`](https://freshports.org/devel/cmake/))

***Please Note:*** Dependencies that are part of the base OS are not listed here.

---
## Building
After checkout, `cd` into the checked-out project folder and then simply do the following:
```sh
cmake .
make
```

To run:
```sh
./build/bin/klaculate -dp
```
---
# Command arguments:
```none
klaculate [-dpt]
-d : Don't daemonize (don't detach)
-p : Don't write pidfile
-t : Don't set signal traps (probably never use this)
```
---

## Tools:
* `clean.sh` - calls `make clean` (if needed) and then scrubs the folder.
* `debug.sh` - runs GDB.  With no arguments, runs GDB against a core dump (`klaculate.core`).  When supplied with a PID, attaches to that PID.
* `llvm-debug.sh` - runs LLDB.  With no arguments, runs LLDB against a core dump (`klaculate.core`).  When supplied with a PID, attaches to that PID.
* `ver.sh` - Used by `cmake` to make a version string.  Not meant to be used by itself.
* `updateserver.sh` - Synchronizes local stuffs with the server.
