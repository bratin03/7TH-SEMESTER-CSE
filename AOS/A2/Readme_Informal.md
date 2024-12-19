# Steps to insert Syscall

[Link](https://dev.to/jasper/adding-a-system-call-to-the-linux-kernel-5-8-1-in-ubuntu-20-04-lts-2ga8)

1. Unzip the Kernel Code and inside the `linux-5.10.223` directory create directory `gettaskinfo` and enter it.
```bash
mkdir gettaskinfo
cd gettaskinfo
```
2. Inside this directory, put the implementation of the code in `gettaskinfo.c` and create a `Makefile` in the same directory. The content of the Makefile:
```Make
obj-y := gettaskinfo.o
```

3. Come out of this directory and open the `Makefile` of the kernel. Search for `core-y`. In the second result, add the `gettaskinfo` directory.
```Make
core-y		+= kernel/ certs/ mm/ fs/ ipc/ security/ crypto/ block/ io_uring/ gettaskinfo/
```

4. Open the `include/linux/syscalls.h` file and add the following line at the end of it before `#endif`:
```h
asmlinkage long sys_gettaskinfo(pid_t pid, char __user *buffer);
```

5. Open the `arch/x86/enrty/syscalls/syscall_64.tbl` and add the new syscall in it at a free entry:
```tbl
441 common  gettaskinfo     sys_gettaskinfo
```
Currently added at line number 365 for SYSCALL number 441

6. Build the kernel and switch to this new version
```bash
make menuconfig
make -j4
sudo make modules_install -j4
sudo make install -j4
sudo update-grub
```

7. Use the Makefile for running the codes.


## The directory structures shoule be
```bash
bratin@Ubuntu:~/KERNEL$ ls -la
total 28
drwxrwxr-x  5 bratin bratin 4096 Sep  3 12:12 .
drwxr-x--- 21 bratin bratin 4096 Sep  3 08:29 ..
drwxrwxr-x  2 bratin bratin 4096 Sep  3 10:48 libgettaskinfo <------- (1)
drwxrwxr-x 26 bratin bratin 4096 Sep  3 12:10 linux-5.10.223 <------- (2)
-rw-rw-r--  1 bratin bratin  689 Sep  3 11:37 Makefile
-rw-rw-r--  1 bratin bratin 1366 Sep  3 12:12 Readme.md
drwxrwxr-x  2 bratin bratin 4096 Sep  3 11:33 test           <------- (3)
```
### (1)
```bash
bratin@Ubuntu:~/KERNEL/libgettaskinfo$ ls -la
total 16
drwxrwxr-x 2 bratin bratin 4096 Sep  3 10:48 .
drwxrwxr-x 5 bratin bratin 4096 Sep  3 12:12 ..
-rw-rw-r-- 1 bratin bratin 2065 Sep  3 12:06 libgettaskinfo.c
-rw-rw-r-- 1 bratin bratin  415 Sep  3 10:45 libgettaskinfo.h
```
### (2)
```bash
bratin@Ubuntu:~/KERNEL/linux-5.10.223$ ls -la
total 996
drwxrwxr-x  26 bratin bratin   4096 Sep  3 12:15 .
drwxrwxr-x   5 bratin bratin   4096 Sep  3 12:12 ..
drwxrwxr-x  26 bratin bratin   4096 Sep  3 12:10 arch
drwxrwxr-x   3 bratin bratin  12288 Sep  3 12:10 block
drwxrwxr-x   2 bratin bratin   4096 Sep  3 12:10 certs
-rw-rw-r--   1 bratin bratin  16673 Jul 27 14:10 .clang-format
-rw-rw-r--   1 bratin bratin     59 Jul 27 14:10 .cocciconfig
-rw-rw-r--   1 bratin bratin    496 Jul 27 14:10 COPYING
-rw-rw-r--   1 bratin bratin 100478 Jul 27 14:10 CREDITS
drwxrwxr-x   4 bratin bratin  36864 Sep  3 12:10 crypto
drwxrwxr-x  81 bratin bratin   4096 Jul 27 14:10 Documentation
drwxrwxr-x 140 bratin bratin   4096 Sep  3 12:10 drivers
drwxrwxr-x  79 bratin bratin  12288 Sep  3 12:10 fs
-rw-rw-r--   1 bratin bratin     71 Jul 27 14:10 .get_maintainer.ignore
drwxrwxr-x   2 bratin bratin   4096 Sep  3 12:10 gettaskinfo  <---------- (a)
-rw-rw-r--   1 bratin bratin     62 Jul 27 14:10 .gitattributes
-rw-rw-r--   1 bratin bratin   1911 Jul 27 14:10 .gitignore
drwxrwxr-x  29 bratin bratin   4096 Sep  3 12:15 include
drwxrwxr-x   2 bratin bratin   4096 Sep  3 12:10 init
drwxrwxr-x   2 bratin bratin   4096 Sep  3 12:10 io_uring
drwxrwxr-x   2 bratin bratin   4096 Sep  3 12:10 ipc
-rw-rw-r--   1 bratin bratin   1327 Jul 27 14:10 Kbuild
-rw-rw-r--   1 bratin bratin    555 Jul 27 14:10 Kconfig
drwxrwxr-x  21 bratin bratin  12288 Sep  3 12:10 kernel
drwxrwxr-x  21 bratin bratin  28672 Sep  3 12:10 lib
drwxrwxr-x   6 bratin bratin   4096 Jul 27 14:10 LICENSES
-rw-rw-r--   1 bratin bratin  18204 Jul 27 14:10 .mailmap
-rw-rw-r--   1 bratin bratin 576604 Jul 27 14:10 MAINTAINERS
-rw-rw-r--   1 bratin bratin  64983 Sep  3 09:14 Makefile
drwxrwxr-x   3 bratin bratin  16384 Sep  3 12:10 mm
drwxrwxr-x  72 bratin bratin   4096 Sep  3 12:10 net
-rw-rw-r--   1 bratin bratin    727 Jul 27 14:10 README
drwxrwxr-x  32 bratin bratin   4096 Sep  3 12:10 samples
drwxrwxr-x  17 bratin bratin   4096 Sep  3 12:15 scripts
drwxrwxr-x  13 bratin bratin   4096 Sep  3 12:10 security
drwxrwxr-x  26 bratin bratin   4096 Sep  3 12:10 sound
drwxrwxr-x  36 bratin bratin   4096 Jul 27 14:10 tools
drwxrwxr-x   3 bratin bratin   4096 Sep  3 12:10 usr
drwxrwxr-x   4 bratin bratin   4096 Sep  3 12:10 virt
```
#### (a)
```bash
bratin@Ubuntu:~/KERNEL/linux-5.10.223/gettaskinfo$ ls -la
total 16
drwxrwxr-x  2 bratin bratin 4096 Sep  3 12:10 .
drwxrwxr-x 26 bratin bratin 4096 Sep  3 12:15 ..
-rw-rw-r--  1 bratin bratin 1220 Sep  3 11:13 gettaskinfo.c
-rw-rw-r--  1 bratin bratin   22 Sep  3 09:13 Makefile
```
### (3)
```bash
bratin@Ubuntu:~/KERNEL/test$ ls -la
total 16
drwxrwxr-x 2 bratin bratin 4096 Sep  3 11:33 .
drwxrwxr-x 5 bratin bratin 4096 Sep  3 12:12 ..
-rw-rw-r-- 1 bratin bratin  989 Sep  3 12:04 raw_test.c
-rw-rw-r-- 1 bratin bratin  776 Sep  3 12:07 test_gettaskinfo.c
```
