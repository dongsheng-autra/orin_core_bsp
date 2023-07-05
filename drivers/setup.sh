#!/bin/bash

echo "[module]: insmod begin"
if [ "`sudo lsmod | grep gmsl_max9295`" == "" ]; then
    sudo insmod gmsl_max9295.ko
fi

if [ "`sudo lsmod | grep gmsl_max96712`" == "" ]; then
    sudo insmod gmsl_max96712.ko
fi

if [ "`sudo lsmod | grep imx490`" == "" ]; then
    sudo insmod imx490.ko
fi
echo "[module]: insmod done"

echo "[max9295-a MFP8]: trigger"
i2ctransfer -y -f 30 w3@0x60 0x02 0xd6 0x10
sleep 0.5
i2ctransfer -y -f 30 w3@0x60 0x02 0xd6 0x00
sleep 0.5
echo "[max9295-a MFP8]: trigger done"

echo "[max9295-b MFP8]: trigger"
i2ctransfer -y -f 30 w3@0x62 0x02 0xd6 0x10
sleep 0.5
i2ctransfer -y -f 30 w3@0x62 0x02 0xd6 0x00
sleep 0.5
echo "[max9295-b MFP8]: trigger done"

echo "[imx490]: waiting"
sleep 1

echo "[imx490]: v4l2 running"
v4l2-ctl --set-fmt-video=width=2880,height=1860 --stream-mmap --stream-count=300 -d /dev/video$1
