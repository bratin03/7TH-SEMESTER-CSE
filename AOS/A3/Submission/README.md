# AOS Assignment-3
## Custom Linux distribution for embedded systems
>	Bratin Mondal (21CS10016)  
    Datta Ksheeraj (21CS30037)

## Steps to build a Custom embedded Linux distribution for Raspberry Pi 3 using Buildroot

1. Download the latest stable Buildroot tarball from the official Buildroot website. For our project, we have used Buildroot 2024.02.6.

2. Extract the tarball and navigate to the extracted directory.
```bash
tar -xvf buildroot-2024.02.6.tar.gz
cd buildroot-2024.02.6
```

3. Configure Buildroot for Raspberry Pi 3
```bash
make raspberrypi3_defconfig
```

4. Configure Buildroot
```bash
make menuconfig
```
### a) Display names in system banner
```
System configuration --->
  System banner
```
In system banner, enter the text you want to display on the system banner.

### b) Enabling `nano` text editor
```
Target Packages --->
  Text editors and viewers --->
   [*] nano
```
Select `nano` from the list of text editors and viewers.

### c) Set root password
```
System configuration --->
  Root password
```
Set the root password for the system. We have set it to "root"

### d) Enabling SSH server capabilities
```
Target Packages --->
  Networking Applications --->
    [*] openssh --->
      [*] server
```

### e) Enabling network utilities by selecting Net-tools
```
Target Packages --->
  Networking Applications --->
    [*] net-tools
```

5. Save the configuration and exit.

6. To compile the Buildroot, run the following command:
```bash
make
```
