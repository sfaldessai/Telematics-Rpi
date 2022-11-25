#!/bin/bash

for sysdevpath in $(find /sys/bus/usb/devices/usb*/ -name dev); do
    (
    if [[ "$sysdevpath" == *"tty"* ]]; then
        syspath="${sysdevpath%/dev}"
        devname="$(udevadm info -q name -p $syspath)"
        [[ "$devname" == "bus/"* ]] && exit
        eval "$(udevadm info -q property --export -p $syspath)"
        [[ -z "$ID_SERIAL" ]] && exit
	if [[ "$ID_SERIAL" == *"$1"* ]]
	then
		echo "/dev/$devname"
	fi
    fi
    )
done