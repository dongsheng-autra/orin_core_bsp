#!/bin/bash

echo "[module]: insmod begin"
if [ "`sudo lsmod | grep gmsl_max9295`" == "" ]; then
    sudo insmod gmsl_max9295.ko
fi

if [ "`sudo lsmod | grep gmsl_max9296`" == "" ]; then
    sudo insmod gmsl_max9296.ko
fi

if [ "`sudo lsmod | grep imx490`" == "" ]; then
    sudo insmod imx490.ko
fi
echo "[module]: insmod done"

echo "[max9296]: step1"
i2ctransfer -y -f 30 w3@0x48 0x03 0x13 0x00
echo "[max9296]: step1 0"
i2ctransfer -y -f 30 w3@0x48 0x00 0x03 0x40
echo "[max9296]: step1 1"
i2ctransfer -y -f 30 w3@0x48 0x02 0xc2 0x83
echo "[max9296]: step1 2"
i2ctransfer -y -f 30 w3@0x48 0x02 0xc3 0xa7
echo "[max9296]: step1 done"

echo "[max9295]: setup"
i2ctransfer -y -f 30 w3@0x60 0x02 0xd3 0x04
echo "[max9295]: setup 0"
i2ctransfer -y -f 30 w3@0x60 0x02 0xd5 0x07
echo "[max9295]: setup 1"
i2ctransfer -y -f 30 w3@0x60 0x02 0xd6 0x04
echo "[max9295]: setup 2"
i2ctransfer -y -f 30 w3@0x60 0x02 0xd8 0x07
echo "[max9295]: setup 3"
i2ctransfer -y -f 30 w3@0x60 0x02 0xbe 0x10
echo "[max9295]: setup 4"
i2ctransfer -y -f 30 w3@0x60 0x00 0x57 0x12
echo "[max9295]: setup 5"
i2ctransfer -y -f 30 w3@0x60 0x00 0x5b 0x11
echo "[max9295]: setup 6"
i2ctransfer -y -f 30 w3@0x60 0x00 0x10 0x20
sleep 0.1
echo "[max9295]: setup 7"
i2ctransfer -y -f 30 w3@0x60 0x03 0x18 0x5e
echo "[max9295]: setup done"

echo "[max9296]: step2"
i2ctransfer -y -f 30 w3@0x48 0x03 0x20 0x30
echo "[max9296]: step2 0"
i2ctransfer -y -f 30 w3@0x48 0x03 0x13 0x02
echo "[max9296]: step2 done"

echo "[max9295 MFP8]: trigger"
i2ctransfer -y -f 30 w3@0x60 0x02 0xd6 0x10
sleep 0.5
i2ctransfer -y -f 30 w3@0x60 0x02 0xd6 0x00
sleep 0.5
echo "[max9295 MFP8]: trigger done"

echo "[imx490]: waiting"
sleep 1

echo "[imx490]: v4l2 running"
v4l2-ctl --set-fmt-video=width=2880,height=1860 --stream-mmap --stream-count=300 -d /dev/video0
#v4l2-ctl --set-fmt-video=width=$2,height=$3 --stream-mmap --stream-count=10000000 -d /dev/video$1 #--stream-to=camera_0_1280_720.yuv
