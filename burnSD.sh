#!/bin/bash

./mkv210 u-boot.bin u-boot.16k
sudo dd iflag=dsync oflag=dsync if=u-boot.16k of=/dev/sdb seek=1
sudo dd iflag=dsync oflag=dsync if=u-boot.bin of=/dev/sdb seek=49
