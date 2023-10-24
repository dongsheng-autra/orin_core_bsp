#!/bin/bash

sudo cp camera_dms.service /lib/systemd/system/
sudo systemctl enable camera_dms
sudo systemctl restart camera_dms
