# pintos
Pintos is a simple operating system framework for the 80x86 architecture. It support kernel threads, loading and running user programs, and a file system. To learn more refer to the [manual](http://web.stanford.edu/class/cs140/projects/pintos/pintos.html).

## How to install and run pintos?
Clone this repo

We will use `qemu` to run pintos. To begin with, install qemu on your system:

### Install QEMU
For ubuntu: `sudo apt-get install qemu-system`

For others, follow the instructions [here](https://www.qemu.org/download/).

After installing qemu create a symlink for the command qemu: \
`sudo ln -s /bin/qemu-system-i386 /bin/qemu` \
or \
`sudo ln -s /bin/qemu-system-x86_64 /bin/qemu`

This is because, qemu doesn't come as a standalone command, rather as architecture specific commands such as `qemu-system-x86_64` or `qemu-system-arm`. This command creates a symlink for our desired qemu architecture.

### Patch Scripts
1. patch src/utils/pintos-gdb

```diff
-GDBMACROS=/usr/class/cs140/pintos/pintos/src/misc/gdb-macros
+GDBMACROS=<path to pintos>/src/misc/gdb-macros
```
2. patch src/utils/pintos

```diff
-    $sim = "bochs" if !defined $sim;   <<< if using simulator-bochs
+    $sim = "qemu" if !defined $sim;    <<< if using simulator-qemu
  
-	my $name = find_file ('kernel.bin');
+	my $name = find_file ('<path to pintos>/src/threads/build/kernel.bin');
```
3. patch src/utils/Pintos.pm
```diff
-    $name = find_file ("loader.bin") if !defined $name;
+    $name = find_file ("<path to pintos>/src/threads/build/loader.bin") if !defined $name;
```

### Patch Tools
```bash
$ cd src/utils
$ make
```
If running the above command returns the following error:
```
squish-pty.c:10:21: fatal error: stropts.h: No such file or directory
#include <stropts.h>
```
Then patch the following files as described:
1. Patch src/utils/squish-pty.c
```diff
+/*
 #include <stropts.h>
+*/

   /* System V implementations need STREAMS configuration for the
      slave. */
+  /*
   if (isastream (slave))
     {
       if (ioctl (slave, I_PUSH, "ptem") < 0
           || ioctl (slave, I_PUSH, "ldterm") < 0)
         fail_io ("ioctl");
     }
+  */
```

2. Patch src/utils/squish-unix.c  
```diff
+/*
 #include <stropts.h>
+*/
```

### Make Tools
```bash
$ cd src/utils
$ make
$ export PATH=$PATH:<path to pintos>/src/utils
```
**Note:** exporting the path variable only changes it for the current terminal session. To make it uniform you can modify the `.bashrc` file.

### Make pintos Kernel
**Note:** Please sure the dir 'utils' have been set in $PATH

* Patch src/threads/Make.vars
```diff
-SIMULATOR = --bochs   <<< if using simulator-bochs
+SIMULATOR = --qemu    <<< if using simulator-qemu
```

Now compile the pintos kernel:
```bash
$ cd src/threads
$ make
```

### Run
**Note:** Please sure the dir 'utils' have been set in $PATH

```bash
$ pintos -h
```
It returns:
```bash
Command line syntax: [OPTION...] [ACTION...]
Options must precede actions.
Actions are executed in the order specified.

Available actions:
  run TEST           Run TEST.

Options:
  -h                 Print this help message and power off.
  -q                 Power off VM after actions or on panic.
  -r                 Reboot after actions.
  -rs=SEED           Set random number seed to SEED.
  -mlfqs             Use multi-level feedback queue scheduler.
```

```bash
$ pintos run alarm-multiple
```

This command passes the arguments 'run alarm-multiple' to the pintos kernel. In these arguments, run instructs the kernel to run a test and alarm-multiple is the test to run.

You can use the -v option to disable X output, -q let simulator auto-quit:
```bash
$ pintos -v -- run alarm-multiple
```
## Tests
Make sure that the dir 'utils' have been set in $PATH

To run the tests run the following commands:
```bash
$ cd src/threads/build
$ make check
```
For a more detailed info:
```bash
make check VERBOSE=1
```

You can also run specific checks as follows:
```bash
$ make tests/threads/alarm-multiple.result
$ cat tests/threads/alarm-multiple.result
PASS                # (if the test has passed)
```

