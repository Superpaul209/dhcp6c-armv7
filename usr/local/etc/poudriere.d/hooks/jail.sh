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
