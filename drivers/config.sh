#!/bin/bash

# [OV2311]

I2C_SWITCH=30
if [ ! $1 ]; then
    I2C_SWITCH=30
else
    I2C_SWITCH=$1
fi

function red_print() {
    echo -e "\e[1;31m$1\e[0m"
}

function green_print() {
    echo -e "\e[1;32m$1\e[0m"
}

# 9296A
red_print "[9296]"
i2ctransfer -f -y $I2C_SWITCH w3@0x48 0x03 0x13 0x00
sleep 0.5
i2ctransfer -f -y $I2C_SWITCH w3@0x48 0x00 0x01 0x01  # MAX96717F use 3Gbps
sleep 0.5
i2ctransfer -f -y $I2C_SWITCH w3@0x48 0x00 0x10 0x20
sleep 1

# 96717-A
red_print "[DMS]"
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x04
green_print "2"
sleep 0.5
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD5 0x07
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x04
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD8 0x07
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xBE 0x10
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x00 0x57 0x12
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x00 0x5B 0x11
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x00 0x10 0x20
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x03 0x18 0x5E

# 9296A
red_print "[9296]"
i2ctransfer -f -y $I2C_SWITCH w3@0x48 0x03 0x20 0x2F
i2ctransfer -f -y $I2C_SWITCH w3@0x48 0x03 0x13 0x02

sleep 0.5
echo "[9296-bit3]: GMSL2 link locked"
i2ctransfer -f -y $I2C_SWITCH w2@0x48 0x00 0x13 r1

echo "[9296-bit6]: Video pipeline locked"
i2ctransfer -f -y $I2C_SWITCH w2@0x48 0x01 0x08 r1
i2ctransfer -f -y $I2C_SWITCH w2@0x48 0x01 0x1A r1
i2ctransfer -f -y $I2C_SWITCH w2@0x48 0x01 0x2C r1
i2ctransfer -f -y $I2C_SWITCH w2@0x48 0x01 0x3E r1

# [OV2311]: 1600, 1300
#v4l2-ctl --set-fmt-video=width=1600,height=1300 --stream-mmap --stream-count=1000 -d /dev/video$1
