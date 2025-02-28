import requests
import json
import sys


#############################
# e.g. python ./iiwa_rest.py devices
# the following "add" only works on devices existing in WG
# e.g. python ./iiwa_rest.py add 62c7c657127dbd00011540a4 SOIL-AREA-1 1_capacitive temperatureSensor_0
# e.g. python ./iiwa_rest.py add 62c7c657127dbd00011540a6 SOIL-AREA-2 1_watermark temperatureSensor_0
# e.g. python ./iiwa_rest.py add 62c7c657127dbd00011540a8 SOIL-AREA-3 2_watermark temperatureSensor_0
# e.g. python ./iiwa_rest.py add 62c7c657127dbd00011540a8 SOIL-AREA-3 2_watermark temperatureSensor_2
# e.g. python ./iiwa_rest.py delete 62c7c657127dbd00011540a6
# e.g. python ./iiwa_rest.py delete all
# e.g. python ./iiwa_rest.py data
# e.g. python ./iiwa_rest.py configs
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
    'Authorization': 'Bearer **'
}
# ---------------------#






if len(sys.argv)>1:
    print('=========================================')

    my_token = "hello"
    # get the token first
    WaziGate_url = 'http://wazigate.local/auth/token'
    try:
        pload = '{"username":"admin","password":"loragateway"}'
        response = requests.post(
            WaziGate_url, headers=WaziGate_headers, data=pload, timeout=30)

    except requests.exceptions.RequestException as e:
        print(e)
        print('get-entry: requests command failed')

    print('=========================================')



    if sys.argv[1]=="devices":

        # WaziGate_url = 'http://localhost/devices/' + device_id + '/sensors/' + sensor_id
        WaziGate_url = 'http://wazigate.local:5000/devices'
        
        try:
            response = requests.get(
                WaziGate_url, headers=WaziGate_headers, timeout=30)
            print(response.text)
            # device_json = json.loads(response.text)
            # print(device_json)            

        except requests.exceptions.RequestException as e:
            print(e)
            print('get-devices: requests command failed')


    if sys.argv[1]=="add":
        # python iiwa_rest.py add dev_id dev_name dev_structure sensor_id
        print(sys.argv)

        WaziGate_url = 'http://wazigate.local:5000/devices/'+sys.argv[2]
        try:
            # pload='{"device_id":"Caramba","device_name":"Fichtre","sensors_structure":"2_watermark"}'
            pload='{"device_id":"'+sys.argv[2]+'","device_name":"'+sys.argv[3]+'","sensors_structure":"'+sys.argv[4]+'"}'
            response = requests.post(
                WaziGate_url, headers=WaziGate_headers_auth, data=pload, timeout=30)

        except requests.exceptions.RequestException as e:
            print(e)
            print('add dev request command failed')

        WaziGate_url = 'http://wazigate.local:5000/devices/'+sys.argv[2]+'/sensors/'+sys.argv[5]

        if "watermark" in sys.argv[4]:
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
                "soil_temperature_device_id": sys.argv[2], 
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


    if sys.argv[1]=="delete":
        if sys.argv[2]=="all":
            WaziGate_url = 'http://wazigate.local:5000/devices'    
            try:
                response = requests.get(
                    WaziGate_url, headers=WaziGate_headers, timeout=30)
                # print(response.text)
                device_json = json.loads(response.text)


            except requests.exceptions.RequestException as e:
                print(e)
                print('delete-all: request 1 command failed')

            for dev_dash in device_json:
                dev_id=dev_dash["device_id"]
                WaziGate_url = 'http://wazigate.local:5000/devices/'+dev_id
                try:
                    response = requests.delete(
                        WaziGate_url, headers=WaziGate_headers, timeout=30)
                    print(response.text)

                except requests.exceptions.RequestException as e:
                    print(e)
                    print('delete-device {0}: request command failed'.format(dev_id))

        else:
            WaziGate_url = 'http://wazigate.local:5000/devices/'+sys.argv[2]
            try:
                response = requests.delete(
                    WaziGate_url, headers=WaziGate_headers, timeout=30)

            except requests.exceptions.RequestException as e:
                print(e)
                print('delete-device: request command failed')


    if sys.argv[1]=="data":
        WaziGate_url = 'http://wazigate.local:5000/devices/data'
        
        try:
            response = requests.get(
                WaziGate_url, headers=WaziGate_headers, timeout=30)
            print(response.text)
            # device_json = json.loads(response.text)
            # print(device_json)            
            

        except requests.exceptions.RequestException as e:
            print(e)
            print('get-data: request command failed')

    if sys.argv[1]=="configs":
        WaziGate_url = 'http://wazigate.local:5000/sensors_configurations'
        
        try:
            response = requests.get(
                WaziGate_url, headers=WaziGate_headers, timeout=30)
            print(response.text)
            # device_json = json.loads(response.text)
            # print(device_json)            
            

        except requests.exceptions.RequestException as e:
            print(e)
            print('get-sensors_configurations: request command failed')            

    print('=========================================')

