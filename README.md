# Configure DHCP6c for FreeBSD ARMv7 (pfSense / OpnSense / other)

In this github, you will find different files to compile dhcp6c by yourself and learn more how to do it.
The main objective of this repository is to create a new version of the dhcp6 client to be able to use raw-options and fix main bugs. This package is used on pfSense and isn't available for armv7 arch, if you have a SG-3100, you will need to compile or download the binary in this repository.

You will also be able to compile any package on FreeBSD for armv7 with this tutorial.

A new compiled dhcp6c package is available to download directly in this repository to avoid doing all the steps below !

# Step 1 : Install a FreeBSD VM

First download your favorite software to make a new FreeBSD VM, install it and download the FreeBSD image here :
https://www.freebsd.org/where/
It is very important that you take the version 12.3 if you want to build packages for pfSense since they are using a FreeBSD 12.2 (in 2022-07). You must match the major release of the pfSense for your packages. Your host must be more recent than your jail.

You will need a very high amount of disk space, its recommended to put a minimum of 60 GB HD/SSD.
Install this new VM and make the default conf like your account and IP.

Enable SSH by doing those steps :
Add line in `/etc/rc.conf` : `sshd_enable="YES"`
Run: 
```
service sshd start
```

# Step 2 : Install packages

```
pkg install -y bash
pkg install -y vim-console
pkg install -y git
pkg install -y poudriere-devel
pkg install -y mkfile
pkg install -y rsync
pkg install -y nginx
pkg install -y unbound
pkg install -y screen
pkg install -y the_silver_searcher
pkg install -y htop
pkg install -y tmux
pkg install -y qemu-user-static
pkg install -y subversion
pkg install -y bash-completion
pkg install -y gnupg
pkg install -y coreutils
pkg install -y libpcap
pkg install -y gmake
pkg install -y gsed
pkg install -y pkgconf
pkg install -y cmake
pkg install -y pkgconf
pkg install -y sudo
```

Add your account in `/usr/local/etc/sudoers` if you use a different account other than root.

# Step 3 : Configure Poudriere

Poudriere will be used to make an armv7 jail (container) for all packages. We will add a default config inside the folder `/usr/local/etc`
Modify poudriere.conf and add all lines below :

```
NO_ZFS=yes
FREEBSD_HOST=ftp://ftp.fr.freebsd.org/pub/FreeBSD/
BASEFS=/usr/local/poudriere
SVN_HOST=svnmir.bme.freebsd.org
POUDRIERE_DATA=${BASEFS}/data
NOLINUX=yes
USE_COLORS=yes
PRESERVE_TIMESTAMP=yes
BUILDER_HOSTNAME=build
DISTFILES_CACHE=/usr/ports/distfiles
PARALLEL_JOBS=8
ALLOW_MAKE_JOBS=yes
```

Do this command : 
```
sudo mkdir -p /usr/local/poudriere/data
```
This data folder is used by poudriere to store all logs, built packages and more.
Edit the `SVN_HOST` and `FREEBSD_HOST` by your favorite hosts near you.

Add the following code inside `/usr/local/etc/poudriere.d` inside the file `make.conf` :
```
ALLOW_UNSUPPORTED_SYSTEM=yes
MAKE_JOBS_UNSAFE=yes
WITH_DEBUG=yes
EXTRA_PATCHES+= /local-patches/
```
`WITH_DEBUG` option is optional, it will be used to track code errors later in the package `gdb`

Do this command : 
```
sudo mkdir -p /usr/local/etc/poudriere.d/local-patches/
```

Add the following code inside `/usr/local/etc/poudriere.d/hooks` inside the file `jail.sh` :

```
#!/bin/sh
# /usr/local/etc/poudriere.d/hooks/jail.sh
 
status="$1"
 
# NOTE: mount is invoked before start: re https://github.com/freebsd/poudriere/wiki/hooks
if [ "$status" = "mount" ]; then
  mntpath="$2"
 
  # The local-patches directory is created only if it does not already exist.
  # If it does not already exist, it means we are doing this on the master jail
  if [ ! -d "${mntpath}/local-patches" ]; then
    /bin/mkdir "${mntpath}/local-patches"
  fi
 
  # mount our patches to that location
  /sbin/mount -t nullfs /usr/local/etc/poudriere.d/local-patches "${mntpath}/local-patches"
fi
 
exit 0
```

# Step 4 : Create a poudriere jail

Its time to build your jail in the target arch (armv7) and FreeBSD version :
```
sudo poudriere jails -c -j freebsd-12-2-armv7 -a arm.armv7 -m svn -v release/12.2.0
```
IMPORTANT : This command will run for a long time !!

If you want to build a more recent version, check the SVN FreeBSD website and explore to find if your build is available. Modify the command with the version you want.

You can view all the jails that are available with this command :
```
poudriere jail -l
```
All commands for jails are available under `poudriere jail`

# Step 5 : Configure default ports of FreeBSD for your jail

FreeBSD ports will be used as "references" for the packages we want to modify.
Do this command to update all ports of your host FreeBSD :
```
sudo portsnap auto
```

This command below will create a "default" ports folder for your jails :
```
sudo poudriere ports -c
```
All commands for jails are available under `poudriere ports`

# Step 6 : Edit the source code and prepare the build

After all the steps above are done successfully, you will be able to build packages.
All the packages are available inside the folder `/usr/local/poudriere/ports/default` if you have created a "default" ports tree.

For example, we will modify the source code of the dhcp6 package. We need to enter the folder `/usr/local/poudriere/ports/default/net/dhcp6`.
You will find many files that are used to build the package.

Extract the source code of the default package with this command :
```
make extract
```
A new folder will be created `work` with all the source code inside.

If you have edited the code of the file `dhcp6c.c` with new code, you will need to rename the original file to `dhcp6c.c.orig` and then apply your new code inside the file `dhcp6c.c`. Poudriere will compare the original file with the new file and create a diff that will be used to patch the package.

Do this step for all the new files that you have edited.
After everything is done, do this command to create patches :
```
make makepatch
```
All differences will be written in the folder `files` in a `patch-<FILE>` file.
After everything is done, do this command to clean the `work` folder. Please be careful to backup files.
```
make clean
```

Copy all the patches inside the folder `/usr/local/etc/poudriere.d/local-patches`

IMPORTANT !! Your new source code must be compared with the original files from the default ports tree. Your code can't be compared with a different initial package. Doing that will gives you errors that your patches can't apply.

# Step 7 : Build the new package in your poudriere jail

Your code is ready to be built and patched to create a new package !
Run this command to ouput all debug and test if the compile is successful or wrong :
```
sudo poudriere testport -j freebsd-12-2-armv7 -vv net/dhcp6
```
Please fix all errors that may occur if you have any. All errors are available in the folder `/usr/local/poudriere/data/logs/bulk/latest-per-pkg`

If the testport is done successfully, you can now build the new package :
```
sudo poudriere bulk -j freebsd-12-2-armv7 -vv net/dhcp6
```
The package will be available here : `/usr/local/poudriere/data/packages`

You may have errors at the step "package", if your code doesn't include all the files for dhcp6, you may need to edit what you want to build in the `Makefile` and `pkg-plist`. For example in my repository, I have done only the client and not the server.
Edit this line :
```
USE_RC_SUBR=	dhcp6c dhcp6relay dhcp6s
```
Remove `dhcp6relay` and `dhcp6s` if you dont want them.
```
etc/dhcp6c.conf.sample
sbin/dhcp6c
man/man5/dhcp6c.conf.5.gz
man/man8/dhcp6c.8.gz
@owner root
```

# Step 8 : Test your new package

Connect to your pfSense or any OS that you have built it for. For my example, its a pfSense SG-3100 running a FreeBSD 12.2 armv7.
Im sending the .pkg file in my pfsense and run the command `pkg add -f <FILE>.pkg`.

# (Optional) Step 9 : Debug any bad packages

If your new package doesn't run and get errors in the OS, you can install the following package :
```
pkg install -y gdb
```
Run your package like `/usr/local/sbin/dhcp6c -n -c /var/etc/dhcp6c.conf -p /var/run/dhcp6c.pid mvneta2.832` if its dhcp6c. A core dump will appear in the folder.
With this core dump, you will able to find errors in your code.

# Step 10 : Useful links

Poudriere jail commands : https://www.gsp.com/cgi-bin/man.cgi?topic=POUDRIERE-JAIL \
How to use gdb : https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf \
Outdated wiki how to build on armv7 : https://github.com/rickyzhang82/FreeBSDWiki \
Poudriere hooks : https://dan.langille.org/2019/08/10/poudriere-hooks/ \
DHCP6c on aarch64 : https://ftp.yrzr.tk/opnsense/

Thanks for reading, I hope that you like this tutorial !
