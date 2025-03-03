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
    
    last_value = {"time": datetime.utcnow().isoformat(timespec="seconds")}
    print(last_value["time"])

    for sensor_dict in last_value_full["sensors"]:
        if sensor_dict["id"] == sensor_id:
            last_value = sensor_dict

    ##################
    # try:
    #     print(last_value)
    #     print(last_value["time"])
    #     #print(last_value["time"].replace('Z', '0000+00:00'))
    #     millis_index = last_value["time"].find('.')
    #     time_last = datetime.fromisoformat((last_value["time"][:millis_index]+"+00:00"))
    #     print("no except")
    #     print(time_last)
    # except:
    #         time_last = datetime.strptime(last_value["time"],"%Y-%m-%dT%H:%M:%S.%fZ")
    #         print("except 2")
    #         print(time_last)

    try:
        replace_z = last_value["time"].replace("Z", "+00:00")
        without_tz, tz = replace_z[:-6], replace_z[-6:]
        millis_index = without_tz.find(".")
        without_millis = (
            without_tz[:millis_index] + tz if millis_index != -1 else replace_z
        )

        time_last = datetime.fromisoformat(without_millis)
    except:  # fromisoformat went wrong, old py<3.6 ?
        try:
            time_last = datetime.strptime(without_millis, "%Y-%m-%dT%H:%M:%S%z")
            print("except 1")
            print(time_last)
        except:
            print("could not parse time: {0}".format(last_value["time"]))
            sys.exit("Something bad happened")

    image_time_range = 0
    
    if len(sys.argv) > 3:
        # for images of last X hours before last:
        image_time_range = float(sys.argv[3]) * 60

    print(
        image_time_range,
        "time duration considered, before last packet -6 min, here in minutes",
    )

    date_from = (
        (time_last - timedelta(minutes=6 + image_time_range)).astimezone().isoformat()
    )
    print(date_from)

    date_from = (
        (time_last - timedelta(minutes=6 + image_time_range))
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

    # print(sensor_values)

    print("=========================================")

    if len(sys.argv) > 4 and sys.argv[4][0].upper()=='F':
        prefixes = [sys.argv[4][i : i + 2].upper() for i in range(0, len(sys.argv[4]), 2)]
        # prefixes = [sys.argv[4].upper()]
    else:
        prefixes = []

    images_data = {}

    for vals in sensor_values:
        if not isinstance(vals["value"], str): # some old initial value 800
            continue
        if len(prefixes) > 0 and not (vals["value"][:2].upper() in prefixes): # prefixes exist and dont match
            continue

        im_id = -1

        for imd in images_data:
            if not images_data[imd]["last_was_received"]: # image reception is not finished for imd
                if (vals["value"][:2].upper() == images_data[imd]["prefix"]): # imd has same prefix
                    if images_data[imd]["Pkt_id"] < int(vals["value"][2:4],16): # pkt has greater hex id
                        im_id = imd
                    else: # a new image has begun, finish the other
                        images_data[imd]["last_was_received"] = True

                    break # whether found image or a new image will be created

        if im_id == -1:

            im_id = len(images_data)

            images_data[im_id] = {
                "Pkts": {},
                "nPkt": 0,
                "Q": vals["value"][4:6].upper(),
                # "raw": "",
                "last_was_received":False,
                "prefix":vals["value"][:2].upper()
            }

            try:
                replace_z = vals["time"].replace("Z", "+00:00")
                without_tz, tz = replace_z[:-6], replace_z[-6:]
                millis_index = without_tz.find(".")
                without_millis = (
                    without_tz[:millis_index] + tz if millis_index != -1 else replace_z
                )

                time_first = datetime.fromisoformat(without_millis)
            except:
                try:
                    time_first = datetime.strptime(vals["time"], "%Y-%m-%dT%H:%M:%S%z")
                    print("except 1")
                    print(time_first)
                except:
                    print("could not parse time: {0}".format(vals["time"]))
                    sys.exit("Something bad happened")

            images_data[im_id]["date"] = time_first.strftime(
                "%Y-%m-%d-%H-%M-%S"
            )

        # images_data[im_id]["raw"] += vals["value"] + "\n"
        images_data[im_id]["Pkt_id"] = int(vals["value"][2:4],16)
        
        # print("prefix " + vals["value"][:2])
        # print("---------------------------")
        # print("--> " + vals["value"])

        images_data[im_id]["Pkts"][len(images_data[im_id]["Pkts"])] = re.sub(r'(.{2})', r'\1 ', vals["value"][6:].replace('\n', '').replace('\r', ''))


    for imd in images_data:
        
        last_image_raw = "0080 0080 00" + images_data[imd]["Q"] + " "
        nPkt = 0

        for imgPkt in images_data[imd]["Pkts"]:  # todo: sort pkts, what if missing ??
            nPkt += 1
            last_image_raw = (
                last_image_raw + "00" + images_data[imd]["Pkts"][imgPkt].upper()
            )

            nPktHex = hex(nPkt).upper()[2:]

        final_last_image_raw = "00" + nPktHex + " " + last_image_raw

        outputFile = "{0}_".format(images_data[imd]["date"]) + dev_name + ".txt"

        with open(outputFile, "w") as outfile:
            outfile.write(final_last_image_raw)

        print(outputFile)
        print("decode with: ./decode_to_bmp", outputFile, "128x128-ESP32S3-test.bmp")
        
        if len(sys.argv) > 5 and sys.argv[5][0]=='.':
            path_prefix = sys.argv[5]
        else:
            if len(sys.argv) > 4 and sys.argv[4][0]=='.':
                path_prefix = sys.argv[4]
            else:
                path_prefix = "."
                      
        try:
            subprocess.check_output(path_prefix+"/decode_to_bmp "
                +outputFile+" "+path_prefix+"/128x128-ESP32S3-test.bmp", shell = True
            )
        except:
            print("cannot decode automatically")
            print(path_prefix+"/decode_to_bmp not found") 

        # outputFile = "{0}_".format(images_data[imd]["date"]) + dev_name + "_raw.txt"

        # with open(outputFile, "w") as outfile:
        #     outfile.write(images_data[imd]["raw"])
        
        # print(outputFile)

else:
    print("get_multiple_images_files.py requires arguments")
    print("e.g.: python get_multiple_image_files.py 192.168.0.29 67b6fe8568f3190a22e91c6f 1.5") 
    print("  --> image data from 192.168.0.29 for device 67b6fe8568f3190a22e91c6f in the last 1.5 hour")
    print("      optional 4th argument: FEFAF1F3")       
    print("  --> for a list of prefixes of image packets")
    print("      optional 4th or 5th argument: ../AGRIFUTUR")        
    print("  --> and automatic decoding with ../AGRIFUTUR/decode_to_bmp")