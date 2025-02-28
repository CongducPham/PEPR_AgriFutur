# this simple script takes as input a txt file where each line 
# is the raw content of image pkt received by the WaziGate
# it outputs to default stdout

import re
import sys

file = open(sys.argv[1], 'r')
lines = file.readlines()

prefix = lines[0][:2]
Q = lines[0][4:6].upper()
nPktHex = hex(len(lines)).upper()[2:]

print("00"+nPktHex, "0080 0080", "00"+Q+" ", end='')

for line in lines:			
		line = line[6:].replace('\n', '').replace('\r', '')			
		output_string = re.sub(r'(.{2})', r'\1 ', line)
		print("00"+output_string.upper(), end='')
			
file.close()			

#curl -X GET "http://localhost/devices/67b6fe8568f3190a22e91c6f/sensors/imagePkt/values" -H  "accept: application/json" | jq '.[].value' | tr -d '\"'