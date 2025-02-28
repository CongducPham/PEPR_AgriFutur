import requests
import json
import sys


#############################
# e.g. python ./iiwa_rest_remote.py localhost devices
# the following "add" only works on devices existing in WG
# e.g. python ./iiwa_rest_remote.py wazigate.local add 62c7c657127dbd00011540a4 SOIL-AREA-1 1_capacitive temperatureSensor_0
# e.g. python ./iiwa_rest_remote.py 10.42.0.1 add 62c7c657127dbd00011540a6 SOIL-AREA-2 1_watermark temperatureSensor_0
# e.g. python ./iiwa_rest_remote.py 192.168.0.25 add 62c7c657127dbd00011540a8 SOIL-AREA-3 2_watermark temperatureSensor_0
# e.g. python ./iiwa_rest_remote.py wazigate.local add 62c7c657127dbd00011540a8 SOIL-AREA-3 2_watermark temperatureSensor_2
# e.g. python ./iiwa_rest_remote.py wazigate.local delete 62c7c657127dbd00011540a6
# e.g. python ./iiwa_rest_remote.py wazigate.local delete all
# e.g. python ./iiwa_rest_remote.py wazigate.local data
# e.g. python ./iiwa_rest_remote.py wazigate.local configs
# e.g. python ./iiwa_rest_remote.py 192.168.0.42 update 6454c62e68f319085c988d67 temperatureSensor_0 "{\"weather_weekly_pluviometry\" : \"588\"}"
#############################


BASE_URL = "http://localhost/"

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
    'Authorization': 'Bearer '
}
# ---------------------#






if len(sys.argv)>2:
    print('=========================================')


    my_token = "hello"
    # get the token first
    WaziGate_url = 'http://'+sys.argv[1]+'/auth/token'
    try:
        pload = '{"username":"admin","password":"loragateway"}'
        response = requests.post(
            WaziGate_url, headers=WaziGate_headers, data=pload, timeout=30)
        WaziGate_headers_auth['Authorization']+=response.text.replace('"','')

    except requests.exceptions.RequestException as e:
        print(e)
        print('get-entry: requests command failed')

    print('=========================================')



    if sys.argv[2]=="devices":

        # WaziGate_url = 'http://localhost/devices/' + device_id + '/sensors/' + sensor_id
        WaziGate_url = 'http://'+sys.argv[1]+':5000/devices'
        
        try:
            response = requests.get(
                WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
            print(response.text)
            with open("iiwa_devices.json", "w") as outfile:
                outfile.write(response.text)
            
            # device_json = json.loads(response.text)
            # print(device_json)            

        except requests.exceptions.RequestException as e:
            print(e)
            print('get-devices: requests command failed')


    if sys.argv[2]=="add":
        # python iiwa_rest.py add dev_id dev_name dev_structure sensor_id
        print(sys.argv)

        WaziGate_url = 'http://'+sys.argv[1]+':5000/devices/'+sys.argv[3]
        try:
            # pload='{"device_id":"Caramba","device_name":"Fichtre","sensors_structure":"2_watermark"}'
            pload='{"device_id":"'+sys.argv[3]+'","device_name":"'+sys.argv[4]+'","sensors_structure":"'+sys.argv[5]+'"}'
            response = requests.post(
                WaziGate_url, headers=WaziGate_headers_auth, data=pload, timeout=30)

        except requests.exceptions.RequestException as e:
            print(e)
            print('add dev request command failed')

        WaziGate_url = 'http://'+sys.argv[1]+':5000/devices/'+sys.argv[3]+'/sensors/'+sys.argv[6]

        if "watermark" in sys.argv[5]:
            pload= json.dumps({ 
                "sensor_type": "tensiometer_cbar", 
                "sensor_age": "0", 
                "sensor_max_value": "124", 
                "sensor_min_value": "0", 
                "soil_type": "silty", 
                "soil_irrigation_type": "furrow", 
                "soil_salinity": "undefined", 
                "soil_bulk_density": "undefined",
                "soil_field_capacity":"undefined", 
                "soil_temperature_value": "undefined", 
                "soil_temperature_device_id": sys.argv[3], 
                "soil_temperature_sensor_id": "temperatureSensor_5", 
                "plant_category":"undefined",
                "plant_type":"undefined",
                "plant_variety":"undefined",
                "plant_planting_date": "undefined", 
                "weather_region": "semi-arid", 
                "weather_weekly_evaporation":"undefined",
                "weather_weekly_pluviometry":"undefined"
                # "device_id": "Caramba", 
                # "sensor_id": "temperatureSensor_0" 
            })
        else:
            pload= json.dumps({ 
                "sensor_type": "capacitive", 
                "sensor_age": "0", 
                "sensor_max_value": "800", 
                "sensor_min_value": "0", 
                "soil_type": "silty", 
                "soil_irrigation_type": "furrow", 
                "soil_salinity": "undefined", 
                "soil_bulk_density": "undefined",
                "soil_field_capacity":"undefined", 
                "soil_temperature_value": "undefined", 
                "soil_temperature_device_id": "undefined", 
                "soil_temperature_sensor_id": "undefined", 
                "plant_category":"undefined",
                "plant_type":"undefined",
                "plant_variety":"undefined",
                "plant_planting_date": "undefined", 
                "weather_region": "semi-arid", 
                "weather_weekly_evaporation":"undefined",
                "weather_weekly_pluviometry":"undefined"
                # "device_id": "Caramba", 
                # "sensor_id": "temperatureSensor_0" 
            })
            # print(pload)
            # print(0/0)

        try:
            response = requests.post(
                WaziGate_url, headers=WaziGate_headers_auth, data=pload, timeout=30)

        except requests.exceptions.RequestException as e:
            print(e)
            print('add sensors: request command failed')


    if sys.argv[2]=="delete":
        if sys.argv[3]=="all":
            WaziGate_url = 'http://'+sys.argv[1]+':5000/devices'    
            try:
                response = requests.get(
                    WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
                # print(response.text)
                device_json = json.loads(response.text)


            except requests.exceptions.RequestException as e:
                print(e)
                print('delete-all: request 1 command failed')

            for dev_dash in device_json:
                dev_id=dev_dash["device_id"]
                WaziGate_url = 'http://'+sys.argv[1]+':5000/devices/'+dev_id
                try:
                    response = requests.delete(
                        WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
                    print(response.text)

                except requests.exceptions.RequestException as e:
                    print(e)
                    print('delete-device {0}: request command failed'.format(dev_id))

        else:
            WaziGate_url = 'http://'+sys.argv[1]+':5000/devices/'+sys.argv[3]
            try:
                response = requests.delete(
                    WaziGate_url, headers=WaziGate_headers_auth, timeout=30)

            except requests.exceptions.RequestException as e:
                print(e)
                print('delete-device: request command failed')


    if sys.argv[2]=="data":
        WaziGate_url = 'http://'+sys.argv[1]+':5000/devices/data'
        
        try:
            response = requests.get(
                WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
            print(response.text)
            with open("iiwa_data.json", "w") as outfile:
                outfile.write(response.text)

            # device_json = json.loads(response.text)
            # print(device_json)            
            

        except requests.exceptions.RequestException as e:
            print(e)
            print('get-data: request command failed')

    if sys.argv[2]=="configs":
        WaziGate_url = 'http://'+sys.argv[1]+':5000/sensors_configurations'
        
        try:
            response = requests.get(
                WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
            print(response.text)
            with open("iiwa_sensors_configurations.json", "w") as outfile:
                outfile.write(response.text)
            # device_json = json.loads(response.text)
            # print(device_json)            
            

        except requests.exceptions.RequestException as e:
            print(e)
            print('get-sensors_configurations: request command failed')

    if sys.argv[2]=="update":
        WaziGate_url = 'http://'+sys.argv[1]+':5000/sensors_configurations'
        configs_json={}
        new_conf={}

        try:
            response = requests.get(
                WaziGate_url, headers=WaziGate_headers_auth, timeout=30)
            configs_json = json.loads(response.text)


        except requests.exceptions.RequestException as e:
            print(e)
            print('get-sensors_configurations: request command failed')

        found=False
        for sens_dict in configs_json["sensors"]:
            if sens_dict["device_id"]==sys.argv[3] and sens_dict["sensor_id"]==sys.argv[4]:
                found=True
                new_conf.update(sens_dict["value"])
                new_conf.update(sens_dict["soil_temperature_source"])
                break

        if found:
            wished_update=json.loads(sys.argv[5])
            new_conf.update(wished_update)
            pload= json.dumps(new_conf)

            WaziGate_url = 'http://'+sys.argv[1]+':5000/devices/'+sys.argv[3]+'/sensors/'+sys.argv[4]
            try:
                response = requests.post(
                    WaziGate_url, headers=WaziGate_headers_auth, data=pload, timeout=30)

            except requests.exceptions.RequestException as e:
                print(e)
                print('add sensors: request command failed')




    print('=========================================')

