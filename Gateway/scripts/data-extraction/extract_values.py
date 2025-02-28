import requests
import json
import sys


# common headers for requests
iiwa_headers = {
    'content-type': 'application/json'
}
WaziGate_headers = {
    'accept': 'application/json',
    'content-type': 'application/json'
}
WaziGate_headers_auth = {
    'accept': 'application/json',
    'content-type': 'application/json',
    # 'Authorization': 'Bearer **'
    'Authorization': 'Bearer '
}
# ---------------------#

if len(sys.argv)>2:
    print('=========================================')

    # BASE_URL = "http://localhost/"
    # BASE_URL = "http://wazigate.local/"
    BASE_URL = "http://"+sys.argv[1]+"/"


    my_token = "hello"
    # get the token first
    WaziGate_url = 'http://'+sys.argv[1]+'/auth/token'
    try:
        pload = '{"username":"admin","password":"loragateway"}'
        response = requests.post(
            WaziGate_url, headers=WaziGate_headers, data=pload, timeout=30)
        # print(response.text)
        WaziGate_headers_auth['Authorization']+=response.text.replace('"','')
    except requests.exceptions.RequestException as e:
        print(e)
        print('get-entry: requests command failed')

    print('=========================================')


    WaziGate_url = BASE_URL+'devices'    
    try:
        response = requests.get(
            WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
    except requests.exceptions.RequestException as e:
        print(e)
        print('get-devices: request 1 command failed')
        sys.exit('Something bad happened')

    try:
        device_json = json.loads(response.text)
    except:
        print('could not parse JSON')
        print(response.text)
        sys.exit('Something bad happened')

    # print(json.dumps(device_json, indent=4))

    if sys.argv[2]=="devices":
        devices={}
        for dev_dash in device_json:
            #discard GWs
            if not (len(dev_dash["actuators"])==0 and len(dev_dash["sensors"])==0):
                dev_ind=len(devices)
                devices[dev_ind]={
                    "id":dev_dash["id"],
                    "name":dev_dash["name"],
                }
                try:
                    devices[dev_ind]["devAddr"]=dev_dash["meta"]["lorawan"]["devAddr"]
                except:
                    devices[dev_ind]["devAddr"]="null"

                devices[dev_ind]["sensors"]=[{
                    "id":sens_dict["id"],
                    "name":sens_dict["name"],
                    "meta_kind":sens_dict["meta"]["kind"] if ("meta" in sens_dict and "kind" in sens_dict["meta"]) else "undefined",
                    "meta_xlppChan":sens_dict["meta"]["xlppChan"] if ("meta" in sens_dict and "xlppChan" in sens_dict["meta"]) else -1,
                } for sens_dict in dev_dash["sensors"]]

                #TODO: actuators idem
        
        json_dict = json.dumps(devices, indent=4)
        with open("devices_to_export.json", "w") as outfile:
            outfile.write(json_dict)

        if len(sys.argv)>3 and sys.argv[3]=="print":
            print(json_dict)


    elif sys.argv[2][-5:]==".json":
        extracted_values={}
        dev_dict={}
        with open(sys.argv[2]) as f:
            dev_dict = json.load(f)
        for dev in dev_dict:
            dev_id=dev_dict[dev]["id"]
            found_dev=False
            for dev_dash in device_json:
                if dev_dash["id"]==dev_id:
                    found_dev=True
                    break
            if found_dev:
                current_dev_id=int(dev)#len(extracted_values)
                extracted_values[current_dev_id]={
                    "devName":dev_dict[dev]["name"],
                    "devAddr":dev_dict[dev]["devAddr"],
                    "devID":dev_dict[dev]["id"],
                    "sensors":[]
                }
                for sens in dev_dict[dev]["sensors"]:
                    sens_id=sens["id"]
                    found_sens=False
                    for sens_dict in dev_dash["sensors"]:
                        if sens_dict["id"]==sens_id:
                            found_sens=True
                            break
                    if found_sens:

                        WaziGate_url = BASE_URL+'devices/'+dev_id+'/sensors/'+sens_id+'/values'    
                        try:
                            response = requests.get(
                                WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
                        except requests.exceptions.RequestException as e:
                            print(e)
                            print('get-values: request 1 command failed')
                            sys.exit('Something bad happened')
            
                        try:
                            sens_values = json.loads(response.text)
                        except:
                            print('could not parse JSON')
                            print(WaziGate_url)
                            print(response.text)
                            sens_values={}

                        pretty=sens["name"] + " / " + sens["meta_kind"].split(',')[0]
                        # pretty=pretty.replace(" ","")
                        extracted_values[current_dev_id]["sensors"].append({
                            "id":sens_id,
                            "name":sens["name"],
                            "pretty_name":pretty,
                            "values":sens_values
                        })

        
        if len(sys.argv)>3:
            if sys.argv[3]=="json":

                json_dict = json.dumps(extracted_values, indent=4)
                with open("full_JSON_export.json", "w") as outfile:
                    outfile.write(json_dict)

            elif sys.argv[3]=="csv_Bondy":

                # exporting to Bondy aggregated CSV format
                with open('extracted_Bondy_aggregated.csv', 'w') as f:
                    aggr_timestamps=[]

                    f.write(',')
                    for device_id in extracted_values:
                        # device=extracted_values[len(extracted_values)-device_id-1]
                        device=extracted_values[device_id]
                        # aggr_timestamps+=[timeval["time"] for timeval in device["temperatureSensor_0"][2:]]
                        f.write(device["devName"])
                        for sensor in ["temperatureSensor_0","temperatureSensor_1","temperatureSensor_2","temperatureSensor_3","temperatureSensor_5","analogInput_6"]:
                            found=False
                            for existing_sensor in device["sensors"]:
                                if existing_sensor["id"]==sensor:
                                    found=True
                                    break
                            if found and len(existing_sensor["values"])>0:
                                f.write(',')
                                aggr_timestamps+=[timeval["time"] for timeval in existing_sensor["values"][2 if sensor[-1] in ["0","1","2","3"] else 1:]]

                    # sort list of timestamps
                    aggr_timestamps = sorted(set(aggr_timestamps))

                    f.write('\n,')

                    for device_id in extracted_values:
                        # for 
                        # device=extracted_values[len(extracted_values)-device_id-1]
                        device=extracted_values[device_id]
                        f.write(device["devID"])
                        for sensor in ["temperatureSensor_0","temperatureSensor_1","temperatureSensor_2","temperatureSensor_3","temperatureSensor_5","analogInput_6"]:
                            found=False
                            for existing_sensor in device["sensors"]:
                                if existing_sensor["id"]==sensor:
                                    found=True
                                    break
                            if found and len(existing_sensor["values"])>0:
                                f.write(',')

                    f.write('\n,')

                    for device_id in extracted_values:
                        # device=extracted_values[len(extracted_values)-device_id-1]
                        device=extracted_values[device_id]
                        for sensor in ["temperatureSensor_0","temperatureSensor_1","temperatureSensor_2","temperatureSensor_3","temperatureSensor_5","analogInput_6"]:
                            found=False
                            for existing_sensor in device["sensors"]:
                                if existing_sensor["id"]==sensor:
                                    found=True
                                    break
                            if found and len(existing_sensor["values"])>0:
                                f.write(existing_sensor["pretty_name"])
                                f.write(',')

                    f.write('\n')


                    for ordered_timestamp in aggr_timestamps:
                        f.write(ordered_timestamp)
                        f.write(',')

                        for device_id in extracted_values:
                            # for 
                            # device=extracted_values[len(extracted_values)-device_id-1]
                            device=extracted_values[device_id]


                            for sensor in ["temperatureSensor_0","temperatureSensor_1","temperatureSensor_2","temperatureSensor_3","temperatureSensor_5","analogInput_6"]:
                                found=False
                                for existing_sensor in device["sensors"]:
                                    if existing_sensor["id"]==sensor:
                                        found=True
                                        break
                                if found and len(existing_sensor["values"])>0:
                                    not_found=True
                                    for timevalpair in existing_sensor["values"][2 if sensor[-1] in ["0","1","2","3"] else 1:]:
                                        if timevalpair["time"]==ordered_timestamp:
                                            f.write(str(timevalpair["value"]))
                                            f.write(',')
                                            not_found=False
                                            break
                                    if not_found:
                                        f.write("n/a")
                                        f.write(',')
                        f.write('\n')


            elif sys.argv[3]=="csv":
                matrix=[]
                max_len=0
                for device_id in extracted_values:
                    device=extracted_values[device_id]
                    row1_ori=[device["devID"],device["devName"]]

                    for existing_sensor in device["sensors"]:
                        row1=row1_ori+[timeval["time"] for timeval in existing_sensor["values"]]
                        row2=[existing_sensor["id"],existing_sensor["pretty_name"]]
                        row2+=[timeval["value"] for timeval in existing_sensor["values"]]

                        max_len=max(max_len,len(row2))
                        matrix+=[row1,row2]

                #pad
                for row in matrix:
                    to_pad=max_len-len(row)
                    if to_pad>0:
                        row+=[""]*to_pad

                #transpose to a csv
                with open('extracted_data.csv', 'w') as f:
                    for col in zip(*matrix):
                        for cell in col:
                            f.write(str(cell))
                            f.write(',')
                        f.write('\n')

    else:
        print("{0} is an unknown argument.".format(sys.argv[2]))

else:
    print("extract_values.py requires arguments, see README")
