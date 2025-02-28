import requests
import json
import sys
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

    print("=========================================")

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
    sens_id = "imagePkt"

    # WaziGate_url = BASE_URL+'devices/'+dev_id+'/sensors/'+sens_id+'/value'
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

    for sens_dict in last_value_full["sensors"]:
        if sens_dict["id"] == sens_id:
            last_value = sens_dict

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

    date_from = (
        (time_last - timedelta(minutes=6))
        .replace(microsecond=0)
        .astimezone()
        .isoformat()
        .replace(":", "%3A")
        .replace("+", "%2B")
    )
    print(date_from)

    # difficile de programmer "2025-02-21T10 %3A 29 %3A 35 %2B 01 %3A 00"
    # difficile de programmer "2025-02-21T10 29 35 01 00"

    # curl -X GET "http://192.168.0.29/devices/67b6fe8568f3190a22e91c6f/sensors/imagePkt/values?from=2025-02-21T10%3A29%3A35%2B01%3A00" -H "accept: application/json"

    WaziGate_url = (
        BASE_URL
        + "devices/"
        + dev_id
        + "/sensors/"
        + sens_id
        + "/values?from="
        + date_from
    )
    try:
        response = requests.get(WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
    except requests.exceptions.RequestException as e:
        print(e)
        print("get-values: request 1 command failed")
        sys.exit("Something bad happened")

    try:
        sens_values = json.loads(response.text)
    except:
        print("could not parse JSON")
        print(WaziGate_url)
        print(response.text)
        sens_values = []

    last_image_date = "empty"
    last_image_raw = ""
    first_chunk_parsed = False

    print(sens_values)

    print("=========================================")

    prefix = last_value["value"][:2].upper()

    if len(sys.argv) > 4:
        prefix = sys.argv[4].upper()
    else:
        prefix = last_value["value"][:2].upper()

    for vals in sens_values:
        if vals["value"][:2].upper() == prefix:
            last_image_raw += vals["value"] + "\n"
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

    outputFile = "{0}_image_pkt.txt".format(last_image_date)

    with open(outputFile, "w") as outfile:
        outfile.write(last_image_raw)

    print(outputFile)
    print("display in dat file: python image_pkt_to_image_dat.py ", outputFile)
else:
    print("get_last_image_pkt.py requires at least 2 arguments")
    print("e.g.: python get_last_image_pkt.py 192.168.0.29 67b6fe8568f3190a22e91c6f")