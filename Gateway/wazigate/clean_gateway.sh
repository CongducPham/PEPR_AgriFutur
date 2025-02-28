#!/bin/bash

echo "enable auto configuration on boot"
sudo rm /boot/intel-irris-auto-config.done
sudo rm /boot/intel-irris-auto-config.log

echo "clear history"
history -c


