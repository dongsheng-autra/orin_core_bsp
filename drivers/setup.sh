#!/bin/bash

echo "[module]: insmod begin"
if [ "`sudo lsmod | grep gmsl_max9295`" == "" ]; then
    sudo insmod gmsl_max9295.ko
fi

if [ "`sudo lsmod | grep gmsl_max96712`" == "" ]; then
    sudo insmod gmsl_max96712.ko
fi

if [ "`sudo lsmod | grep gmsl2_sensors`" == "" ]; then
    sudo insmod gmsl2_sensors.ko
fi
echo "[module]: insmod done"

echo "[sensors]: v4l2 running"
v4l2-ctl --set-fmt-video=width=2880,height=1860 --stream-mmap --stream-count=300 -d /dev/video$1
