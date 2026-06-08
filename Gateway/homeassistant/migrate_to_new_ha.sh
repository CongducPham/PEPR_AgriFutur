#!/bin/bash

cd /opt/homeassistant/config
sudo cp /home/pi/homeassistant/configuration_for_new_ha.yaml ./configuration.yaml
cd /home/pi/homeassistant
echo "removing if needed *old_ha.yaml"
rm -f configuration_old_ha.yaml view_block_loracam_old_ha.yaml conf_block_loracam_old_ha.yaml
echo "replace configuration, conf_block and view_block files for loracam"
mv configuration.yaml configuration_old_ha.yaml
mv configuration_for_new_ha.yaml configuration.yaml
mv view_block_loracam.yaml view_block_loracam_old_ha.yaml
mv view_block_loracam_for_new_ha.yaml view_block_loracam.yaml
mv conf_block_loracam.yaml conf_block_loracam_old_ha.yaml
mv conf_block_loracam_for_new_ha.yaml conf_block_loracam.yaml
echo "Done"
