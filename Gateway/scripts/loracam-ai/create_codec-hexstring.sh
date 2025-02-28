#!/bin/bash

echo "--> Get token"
TOK=`curl -X POST "http://localhost/auth/token" -H  "accept: application/json" -H  "Content-Type: application/json" -d "{\"username\":\"admin\",\"password\":\"loragateway\"}" | tr -d '\"'`

DATE=`date +"%Y-%m-%dT06:00:00.001Z"`

echo "--> Use date of $DATE"

ERR=`curl -X POST "http://localhost/codecs" -H "accept: application/json" -H "Authorization: Bearer $TOK" -H  "Content-Type: application/json" -d "{
  \"id\": \"application/hexstring\",
  \"name\": \"HEXSTRING\",
	\"mime\": \"application/javascript\",  
  \"script\": \"\nfunction Decoder(bytes, port) {\n  // Convert bytes to hexadecimal string\n  var hexString = bytes2HexString(bytes);\n\n  // Return the decoded values as a JSON object\n  return {\n    imagePkt: hexString\n  };\n}\n\n// Helper function to convert byte array to hexadecimal string\nfunction bytesToHex(bytes) {\n  return bytes.map(function(byte) {\n    return ('0' + (byte \u0026 0xFF).toString(16)).slice(-2);\n  }).join('');\n}\n\nfunction bytes2HexString(arrBytes) {\n    var str = ''\n    for (var i = 0; i \u003c arrBytes.length; i++) {\n        var tmp\n        var num = arrBytes[i]\n        if (num \u003c 0) {\n            tmp = (255 + num + 1).toString(16)\n        } else {\n            tmp = num.toString(16)\n        }\n        if (tmp.length === 1) {\n            tmp = '0' + tmp\n        }\n        str += tmp\n    }\n    return str\n}\n}\"
}" | tr -d '\"'`
  
echo "return code: $ERR"