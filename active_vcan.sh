#!/bin/bash

sudo ip link add dev vcan0 type vcan
sudo ip link set dev vcan0 up
ip addr
