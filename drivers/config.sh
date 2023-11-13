#!/bin/bash

# [IMX490 & OX08B & ISX031]

I2C_SWITCH=7
if [ ! $1 ]; then
    I2C_SWITCH=7
else
    I2C_SWITCH=$1
fi

I2C_MAX96712=0x6b

function red_print() {
    echo -e "\e[1;31m$1\e[0m"
}

function green_print() {
    echo -e "\e[1;32m$1\e[0m"
}

camera_array=([1]=sg3-isx031-gmsl2
              [2]=sg8-ox08bc-gmsl2
              [3]=sg5-imx490-gmsl2)

echo 1:${camera_array[1]}
echo 2:${camera_array[2]}
echo 3:${camera_array[3]}

green_print "Press select your camera type:"
read key

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x0b 0x00  # MIPI CSI disable
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x06 0xFF  # Enable all 4 Links in GMSL2 mode

sleep 0.1

# [ISX031] 3Gbps [IMX490 & OX08B] 6Gbps
if [ ${camera_array[key]} == sg3-isx031-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x10 0x11  # MAX96717F use 3Gbps
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x11 0x11
    green_print "DPHY Speed 3Gbps"
else
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x10 0x22  # MAX9295 use 6Gbps
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x11 0x22
    green_print "DPHY Speed 6Gbps"
fi

sleep 0.1

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0xF0 0x62  # LinkB -> Pipe Z -> Pipe 1, LinkA -> Pipe Z -> Pipe 0
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0xF1 0xEA  # LinkD -> Pipe Z -> Pipe 3, LinkC -> Pipe Z -> Pipe 2
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0xF4 0x0F  # Enable 0 - 3 Pipes

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x0B 0x07  # why MAP_SRC_0 MAP_SRC_1 MAP_SRC_2, maybe means 3 fifo ?
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x2D 0x15  # Map CSI to Controller 1 from MAX96712 datasheet
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x0D 0x1E  # YUV422 - 0x1E
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x0E 0x1E  # Map to VC0
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x0F 0x00
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x10 0x00
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x11 0x01
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x12 0x01

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x4B 0x07
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x6D 0x15
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x4D 0x1E
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x4E 0x5E  # Map to VC1 maybe it's should be equal to ORIN vc-id?
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x4F 0x00
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x50 0x40
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x51 0x01
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x52 0x41

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x8B 0x07
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xAD 0x15
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x8D 0x1E
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x8E 0x9E  # Map to VC2
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x8F 0x00
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x90 0x80
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x91 0x01
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x92 0x81

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xCB 0x07
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xED 0x15
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xCD 0x1E
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xCE 0xDE  # Map to VC3
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xCF 0x00
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xD0 0xC0
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xD1 0x01
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xD2 0xC1

sleep 0.1

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x08 0xA0 0x04  # default MIPI PHY 2x4 lanes
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x08 0xA3 0xE4  # PORTA 4 lanes
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x08 0xA4 0xE4  # PORTB 4 lines

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x0A 0xC0  # default Four data lanes
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x4A 0xC0
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0x8A 0xC0
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x09 0xCA 0xC0

i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x08 0xA2 0xF0  # Eable all MIPI PHYS

# DPHY[connect with orin]
if [ ${camera_array[key]} == sg3-isx031-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x15 0x35
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x18 0x35
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x1B 0x35
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x1E 0x35
    green_print "MIPI Speed 2.1Gbps"
else
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x15 0x37
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x18 0x37
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x1B 0x37
    i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x1E 0x37
    green_print "MIPI Speed 2.3Gbps"
fi

sleep 0.1

# 9295-A/96717-A
echo "[sensors]: serializer-a"
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x06 0xF1 # Enable LinkA

sleep 1

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x03 0x18 0x5E

if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
fi
sleep 0.5
if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
fi

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x00 0x00 0x82

# 9295-B/96717-B
echo "[sensors]: serializer-b"
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x06 0xF2 # Enable LinkB

sleep 1

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x03 0x18 0x5E

if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
fi
sleep 0.5
if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
fi

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x00 0x00 0x84

# 9295-C/96717-C
echo "[sensors]: serializer-c"
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x06 0xF4 # Enable LinkC

sleep 1

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x03 0x18 0x5E

if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
fi
sleep 0.5
if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
fi

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x00 0x00 0x86

# 9295-D/96717-D
echo "[sensors]: serializer-d"
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x06 0xF8 # Enable LinkD

sleep 1

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x03 0x18 0x5E

if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
fi
sleep 0.5
if [ ${camera_array[key]} == sg5-imx490-gmsl2 ]; then
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x00
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x00
else
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD6 0x10
    i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x02 0xD3 0x10
fi

i2ctransfer -f -y $I2C_SWITCH w3@0x40 0x00 0x00 0x88

sleep 0.5
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x06 0xFF # Enable all 4 Links in GMSL2 mode
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x00 0x18 0x0F
sleep 0.1
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x04 0x0B 0x02 # Enable MIPI CSI
i2ctransfer -f -y $I2C_SWITCH w3@$I2C_MAX96712 0x08 0xA0 0x84 # Force all MIPI clocks running

sleep 0.5
echo "[96712-bit3]: GMSL2 link locked"
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x00 0x1A r1
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x00 0x0A r1
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x00 0x0B r1
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x00 0x0C r1

echo "[96712-bit6]: Video pipeline locked"
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x01 0x08 r1
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x01 0x1A r1
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x01 0x2C r1
i2ctransfer -f -y $I2C_SWITCH w2@$I2C_MAX96712 0x01 0x3E r1

# [OX08BC]: 3840, 2160
# [IMX490]: 2880, 1860
# [ISX031]: 1920, 1536
#v4l2-ctl --set-fmt-video=width=3840,height=2160 --stream-mmap --stream-count=1000 -d /dev/video$1
#v4l2-ctl --set-fmt-video=width=2880,height=1860 --stream-mmap --stream-count=1000 -d /dev/video$1
#v4l2-ctl --set-fmt-video=width=1920,height=1536 --stream-mmap --stream-count=1000 -d /dev/video$1
