#!/bin/bash

# this script pushes hard-coded values for selected sensors of selected devices
# we use this script manly to build a demo for taking pictures/screenshots
# you can edit this script to add new device types
#
# Here are the list of logical sensor name (for the Wazigate gateway) that are used:
#
# temperatureSensor_0: soil humidity for capacitive
# temperatureSensor_0: for centibar from first watermark
# temperatureSensor_1: for raw resistance value from first watermark
# temperatureSensor_2: for centibar from second watermark
# temperatureSensor_3: for raw resistance value from second watermark
# temperatureSensor_5: first soil temperature, 1st DS18B20
# analogInput_6: battery voltage
# temperatureSensor_7: ambiant air temperature (e.g. DHT/SHT or SCD30/SCD40)
# temperatureSensor_8: ambiant air humidity (e.g. DHT/SHT or SCD30/SCD40)
# temperatureSensor_9: CO2 (SCD30)
# temperatureSensor_10: 2nd soil temperature, 2nd DS18B20
# temperatureSensor_11: 3rd soil temperature, 3rd DS18B20
# imagePkt: image packet for LoRaCAM-AI device
# analogInput_10: #packets for last image for LoRaCAM-AI stat device
# analogInput_11: #kbytes for last image for LoRaCAM-AI stat device
# analogInput_12: time on air for last image for LoRaCAM-AI stat device
# analogInput_13: measured luminosity for last image for LoRaCAM-AI stat device
#
# Ex: ./push_example_value_to_edit.sh

cd /home/pi/scripts

# CAPACITIVE_1

DEV_ID=`./show_device_by_name.sh CAPACITIVE_1 id | tr -d '\"'`

if [[ -n "DEV_ID" ]]; then
./push_sensor_test_value.sh $DEV_ID temperatureSensor_0 215
./push_sensor_test_value.sh $DEV_ID analogInput_6 2.88
fi

# TENSIOMETER_1

DEV_ID=`./show_device_by_name.sh TENSIOMETER_1 id | tr -d '\"'`

if [[ -n "DEV_ID" ]]; then
./push_sensor_test_value.sh $DEV_ID temperatureSensor_0 11.7
./push_sensor_test_value.sh $DEV_ID temperatureSensor_1 166.5
./push_sensor_test_value.sh $DEV_ID temperatureSensor_5 14.6
./push_sensor_test_value.sh $DEV_ID analogInput_6 2.98
fi

# CO2_1

DEV_ID=`./show_device_by_name.sh CO2_1 id | tr -d '\"'`

if [[ -n "DEV_ID" ]]; then
./push_sensor_test_value.sh $DEV_ID temperatureSensor_7 21.4
./push_sensor_test_value.sh $DEV_ID temperatureSensor_8 57.2
./push_sensor_test_value.sh $DEV_ID temperatureSensor_9 656.8
./push_sensor_test_value.sh $DEV_ID analogInput_6 4.38
fi

# LoRaCAM_AI_DEV_2DAA

DEV_ID=`./show_device_by_name.sh LoRaCAM_AI_DEV_2DAA id | tr -d '\"'`

if [[ -n "DEV_ID" ]]; then
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0014270000D7DA81CEDED83ADFB2571DF54633D965523F356F2E1C50B2DDF09D69BA881170869996C41E\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0114260007156BF81CB489693F3A6DD9AC06771590D837E34A2A53BE031D081B1B0B680918926BBB7F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0214240012156BF82978523145A24946E74A59E4B18A0DA7469BFDE48D194EE27A2DB37642AC0F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0314280019156C064C48BE4D50947C2FA6ECBB5288E8A13CBCA3CFB7414D24514200BD7ED7BF3D8F4ECACF\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0414210024E52038B9111A53AE7B2733DC3437B2ACA7135EDF3A9F84A9E07E104C7E599F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC05141F0028EC4A675B1EE1ECEC1C01C583F9F69B506F9A1D63FB87AF18D0046757AD\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC061426002FF0C5A19AEC380260734F33D2F270C144561579C0DE7E9B6310AB6165BE366EE639F8CB7F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0714200036E69301008926025FD11D1572A38A36023A49D13671E33B305BBECC6178F9\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC081427003FEA90A9DAE6253A3A54B7040867087FDAE9D8847C262CDB8B967A03B5AB31D2017C1705CD8F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0914260046E6EA748AD9B2B3538A8F3C00393032815A3136F45EB89891392B9FC43BE59506F918FD7F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0A1425004D156BFF49EC32586949D43ADBACD5373744FF37A5079564D6CFED665A26D79542B6FF7F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0B14210058156C2C4EB08D6FA39FEC21325DA8E8B082C03711008C71B3AF4967F9749097\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0C1420005DE67E7CA7864A86B721C2B37EFF7FAD92FA21660B28736A95E497341960DF\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0D14280061EB72956140A0D71512DC3588E0E606FE7B2B81FA1231F550EF9A1F59A747871A97095537086F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0E14280069156C0042AA6790F8A4C8B8BA5EA78F21DB67545D4753F9E1100418EA1DFAF47845E203F15B55\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC0F14280072C1296D33F2B7FE7D920BFB5AE940585E48C8CB45134A9CD253615C9645C59CE3F4E3980D1047\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC101427007CE68858960787CB0C587EBBD140E5591A96B23DB616789B386152B69BCBEED47964DF30FF7F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1114250086C11CE379481582B7D4A88EFE71F4181B3DE3723D7F7A0193B3DC3E76D723C3670CBD6F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC121425008B155E474E5223D993FD8A1F1B855C4696401D6D43EFBE1229516005EA8877EBD11F2187\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1314240094156C0003328390D44587D09EA937E1F90231D2C6D87373E6234E8A921D3D960F85CF\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC141425009A156C2550B67E4FA45D074774B92F343EB34E8F1ADEEEDBF68A44BD7A8BA19DF07B0DAF\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC15142800A2CBDE8A0DB3C01156E84C798A70A1D406B7A530E371EA641C921E1DF872EFFE1DFC755D6FB2C8\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC16142700AAEA2EA4177175614C46FAB7960A17D119172DEB64DC352D12ADE1758142D0D9910E034E5F59\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC17142600AF156C1263C0C936178AB4508076A628E575D892BE801F89929B64D843CC230CE2AA0B1B7F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC18142500B7E6E98001F30CFB1D0D068EFC3CBAAD4AE465ABA14B191F29D24FE568A5DC2200CCF9BF\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC19142500BDB9DAE2094C73667161100E10EFC3CB4C150B19A9348C044814E6AB61DED14066328B5D\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1A142500C7B1557D8F0314CBCDDBFA64EEED4F64A81CD671DD5DC3F65E8DC835710E7B8563A0F3BF\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1B142500CC156C11E3C7869C147B8AAEC14A1ADB330C639FF5A6E1903CAB1E4A9DAC09021D9EA38F\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1C142300D3C10C62B6F2FCF5E0FF3487873001F4474804E97D8ABDA94BCD2A503E81C634FB9D\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1D142200DBE8AE89E3683BAD91C1AC26273F644ACECF10AC215930F5269530D757337301D8\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1E142500E6F03E8C8A372CDAFD1962BC22E11B0CAFC4F5F2BF18AAE912A39DF87966997A58FC88A1\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC1F142600EE156C1FF4BB0504E51D9506A361206357D839D75012291D9EC1A0DF39D4728BAFA8267613\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC20142400F4156C01600714A003B44E33551CB1961AD12DFB5E3D0D046D414C604EA1145F540EB3\""
./push_sensor_test_value.sh $DEV_ID imagePkt "\"FC21140C00FDC12208DFA387D5BA2C82\""
fi

# LoRaCAM_AI_STATS_2EAA

DEV_ID=`./show_device_by_name.sh LoRaCAM_AI_STATS_2EAA id | tr -d '\"'`

if [[ -n "DEV_ID" ]]; then
./push_sensor_test_value.sh $DEV_ID analogOutput_10 34
./push_sensor_test_value.sh $DEV_ID analogOutput_11 1.161
./push_sensor_test_value.sh $DEV_ID analogOutput_12 0.21
./push_sensor_test_value.sh $DEV_ID analogOutput_13 86
fi






