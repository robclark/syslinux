#!/bin/sh

# TODO probably should search for attached disks (look for partition named
# "boot"??  or how should that work..)
bootdev="/dev/sda1"

mkdir /boot
mount $bootdev /boot

/usr/bin/kexeclinux /boot/extlinux/extlinux.conf

