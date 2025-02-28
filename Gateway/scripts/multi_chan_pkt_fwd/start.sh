#!/bin/bash
# This file initiates the LoRa packet forwarders in a fail-switch manner in order to 
# match with the hardware installed on the pi

########################
# function definitions #
########################

# reset the concentrator via GPIO pins
function reset_gpio {

  echo -e "Restarting the CE0 pin..."
  CSPIN=8
  echo "$CSPIN" > /sys/class/gpio/export
  sleep 1
  echo "out" > /sys/class/gpio/gpio$CSPIN/direction
  echo "0" > /sys/class/gpio/gpio$CSPIN/value
  sleep 1
  echo "1" > /sys/class/gpio/gpio$CSPIN/value
  echo -e "Done\n"
  sleep 1
  
  echo -e "Restarting the Concentrator..."
  RSTPIN=17
  echo "$RSTPIN" > /sys/class/gpio/export
  sleep 1
  echo "out" > /sys/class/gpio/gpio$RSTPIN/direction
  echo "1" > /sys/class/gpio/gpio$RSTPIN/value
  sleep 1
  echo "0" > /sys/class/gpio/gpio$RSTPIN/value
  sleep 1
}


# Start the forwarder
# parameter 1: location of the forwarder executable
# parameter 2: location of the config file (global_conf.json)
function start_forwarder {
    cd $(dirname "$1")
    # Linking global_conf.json
    ln -s $2
    # launch the concentrator
    ./$(basename "$1")
}

####################
# Start forwarders #
####################


# check presence of wazigate-edge
curl --fail http://waziup.wazigate-edge/device/id
res=$?
if test "$res" != "0"; then
   echo "Wazigate-edge cannot be reached, exiting."
   exit 1;
fi

# Trying all enabled concentrators, one at a time

VAR_ENABLE_MULTI_SPI=${ENABLE_MULTI_SPI:-0}
if [ "$VAR_ENABLE_MULTI_SPI" == "1" ]; then

  echo -e "\n============================\n"
  echo -e "Initiating the SPI multi-channel Lora packet forwarder..."
  echo -e "\n============================\n\n"

  ~/spi_multi_chan/reset_lgw.sh start 17
  reset_gpio
  start_forwarder ~/spi_multi_chan/lora_pkt_fwd ~/conf/multi_chan_pkt_fwd/global_conf.json
  ~/spi_multi_chan/reset_lgw.sh stop 17

fi

#---------------------------------------#

VAR_ENABLE_SINGLE_SPI=${ENABLE_SINGLE_SPI:-0}
if [ "$VAR_ENABLE_SINGLE_SPI" == "1" ]; then

  echo -e "\n============================\n"
  echo -e "Initiating the single-channel Lora packet forwarder..."
  echo -e "\n============================\n\n"

  #reset_gpio
  echo -e "Restarting the CE0 pin..."
  CSPIN=8
  echo "$CSPIN" > /sys/class/gpio/export
  sleep 1
  echo "out" > /sys/class/gpio/gpio$CSPIN/direction
  echo "0" > /sys/class/gpio/gpio$CSPIN/value
  sleep 1
  echo "1" > /sys/class/gpio/gpio$CSPIN/value
  echo -e "Done\n"
  sleep 1

  echo -e "Restarting the RFM95x Module..."
  #should be 4 for LowCostLoRaGw SX12XX hat (C. Pham)
  #but on RFM9X, the module can work without being explicitly reset as powering on already did the reset
  RSTPIN=17
  echo "$RSTPIN" > /sys/class/gpio/export
  sleep 1
  echo "out" > /sys/class/gpio/gpio$RSTPIN/direction
  echo "0" > /sys/class/gpio/gpio$RSTPIN/value
  sleep 0.0001
  echo "1" > /sys/class/gpio/gpio$RSTPIN/value
  echo -e "Done\nLaunching the forwarder..."
  sleep 1
  start_forwarder ~/single_chan/lora_pkt_fwd ~/conf/single_chan_pkt_fwd/global_conf.json

fi

#---------------------------------------#

VAR_ENABLE_MULTI_USB=${ENABLE_MULTI_USB:-0}
if [ "$VAR_ENABLE_MULTI_USB" == "1" ]; then

  echo -e "\n============================\n"
  echo -e "Initiating the USB multi-channel Lora packet forwarder..."
  echo -e "\n============================\n\n"
  start_forwarder ~/usb_multi_chan/lora_pkt_fwd ~/conf/multi_chan_pkt_fwd/global_conf.json &

fi

#---------------------------------------#

echo -e "\n............................\n"
echo -e "Enabled Forwarders: "
if [ "$VAR_ENABLE_MULTI_SPI" == "1" ]; then echo "\n\t-\t MULTI_SPI"; fi
if [ "$VAR_ENABLE_MULTI_USB" == "1" ]; then echo "\n\t-\t MULTI_USB"; fi
if [ "$VAR_ENABLE_SINGLE_SPI" == "1" ]; then echo "\n\t-\t SINGLE_SPI"; fi

echo -e "\nNo forwarders could make it, exiting." 
exit 2