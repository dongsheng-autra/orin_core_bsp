#!/bin/bash

# [IMX490 & OX08B]
echo "[sensors]: setup"

i2ctransfer -f -y 30 w3@0x6b 0x04 0x0b 0x00
i2ctransfer -f -y 30 w3@0x6b 0x00 0x06 0xff

sleep 0.1

i2ctransfer -f -y 30 w3@0x6b 0x00 0x10 0x22
i2ctransfer -f -y 30 w3@0x6b 0x00 0x11 0x22

sleep 0.1

i2ctransfer -f -y 30 w3@0x6b 0x00 0xF0 0x62
i2ctransfer -f -y 30 w3@0x6b 0x00 0xF1 0xEA
i2ctransfer -f -y 30 w3@0x6b 0x00 0xF4 0x0F

i2ctransfer -f -y 30 w3@0x6b 0x09 0x0B 0x07
i2ctransfer -f -y 30 w3@0x6b 0x09 0x2D 0x15
i2ctransfer -f -y 30 w3@0x6b 0x09 0x0D 0x1E
i2ctransfer -f -y 30 w3@0x6b 0x09 0x0E 0x1E
i2ctransfer -f -y 30 w3@0x6b 0x09 0x0F 0x00
i2ctransfer -f -y 30 w3@0x6b 0x09 0x10 0x00
i2ctransfer -f -y 30 w3@0x6b 0x09 0x11 0x01
i2ctransfer -f -y 30 w3@0x6b 0x09 0x12 0x01

i2ctransfer -f -y 30 w3@0x6b 0x09 0x4B 0x07
i2ctransfer -f -y 30 w3@0x6b 0x09 0x6D 0x15
i2ctransfer -f -y 30 w3@0x6b 0x09 0x4D 0x1E
i2ctransfer -f -y 30 w3@0x6b 0x09 0x4E 0x5E
i2ctransfer -f -y 30 w3@0x6b 0x09 0x4F 0x00
i2ctransfer -f -y 30 w3@0x6b 0x09 0x50 0x40
i2ctransfer -f -y 30 w3@0x6b 0x09 0x51 0x01
i2ctransfer -f -y 30 w3@0x6b 0x09 0x52 0x41

i2ctransfer -f -y 30 w3@0x6b 0x09 0x8B 0x07
i2ctransfer -f -y 30 w3@0x6b 0x09 0xAD 0x15
i2ctransfer -f -y 30 w3@0x6b 0x09 0x8D 0x1E
i2ctransfer -f -y 30 w3@0x6b 0x09 0x8E 0x9E
i2ctransfer -f -y 30 w3@0x6b 0x09 0x8F 0x00
i2ctransfer -f -y 30 w3@0x6b 0x09 0x90 0x80
i2ctransfer -f -y 30 w3@0x6b 0x09 0x91 0x01
i2ctransfer -f -y 30 w3@0x6b 0x09 0x92 0x81

i2ctransfer -f -y 30 w3@0x6b 0x09 0xCB 0x07
i2ctransfer -f -y 30 w3@0x6b 0x09 0xED 0x15
i2ctransfer -f -y 30 w3@0x6b 0x09 0xCD 0x1E
i2ctransfer -f -y 30 w3@0x6b 0x09 0xCE 0xDE
i2ctransfer -f -y 30 w3@0x6b 0x09 0xCF 0x00
i2ctransfer -f -y 30 w3@0x6b 0x09 0xD0 0xC0
i2ctransfer -f -y 30 w3@0x6b 0x09 0xD1 0x01
i2ctransfer -f -y 30 w3@0x6b 0x09 0xD2 0xC1

sleep 0.1

i2ctransfer -f -y 30 w3@0x6b 0x08 0xA0 0x04
i2ctransfer -f -y 30 w3@0x6b 0x08 0xA3 0xE4
i2ctransfer -f -y 30 w3@0x6b 0x08 0xA4 0xE4

i2ctransfer -f -y 30 w3@0x6b 0x09 0x0A 0xC0
i2ctransfer -f -y 30 w3@0x6b 0x09 0x4A 0xC0
i2ctransfer -f -y 30 w3@0x6b 0x09 0x8A 0xC0
i2ctransfer -f -y 30 w3@0x6b 0x09 0xCA 0xC0

i2ctransfer -f -y 30 w3@0x6b 0x08 0xA2 0xF0

i2ctransfer -f -y 30 w3@0x6b 0x04 0x15 0x2F
i2ctransfer -f -y 30 w3@0x6b 0x04 0x18 0x2F
i2ctransfer -f -y 30 w3@0x6b 0x04 0x1B 0x2F
i2ctransfer -f -y 30 w3@0x6b 0x04 0x1E 0x2F

sleep 0.1

# 9295-A
echo "[sensors]: 9295a"
i2ctransfer -f -y 30 w3@0x6b 0x00 0x06 0xF1

sleep 1

i2ctransfer -f -y 30 w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y 30 w3@0x40 0x03 0x18 0x5E

i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x00
sleep 0.5
i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x10
i2ctransfer -f -y 30 w3@0x40 0x00 0x00 0x82

# 9295-B
echo "[sensors]: 9295b"
i2ctransfer -f -y 30 w3@0x6b 0x00 0x06 0xF2

sleep 1

i2ctransfer -f -y 30 w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y 30 w3@0x40 0x03 0x18 0x5E

i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x00
sleep 0.5
i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x10
i2ctransfer -f -y 30 w3@0x40 0x00 0x00 0x84

# 9295-C
echo "[sensors]: 9295c"
i2ctransfer -f -y 30 w3@0x6b 0x00 0x06 0xF4

sleep 1

i2ctransfer -f -y 30 w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y 30 w3@0x40 0x03 0x18 0x5E

i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x00
sleep 0.5
i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x10
i2ctransfer -f -y 30 w3@0x40 0x00 0x00 0x86

# 9295-D
echo "[sensors]: 9295d"
i2ctransfer -f -y 30 w3@0x6b 0x00 0x06 0xF8

sleep 1

i2ctransfer -f -y 30 w3@0x40 0x02 0xBE 0x10
sleep 1
i2ctransfer -f -y 30 w3@0x40 0x03 0x18 0x5E

i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x00
sleep 0.5
i2ctransfer -f -y 30 w3@0x40 0x02 0xD6 0x10
i2ctransfer -f -y 30 w3@0x40 0x00 0x00 0x88

sleep 1
i2ctransfer -f -y 30 w3@0x6b 0x00 0x06 0xFF
i2ctransfer -f -y 30 w3@0x6b 0x00 0x18 0x0F
i2ctransfer -f -y 30 w3@0x6b 0x04 0x0b 0x02
i2ctransfer -f -y 30 w3@0x6b 0x08 0xa0 0x84

sleep 1
i2ctransfer -f -y 30 w2@0x6b 0x00 0x1A r1 # Lock status
i2ctransfer -f -y 30 w2@0x6b 0x00 0x0A r1
i2ctransfer -f -y 30 w2@0x6b 0x00 0x0B r1
i2ctransfer -f -y 30 w2@0x6b 0x00 0x0C r1

# [OX08B]: 3840, 2160
# [IMX490]: 2880, 1860
#v4l2-ctl --set-fmt-video=width=3840,height=2160 --stream-mmap --stream-count=300 -d /dev/video$1
