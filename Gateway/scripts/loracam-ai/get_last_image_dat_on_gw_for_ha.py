import subprocess
import requests
import json
import sys
import re
from datetime import datetime, timezone, timedelta

# common headers for requests
iiwa_headers = {"content-type": "application/json"}
WaziGate_headers = {"accept": "application/json", "content-type": "application/json"}
WaziGate_headers_auth = {
    "accept": "application/json",
    "content-type": "application/json",
    # 'Authorization': 'Bearer **'
    "Authorization": "Bearer ",
}
# ---------------------#

if len(sys.argv) > 2:
    print("=========================================")

    # BASE_URL = "http://localhost/"
    # BASE_URL = "http://wazigate.local/"
    BASE_URL = "http://" + sys.argv[1] + "/"

    my_token = "hello"
    # get the token first
    WaziGate_url = "http://" + sys.argv[1] + "/auth/token"
    try:
        pload = '{"username":"admin","password":"loragateway"}'
        response = requests.post(
            WaziGate_url, headers=WaziGate_headers, data=pload, timeout=30
        )
        # print(response.text)
        WaziGate_headers_auth["Authorization"] += response.text.replace('"', "")
    except requests.exceptions.RequestException as e:
        print(e)
        print("get-entry: requests command failed")

    WaziGate_url = BASE_URL + "devices"
    try:
        response = requests.get(WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
    except requests.exceptions.RequestException as e:
        print(e)
        print("get-devices: request 1 command failed")
        sys.exit("Something bad happened")

    try:
        device_json = json.loads(response.text)
    except:
        print("could not parse JSON")
        print(response.text)
        sys.exit("Something bad happened")

    # print(json.dumps(device_json, indent=4))

    #########################
    dev_id = sys.argv[2]
    sensor_id = "imagePkt"

    # WaziGate_url = BASE_URL+'devices/'+dev_id+'/sensors/'+sensor_id+'/value'
    WaziGate_url = BASE_URL + "devices/" + dev_id
    try:
        response = requests.get(WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
    except requests.exceptions.RequestException as e:
        print(e)
        print("get-values: request 1 command failed")
        sys.exit("Something bad happened")

    try:
        last_value_full = json.loads(response.text)
    except:
        print("could not parse JSON")
        print(WaziGate_url)
        print(response.text)
        last_value_full = {"sensors": []}
        sys.exit("Something bad happened")
        
    dev_name = last_value_full["name"]    

    for sensor_dict in last_value_full["sensors"]:
        if sensor_dict["id"] == sensor_id:
            last_value = sensor_dict

    ##################
    try:
        print(last_value)
        print(last_value["time"])
        # print(last_value["time"].replace('Z', '0000+00:00'))
        millis_index = last_value["time"].find(".")
        time_last = datetime.fromisoformat((last_value["time"][:millis_index] + "+00:00"))
        print("no except")
        print(time_last)
    except:
        try:
            time_last = datetime.strptime(last_value["time"], "%Y-%m-%dT%H:%M:%S%z")
            print("except 1")
            print(time_last)
        except:
            time_last = datetime.strptime(last_value["time"], "%Y-%m-%dT%H:%M:%S.%fZ")
            print("except 2")
            print(time_last)

    date_from = (time_last - timedelta(minutes=6)).astimezone().isoformat()
    print(date_from)

    if len(sys.argv) > 3 and sys.argv[3][0].upper()!='.':
        # format of user input date is 2025-02-24T12:43:15+01:00
        date_from = sys.argv[3].replace(":", "%3A").replace("+", "%2B")
    else:
        date_from = (
            (time_last - timedelta(minutes=6))
            .replace(microsecond=0)
            .astimezone()
            .isoformat()
            .replace(":", "%3A")
            .replace("+", "%2B")
        )

    print(date_from)

    # curl -X GET "http://192.168.0.29/devices/67b6fe8568f3190a22e91c6f/sensors/imagePkt/values?from=2025-02-21T10%3A29%3A35%2B01%3A00" -H "accept: application/json"

    WaziGate_url = BASE_URL+'devices/'+dev_id+'/sensors/'+sensor_id+'/values?from='+date_from
    
    try:
        response = requests.get(WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
    except requests.exceptions.RequestException as e:
        print(e)
        print("get-values: request 1 command failed")
        sys.exit("Something bad happened")

    try:
        sensor_values = json.loads(response.text)
    except:
        print("could not parse JSON")
        print(WaziGate_url)
        print(response.text)
        sensor_values = []

    last_image_date = "empty"
    last_image_raw = ""
    first_chunk_parsed = False

    print(sensor_values)

    print("=========================================")

    if len(sys.argv) > 4 and sys.argv[4][0].upper()=='F':
        prefix = sys.argv[4].upper()
    else:
        prefix = last_value["value"][:2].upper()
        
    if prefix[0] == 'F':
        Q = last_value["value"][4:6].upper()
        nPkt = 0
        last_image_raw = "0080 0080 00" + Q + " "
    
        print("Search for prefix", prefix)
    
        for vals in sensor_values:
            if vals["value"][:2].upper() == prefix:
                nPkt = nPkt + 1
                line = vals["value"][6:].replace("\n", "").replace("\r", "")
                imgPkt = re.sub(r"(.{2})", r"\1 ", line)
                last_image_raw = last_image_raw + "00" + imgPkt.upper()
    
                if not first_chunk_parsed:
                    first_chunk_parsed = True
                    try:
                        time_first = datetime.fromisoformat(vals["time"])
                        print("no except")
                        print(time_first)
                    except:
                        try:
                            time_first = datetime.strptime(
                                vals["time"], "%Y-%m-%dT%H:%M:%S%z"
                            )
                            print("except 1")
                            print(time_first)
                        except:
                            time_first = datetime.strptime(
                                vals["time"], "%Y-%m-%dT%H:%M:%S.%fZ"
                            )
                            print("except 2")
                            print(time_first)
    
                    last_image_date = time_first.strftime("%Y-%m-%d-%H-%M-%S")
    
                print(vals["value"])
    
            nPktHex = hex(nPkt).upper()[2:]
            final_last_image_raw = "00" + nPktHex + " " + last_image_raw
    
        outputFile = "{0}_".format(last_image_date) + dev_name + ".txt"
     
        with open(outputFile, "w") as outfile:
            outfile.write(final_last_image_raw)
    
        print(outputFile)
        print("decode with: ./decode_to_bmp", outputFile, "128x128-ESP32S3-test.bmp")
        
        if len(sys.argv) > 3 and sys.argv[3][0]=='.':
            path_prefix = sys.argv[3]
        else:
            if len(sys.argv) > 4 and sys.argv[4][0]=='.':
                path_prefix = sys.argv[4]
            else:
                if len(sys.argv) > 5 and sys.argv[5][0]=='.':
                    path_prefix = sys.argv[5]
                else:    
                    path_prefix = "."
                          
        try:
            subprocess.check_output(path_prefix+"/decode_to_bmp "
                +outputFile+" "+path_prefix+"/128x128-ESP32S3-test.bmp 2> LAST_DECODED_FILENAME.txt", shell = True
            )
        except:
            print("cannot decode automatically")
            print(path_prefix+"/decode_to_bmp not found")    
        
        print("copy to /opt/homeassistant/config/www/loracam-ai") 
        try:
            subprocess.check_output("sudo cp `LAST_DECODED_FILENAME.txt` /opt/homeassistant/config/www/loracam-ai",
            shell = True
            )        
        except:
            print("copy decoded bmp to /opt/homeassistant/config/www/loracam-ai failed")
            
        print("copy as "+"last-"+dev_name+"-image.bmp")
        try:
            subprocess.check_output("sudo cp `LAST_DECODED_FILENAME.txt` /opt/homeassistant/config/www/loracam-ai/"
            +"last-"+dev_name+"-image.bmp",
            shell = True
            )        
        except:
            print("copy to /opt/homeassistant/config/www/loracam-ai/"+"last-"+dev_name+"-image.bmp failed")     

    else:
        print("prefix seems not valid")
else:
    print("get_last_image_dat_on_gw_for_ha.py requires at least 2 arguments")
    print("e.g.: python get_last_image_dat_on_gw_for_ha.py localhost 67b6fe8568f3190a22e91c6f")
    print("      optional arguments: 2025-02-24T12:43:15+01:00 FE")
    print("  --> start date to consider and prefix of image packets")
    print("      optional 3rd, 4th or 5th argument: ../AGRIFUTUR")        
    print("  --> and automatic decoding with ../AGRIFUTUR/decode_to_bmp")    
