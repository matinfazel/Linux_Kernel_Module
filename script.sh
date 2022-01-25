#!/bin/bash
make
sudo insmod mymodule.ko
mjor=$(cat /proc/devices | grep iut_device | tr -dc '0-9')
sudo mknod /dev/iutnode c $mjor 0
sudo python3 pytest.py
sudo rm /dev/iutnode
sudo rmmod mymodule
make clean
