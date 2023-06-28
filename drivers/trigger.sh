#!/bin/bash

# GPIO06 - MAX1/MPF0(MAX9296) - PAA.04
# GPIO10 - MAX1/MPF5(MAX9296) - PBB.02

chmod 777 /sys/class/gpio/export
chmod 777 /sys/class/gpio/unexport

if [ -e /sys/class/gpio/PAA.04 ]; then
	echo "PAA.04 has exported."
else
	echo 320 > /sys/class/gpio/export
fi

if [ -e /sys/class/gpio/PBB.02 ]; then
	echo "PBB.02 has exported."
else
	echo 326 > /sys/class/gpio/export
fi

if [ -e /sys/class/gpio/PCC.00 ]; then
	echo "PCC.00 has exported."
else
	echo 328 > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/PAA.04/direction
echo out > /sys/class/gpio/PBB.02/direction
echo out > /sys/class/gpio/PCC.00/direction

while [ true ] ; do
	echo 1 > /sys/class/gpio/PAA.04/value
	echo 1 > /sys/class/gpio/PBB.02/value
	echo 1 > /sys/class/gpio/PCC.00/value
	sleep 0.03333
	echo 0 > /sys/class/gpio/PAA.04/value
	echo 0 > /sys/class/gpio/PBB.02/value
	echo 0 > /sys/class/gpio/PCC.00/value
	sleep 0.03333
done
