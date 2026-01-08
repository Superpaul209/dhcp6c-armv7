# Configure DHCP6c for FreeBSD ARMv7 (pfSense / OpnSense / other)

In this github, you will find different files to compile dhcp6c by yourself and learn more how to do it.
The main objective of this repository is to create a new version of the dhcp6 client to be able to use raw-options and fix main bugs. This package is used on pfSense and isn't available for armv7 arch, if you have a SG-3100, you will need to compile or download the binary in this repository. All the updated source code is fetched from https://github.com/opnsense/dhcp6c.

You will also be able to compile any package on FreeBSD for armv7 with this tutorial.

# Step 1 : Install a FreeBSD VM

First download your favorite software to make a new FreeBSD VM, install it and download the FreeBSD image here :
https://www.freebsd.org/where/
It is very important that you take the version of the target OS if you want to build packages for pfSense. You must match the major release of the pfSense for your packages. Your host must be more recent than your jail.
Main = CURRENT
Stable/xx = STABLE

You will need a high amount of disk space, its recommended to put a minimum of 60 GB HD/SSD.
Install this new VM with the FreeBSD installer and make the default conf like your account and IP.

Enable SSH by doing those steps (if not already done) :
Add line in `/etc/rc.conf` : `sshd_enable="YES"`
Run: 
```
service sshd start
```

# Step 2 : Install packages and setup the OS

```
pkg install bash vim nano git poudriere rsync nginx unbound screen the_silver_searcher htop tmux qemu-user-static subversion bash-completion gnupg coreutils libpcap gmake gsed pkgconf cmake pkgconf sudo
```
Some packages are recommended but not required like : nano vim screen htop tmux

Add your account in `/usr/local/etc/sudoers` if you use a different account other than root.
```
sudo visudo
```
Enable qemu-user-static in `/etc/rc.conf`: `qemu_user_static_enable="YES"`

Run the startup script to configure your system for building different architectures: `/usr/local/etc/rc.d/qemu_user_static start`

# Step 3 : Configure Poudriere

Poudriere will be used to make an armv7 jail (container) for all packages. We will add a default config inside the folder `/usr/local/etc`
Modify `poudriere.conf` and add all lines below :

```
NO_ZFS=yes
FREEBSD_HOST=http://ftp.freebsd.org/pub/FreeBSD/
BASEFS=/usr/local/poudriere
POUDRIERE_DATA=${BASEFS}/data
NOLINUX=yes
USE_COLORS=yes
PRESERVE_TIMESTAMP=yes
BUILDER_HOSTNAME=build
DISTFILES_CACHE=/usr/ports/distfiles
PARALLEL_JOBS=8
ALLOW_MAKE_JOBS=yes
FREEBSD_GIT_BRANCH=main
```

Create directories with commands below : 
```
sudo mkdir -p /usr/local/poudriere/data
sudo mkdir -p /usr/ports/distfiles
```
This data folder is used by poudriere to store all logs, built packages and more.
Edit the `FREEBSD_HOST` by your favorite location near you. Edit `FREEBSD_GIT_BRANCH` with the version you are building either main (CURRENT) or stable.

Add the following code inside `/usr/local/etc/poudriere.d` inside the file `make.conf` :
```
ALLOW_UNSUPPORTED_SYSTEM=yes
MAKE_JOBS_UNSAFE=yes
WITH_DEBUG=yes
.if empty(EXTRA_PATCHES:M/local-patches/)
EXTRA_PATCHES+= /local-patches/
.endif
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

You will need to create/update the `/usr/src` folder with these commands. First it will downloads the latest sources from GitHub and then compile a build environment in armv7. 

All sources are available on this page : https://cgit.freebsd.org/src
If the target OS that you want to build is a release, choose `stable/<VERSION>`. If the target OS is a "CURRENT" build, you will have to choose the `main` branch.

For other branches, you will have to use a different command to specify the branch :
```
git clone -b stable/<VERSION> --depth 1 https://git.freebsd.org/src.git  /usr/src
```

The last command `make buildworld TARGET_ARCH=armv7` will takes a few hours to complete !

For a "CURRENT" kernel build :
```
sudo rm -rf /usr/src
git clone https://git.freebsd.org/src.git /usr/src
cd /usr
export BASEDIR=$(pwd)
export MAKEOBJDIRPREFIX=$BASEDIR/obj
cd $BASEDIR/src
make buildworld TARGET_ARCH=armv7
```

Then the new poudriere jail command will be used with our updated local sources (make sure your buildworld command is done and successful first) :

/!\ This command may takes a few hours to complete depending of the hardware you have put to your server /!\

```
sudo poudriere jail -c -j freebsd-CURRENT-armv7 -a arm.armv7 -m src=/usr/src
```

You can view all the jails that are available with this command :
```
poudriere jail -l
```

All commands for jails are available under `poudriere jail`

# Step 5 : Configure default ports of FreeBSD for your jail

FreeBSD ports will be used as "references" for the packages we want to modify.

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
After everything is done, do this command to clean the `work` folder. Please be careful to backup files !
```
make clean
```

Before copying patches files in the folder, we need to do a testport to compile pkg for the jail.
```
mkdir /usr/ports
mkdir /usr/ports/distfiles
poudriere testport -j freebsd-CURRENT-armv7 -vv net/dhcp6
```

Then, copy all the patches inside the folder `/usr/local/etc/poudriere.d/local-patches`

IMPORTANT !! Your new source code must be compared with the original files from the default ports tree. Your code can't be compared with a different initial package. Doing that will gives you errors that your patches can't apply.

# Step 7 : Build the new package in your poudriere jail

Your code is ready to be built and patched to create a new package !
Run this command to ouput all debug and test if the compile is successful or wrong :
```
sudo poudriere testport -j freebsd-CURRENT-armv7 -vv net/dhcp6
```
Please fix all errors that may occur if you have any. All errors are available in the folder `/usr/local/poudriere/data/logs/bulk/latest-per-pkg`

If the testport is done successfully, you can now build the new package :
```
sudo poudriere bulk -j freebsd-CURRENT-armv7 -vv net/dhcp6
```
The package will be available here : `/usr/local/poudriere/data/packages`

You may have errors at the step "package", if your code doesn't include all the files for dhcp6 (like this example), you will need to edit the `Makefile` and `pkg-plist` files.

Edit this line in `/usr/local/poudriere/ports/default/net/dhcp6/Makefile` :
```
USE_RC_SUBR=	dhcp6c
```
Remove `dhcp6relay` and `dhcp6s`

Inside the `/usr/local/poudriere/ports/default/net/dhcp6/pkg-plist` file :

```
etc/dhcp6c.conf.sample
sbin/dhcp6c
man/man5/dhcp6c.conf.5.gz
man/man8/dhcp6c.8.gz
@owner root
```
Remove all references about other components except dhcp6c.

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

The official repository for the latest dhcp6c source code : https://github.com/opnsense/dhcp6c \
Poudriere jail commands : https://www.gsp.com/cgi-bin/man.cgi?topic=POUDRIERE-JAIL \
How to use gdb : https://darkdust.net/files/GDB%20Cheat%20Sheet.pdf \
Outdated wiki how to build on armv7 : https://github.com/rickyzhang82/FreeBSDWiki \
Poudriere hooks : https://dan.langille.org/2019/08/10/poudriere-hooks/ \
DHCP6c on aarch64 : https://ftp.yrzr.tk/opnsense/

Thanks for reading, I hope that you like this tutorial !
