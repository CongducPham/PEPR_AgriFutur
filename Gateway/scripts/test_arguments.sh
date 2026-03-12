#!/bin/bash

# Bash scripting cheatsheet https://devhints.io/bash

# Ex: create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 AA
# this script creates a capacitive sensor named CAPACITIVE_1 and dev addr 26011Dyy
# for capacitive, it is recommended to use AA, AB, AC,... for yy

# you can add a third parameter to indicate a specific device id to be assigned to the created device
# Ex: create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 AA 64425c0068f31909357de7c8

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "Need the device name index and the last byte of the device address"
    echo "e.g. create_full_capacitive_device_with_dev_addr.sh CAPACITIVE_1 AA"
    exit
fi

DEV_ID=""
INIT_VALUE=true

POSITIONAL=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --dev-id)
      DEV_ID="$2"
      shift 2
      ;;
    --no-init)
      INIT_VALUE=false
      shift 1
      ;;      
    *)
      POSITIONAL+=("$1")
      shift
      ;;
  esac
done

set -- "${POSITIONAL[@]}"

DEVICE_NAME="$1"
DEV_ADDR="$2"

echo "Device name: $DEVICE_NAME"
echo "Device address: $DEV_ADDR"
echo "Optional device id: $DEV_ID"
echo "Optional init value: $INIT_VALUE"