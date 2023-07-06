#!/bin/bash

# Power - PH.06

chmod 777 /sys/class/gpio/export
chmod 777 /sys/class/gpio/unexport

if [ -e /sys/class/gpio/PH.06 ]; then
	echo "PH.06 has exported."
else
	echo 397 > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/PH.06/direction

echo 1 > /sys/class/gpio/PH.06/value

sleep 1
