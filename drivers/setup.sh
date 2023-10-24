#!/bin/bash

echo "[module]: insmod gmsl2_sensors.ko"
if [ "`sudo lsmod | grep gmsl2_sensors`" == "" ]; then
    sudo insmod gmsl2_sensors.ko
fi

sleep 0.5

# Power - PH.06
chmod 777 /sys/class/gpio/export
chmod 777 /sys/class/gpio/unexport

echo "[PWDN]: power on"
if [ -e /sys/class/gpio/PH.06 ]; then
	echo "PH.06 has exported."
else
	echo 397 > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/PH.06/direction

echo 1 > /sys/class/gpio/PH.06/value

sleep 0.5

echo "[SYS]: power up"
if [ -e /sys/class/gpio/PAC.07 ]; then
	echo "PAC.07 has exported."
else
	echo 493 > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/PAC.07/direction

echo 0 > /sys/class/gpio/PAC.07/value
