#!/bin/bash

# create a custom codec on WaziGate
# https://github.com/Waziup/wazigate-edge/tree/f3c77c1d12abdceb597a0b0d941c82e61f223d8b/edge/codecs
#
echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

ERR=`curl -X POST "http://localhost/codecs" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"id\": \"67b7026e68f3190a22e92c6a\",
  \"name\": \"AQUASCOPELORAIN\",
	\"mime\": \"application/javascript\",  
  \"script\": \"\nfunction decodeUplink(input) {\n\tbytes = input.bytes;\n\tvar data = {};\n  \tfor (i=0 ; i<bytes.length; i++) {\n    \tswitch (bytes[i]) {\n      \t\tcase 0x03:\n        \t\tdata.hw_version = bytes[++i];\n        \t\tdata.capabilities = (bytes[++i] << 8)+  bytes[++i];\n        \t\tbreak;\n      \t\tcase 0x04:\n        \t\tp = bytes[++i];\n        \t\tv = (bytes[++i] << 8)+  bytes[++i];\n        \t\tswitch (p) {\n\t\t\t\tcase 0x02:\n\t\t\t\t\tdata.conf_heartbeat=v;\n        \t        break;\n   \t\t     \tcase 0x03:\n        \t    \tdata.conf_heavyrain=v;\n            \t    break;\n        \t\tcase 0x04:\n        \t\t\tdata.conf_interval=v;\n            \t\tbreak;\n            \tdefault:\n                \tdata.error = \"config parameter? \"+bytes[i];\n        \t\t}\n        \tbreak;\n        \tcase 0x06:\n        \t\tsensor = bytes[++i];\n            \tsensorvalue = (bytes[++i] << 8)+  bytes[++i];\n            \tswitch(sensor) {\n            \tcase 0x01:\n                \tdata.temperature = sensorvalue;\n                    break;\n            \tcase 0x03:\n                \tdata.uptime = sensorvalue;\n                    break;\n            \tcase 0x81:\n                \tdata.rainlevel = sensorvalue;\n                    break;\n                default:\n                    data.error = \"sensor type? \"+bytes[i];\n            \t}\n        \tbreak;\n        case 0x0a:\n            data.fw_version = (bytes[++i] << 24) +(bytes[++i] << 16) + (bytes[++i] << 8) + bytes[++i];\n            break;\n        case 0x0b:\n            data.alarm_status = bytes[++i];\n            data.alarm_type = bytes[++i];\n            data.alarm_value = (bytes[++i] << 8)+  bytes[++i];\n            break;\n        case 0x12:\n            data.bat_voltage = bytes[++i];\n            data.bat_consumption = (bytes[++i] << 8) + bytes[++i];        \n            break;\n        default:\n            data.error = \"command? \"+bytes[i];\n        }\n    }  \n    return {\n    data: data,\n    warnings: [],\n    errors: []\n  };  \n}\n\"
}" | tr -d '\"'`

echo "return code: $ERR"