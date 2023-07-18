#!/bin/bash

# Power - PH.03
chmod 777 /sys/class/gpio/export
chmod 777 /sys/class/gpio/unexport

echo "[sync]"
if [ -e /sys/class/gpio/PH.03 ]; then
	echo "PH.03 has exported."
else
	echo 394 > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/PH.03/direction

while [ true ]; do
	echo 1 > /sys/class/gpio/PH.03/value
	sleep 0.028
	echo 0 > /sys/class/gpio/PH.03/value
	sleep 0.016
done
