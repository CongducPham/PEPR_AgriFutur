"""
Based on example from Adafruit.

INTEL-IRRIS oled service. Author: C. Pham. Last update April 16th, 2022.

Called by /etc/systemd/system/intel-irris-oled-service.service

some useful commands:

- sudo systemctl stop intel-irris-oled-service.service
- systemctl status intel-irris-oled-service.service
- journalctl -u intel-irris-oled-service.service
- sudo systemctl daemon-reload

"""

import sys
import time
#import subprocess
import os
import os.path
import signal
import requests
from datetime import datetime
import random
import json

import board
import digitalio
from PIL import Image, ImageDraw, ImageFont
import adafruit_ssd1306

#import pyqrcode
#import png
#from pyqrcode import QRCode

## String which represents the QR code
#s = "WIFI:S:WAZIGATE_B827EBD1B236;T:WPA2;P:loragateway;;"

## Generate QR code
#url = pyqrcode.create(s)

## Create and save the png file 
#url.png('/home/pi/oled/wifi.png', scale = 6)

#-------------------------------------------------------------------------
# information related to the soil sensor device
#-------------------------------------------------------------------------

if os.path.isfile('/boot/new_key_device.py'):
	sys.path.append('/boot')
	import new_key_device as key_device
else:	
	import key_device
#use default at beginning
sensor_type=key_device.sensor_type
sensor_model=key_device.sensor_model
device_id_for_oled=key_device.device_id_for_oled
sensor_id_for_oled='temperatureSensor_0'
has_found_device=False

#-------------------------------------------------------------------------
# WaziGate API configuration
#-------------------------------------------------------------------------

#common header for requests
WaziGate_headers = {'accept':'application/json','content-type':'application/json'}
WaziGate_headers_auth = {'accept':'application/json','content-type':'application/json','Authorization':'Bearer **'}

#-------------------------------------------------------------------------
# OLED configuration
#-------------------------------------------------------------------------

lang=key_device.lang

#uncomment to overwrite
#lang='fr'

if lang=='fr':
	screen_saver_str_1=" jours"
	main_screen_str_1=" jours"
	main_screen_str_2=""
	main_screen_str_3=" mins"
	main_screen_str_4="valeur: "
	main_screen_str_4cb="cbar: "			
	main_screen_str_5="sol est "			
else:
	screen_saver_str_1=" days"
	main_screen_str_1=" days ago"
	main_screen_str_2="ago"
	main_screen_str_3=" mins ago"
	main_screen_str_4="raw: "
	main_screen_str_4cb="cbar: "		
	main_screen_str_5="soil is "	
			
# reset oled at each main screen cycle to handle screen shut off by other services such as wazigate-system
always_init_oled = True

# Define the Reset Pin
oled_reset = None

# Change these
# to the right size for your display!
WIDTH = 128
HEIGHT = 64  # Change to 64 if needed
BORDER = 2

# Use for I2C.
try:
	i2c = board.I2C()
except OSError:	
	print ('oled-service: error with I2C bus')
	sys.exit(1)
	#while True:
	#	time.sleep(1)

try:
	oled = adafruit_ssd1306.SSD1306_I2C(WIDTH, HEIGHT, i2c, addr=0x3C, reset=oled_reset)
except ValueError:	
	print ('oled-service: error with OLED address')
	sys.exit(2)
	#while True:
	#	time.sleep(1)
		
font_size = 10
font_saver_size = 12
visual_bar_height = 14

# Load default font.
font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", font_size)
font_saver = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", font_saver_size)
#font = ImageFont.load_default()

# Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
image = Image.new("1", (oled.width, oled.height))

# Get drawing object to draw on image.
draw = ImageDraw.Draw(image)

image2 = Image.open("/home/pi/oled/wifi.png")
image_r = image2.resize((oled.height,oled.height), Image.BICUBIC)
image_bw = image_r.convert("1")
	
#-------------------------------------------------------------------------
# oled-service configuration
#-------------------------------------------------------------------------

#set to 0 to disable wifi qrcode
oled_wifi_qrcode_display_duration=8

if key_device.cyclic_show_all_device:
	#duration of screen saver mode
	oled_display_timer=12
	#duration of the full information screen
	oled_display_duration=5
else:
	#duration of screen saver mode
	oled_display_timer=30
	#duration of the full information screen
	oled_display_duration=6	

device_name_for_oled='undef'
all_devices_id_list=[]
last_raw_value=0
last_raw_value_minutes=0
last_raw_value_days=0
value_index_capacitive=key_device.capacitive_sensor_n_interval-1
value_index_tensiometer=key_device.tensiometer_sensor_n_interval-1
capacitive_soil_condition=key_device.capacitive_sensor_soil_condition[key_device.capacitive_sensor_n_interval-1]
tensiometer_soil_condition=key_device.tensiometer_sensor_soil_condition[key_device.tensiometer_sensor_n_interval-1]
value_index_from_iiwa=False		

#-------------------------------------------------------------------------
# check if default device id is valid
#-------------------------------------------------------------------------

def check_for_device(device_id):
	global has_found_device
	has_found_device=False
	print ('oled-service: check for device '+device_id)
	WaziGate_url='http://localhost/devices/'+device_id
	try:
		response = requests.get(WaziGate_url, headers=WaziGate_headers, timeout=30)
		print ('oled-service: returned msg from server is '),
		print (response.status_code)
		print (response.reason)
		
		if 200 <= response.status_code < 300:
			print ('oled-service: GET success')
			print (response.text)
			if "not found" in response.text:
				has_found_device=False
			else:
				device_json=json.loads(response.text)
				global device_name_for_oled
				device_name_for_oled=device_json["name"][0:24]
				has_found_device=True
		else:
			print ('oled-service: bad request')
			print (response.text)			
			
	except requests.exceptions.RequestException as e:
		print (e)
		print ('oled-service: requests command failed')	
	
	return has_found_device
	 
	print ('=========================================')	

#-------------------------------------------------------------------------
# set meta data for new default sensor
#-------------------------------------------------------------------------

def set_meta_data(device_id):
	my_token="hello"
	#get the token first
	WaziGate_url='http://localhost/auth/token'
	try:
		pload = '{"username":"admin","password":"loragateway"}'
		response = requests.post(WaziGate_url, headers=WaziGate_headers, data=pload, timeout=30)
		print ('oled-service: returned msg from server is '),
		print (response.status_code)
		print (response.reason)
	
		if 200 <= response.status_code < 300:
			print ('oled-service: POST success')
			print (response.text)
			my_token=response.text
		else:
			print ('oled-service: bad request')
			print (response.text)			
		
	except requests.exceptions.RequestException as e:
		print (e)
		print ('oled-service: requests command failed')

	print ('=========================================')
				
	WaziGate_url='http://localhost/devices/'+device_id+'/sensors/temperatureSensor_0/meta'
	try:
		if sensor_model=="WM200":
			#pload = '{"model":"WM200","sensor_dry_max":8000,"sensor_n_interval":6,"sensor_wet_max":0,"type":"tensiometer","value_index":0}'
			pload = '{"model":"WM200","sensor_dry_max":120,"sensor_n_interval":6,"sensor_wet_max":0,"type":"tensiometer"}'
		else:
			#pload = '{"model":"SEN0308","sensor_dry_max":800,"sensor_n_interval":6,"sensor_wet_max":0,"type":"capacitive","value_index":0}'
			pload = '{"model":"SEN0308","sensor_dry_max":800,"sensor_n_interval":6,"sensor_wet_max":0,"type":"capacitive"}'
		
		WaziGate_headers_auth['Authorization']='Bearer'+my_token[1:-2]
		response = requests.post(WaziGate_url, headers=WaziGate_headers_auth, data=pload, timeout=30)
		print ('oled-service: returned msg from server is '),
		print (response.status_code)
		print (response.reason)
	
		if 200 <= response.status_code < 300:
			print ('oled-service: POST success')
			print (response.text)
		else:
			print ('oled-service: bad request')
			print (response.text)			
		
	except requests.exceptions.RequestException as e:
		print (e)
		print ('oled-service: requests command failed')
		
	print ('=========================================')	


#-------------------------------------------------------------------------
# get list of all devices
#-------------------------------------------------------------------------

def get_all_devices_id_list():
	print ('oled-service: get list of all devices')
	global has_found_device
	has_found_device=False	
	global all_devices_id_list
	all_devices_id_list=[]
	global sensor_type
	global sensor_model	
	
	WaziGate_url='http://localhost/devices'
	try:
		response = requests.get(WaziGate_url, headers=WaziGate_headers, timeout=30)
		print ('oled-service: returned msg from server is '),
		print (response.status_code)
		print (response.reason)
		
		if 200 <= response.status_code < 300:
			print ('oled-service: GET success')
			print (response.text)
			device_json=json.loads(response.text)
			print (len(device_json))
			
			if len(device_json) >= 2:
				for i in range (1,len(device_json)):
					default_device_json=device_json[i]
					print (default_device_json)
					
					sensor_type="undef"
					sensor_model="undef"
					
					sensor_json=default_device_json['sensors']
					print (len(sensor_json))
					
					if len(sensor_json)!=0:
						if str(default_device_json['sensors'][0]['value'])=='None':
							print ('oled-service: value is null, skip device')
						else:	
							sensor_name=default_device_json['sensors'][0]['name']
							if 'kind' in default_device_json["sensors"][0]["meta"]:
								sensor_meta_kind=default_device_json["sensors"][0]["meta"]["kind"]
							else:
								sensor_meta_kind='no kind in meta'				
			
							if "SEN0308" in sensor_name or "SEN0308" in sensor_meta_kind: 
								sensor_type="capacitive"
								sensor_model="SEN0308"
				
							if "WM200" in sensor_name or "WM200" in sensor_meta_kind:
								sensor_type="tensiometer"
								sensor_model="WM200"			
				
							print(sensor_model, sensor_type)
							
							if sensor_type!='undef':
								device_id=default_device_json["id"]
								print (device_id)
					
								print ('oled-service: add in device id list')
								all_devices_id_list.append(device_id)
					
								has_found_device=True	
				
								set_meta_data(device_id)
							else:
								print ('oled-service: unknown sensor type, skip device')		
					else:
						print ('oled-service: device has no sensor defined, skip device')	
		else:
			print ('oled-service: bad request')
			print (response.text)			
			
	except requests.exceptions.RequestException as e:
		print (e)
		print ('oled-service: requests command failed')	
	
	print ('=========================================')		
	
#-------------------------------------------------------------------------
# find new default device
#-------------------------------------------------------------------------

def find_new_default_device():
	print ('oled-service: try to find new default device')
	global has_found_device
	has_found_device=False
	global sensor_type
	global sensor_model
	
	WaziGate_url='http://localhost/devices'
	try:
		response = requests.get(WaziGate_url, headers=WaziGate_headers, timeout=30)
		print ('oled-service: returned msg from server is '),
		print (response.status_code)
		print (response.reason)
		
		if 200 <= response.status_code < 300:
			print ('oled-service: GET success')
			print (response.text)
			device_json=json.loads(response.text)
			print (len(device_json))
			
			if len(device_json) >= 2:
				default_device_json=device_json[len(device_json)-1]
				print (default_device_json)
				
				sensor_type="undef"
				sensor_model="undef"
				
				sensor_json=default_device_json['sensors']
				print (len(sensor_json))
				
				if len(sensor_json)!=0:
					if str(default_device_json['sensors'][0]['value'])=='None':
						print ('oled-service: value is null, skip device')
					else:				
						sensor_name=default_device_json['sensors'][0]['name']
						if 'kind' in default_device_json["sensors"][0]["meta"]:
							sensor_meta_kind=default_device_json["sensors"][0]["meta"]["kind"]
						else:
							sensor_meta_kind='no kind in meta'	
			
						if "SEN0308" in sensor_name or "SEN0308" in sensor_meta_kind: 
							sensor_type="capacitive"
							sensor_model="SEN0308"
				
						if "WM200" in sensor_name or "WM200" in sensor_meta_kind:
							sensor_type="tensiometer"
							sensor_model="WM200"			
				
						print(sensor_model, sensor_type)

						if sensor_type!='undef':
							global device_id_for_oled
							device_id_for_oled=default_device_json["id"]
							print (device_id_for_oled)
				
							has_found_device=True	
				
							set_meta_data(device_id_for_oled)
						else:
							print ('oled-service: unknown sensor type, skip device')		
				else:
					print ('oled-service: device has no sensor defined, skip device')		
		else:
			print ('oled-service: bad request')
			print (response.text)			
			
	except requests.exceptions.RequestException as e:
		print (e)
		print ('oled-service: requests command failed')	
	
	print ('=========================================')		

#-------------------------------------------------------------------------
# get sensor type and model from local database
#-------------------------------------------------------------------------

def get_sensor_type_from_local_database(device_id):
	global sensor_type
	global sensor_model
	
	WaziGate_url='http://localhost/devices/'+device_id+'/sensors/temperatureSensor_0'
	try:
		response = requests.get(WaziGate_url, headers=WaziGate_headers, timeout=30)
		print ('oled-service: returned msg from server is '),
		print (response.status_code)
		print (response.reason)
		
		if 200 <= response.status_code < 300:
			print ('oled-service: GET success')
			print (response.text)
			device_json=json.loads(response.text)
			sensor_type=device_json["meta"]["type"]
			sensor_model=device_json["meta"]["model"]
			print(sensor_model, sensor_type)
		else:
			print ('oled-service: bad request')
			print (response.text)			
			
	except requests.exceptions.RequestException as e:
		print (e)
		print ('oled-service: requests command failed')
		
	print ('=========================================')	
		
#-------------------------------------------------------------------------
# set sensor intervals
#-------------------------------------------------------------------------		
		
def set_sensor_intervals():
	global last_raw_value
	global last_raw_value_minutes
	global last_raw_value_days
	global value_index_capacitive
	global value_index_tensiometer
	global capacitive_soil_condition
	global tensiometer_soil_condition
	
	if sensor_type=='capacitive':
		last_raw_value=key_device.capacitive_sensor_dry_max
	else:
		last_raw_value=key_device.tensiometer_sensor_wet_max	

	last_raw_value_minutes=0
	last_raw_value_days=0

	value_index_capacitive=key_device.capacitive_sensor_n_interval-1
	value_index_tensiometer=key_device.tensiometer_sensor_n_interval-1

	capacitive_soil_condition=key_device.capacitive_sensor_soil_condition[key_device.capacitive_sensor_n_interval-1]
	tensiometer_soil_condition=key_device.tensiometer_sensor_soil_condition[key_device.tensiometer_sensor_n_interval-1]			

#-------------------------------------------------------------------------
# init sensor data for OLED
#-------------------------------------------------------------------------

if check_for_device(device_id_for_oled):
	print ('oled-service: found device')		
else:
	print ('oled-service: device not found!')
	find_new_default_device()
	
if has_found_device:	
	if key_device.get_sensor_type_from_local_database:
		print ('oled-service: get sensor type and model from local database')
		get_sensor_type_from_local_database(device_id_for_oled)
	else:
		print ('oled-service: get sensor type and model from key_device.py')
		sensor_type=key_device.sensor_type
		sensor_model=key_device.sensor_model			

print(sensor_model, sensor_type)

if sensor_type=='capacitive':
	last_raw_value=key_device.capacitive_sensor_dry_max
if sensor_type=='tensiometer':
	last_raw_value=key_device.tensiometer_sensor_wet_max
else:		
	last_raw_value=key_device.capacitive_sensor_dry_max

last_raw_value_minutes=0
last_raw_value_days=0

value_index_capacitive=key_device.capacitive_sensor_n_interval-1
value_index_tensiometer=key_device.tensiometer_sensor_n_interval-1

capacitive_soil_condition=key_device.capacitive_sensor_soil_condition[key_device.capacitive_sensor_n_interval-1]
tensiometer_soil_condition=key_device.tensiometer_sensor_soil_condition[key_device.tensiometer_sensor_n_interval-1]		

#-------------------------------------------------------------------------
# Interruption handler
#-------------------------------------------------------------------------

def signal_handler(sig, frame):
	print('You pressed Ctrl+C!')
	# Clear display.
	oled.fill(0)
	oled.show()	
	sys.exit(3)

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

#-------------------------------------------------------------------------
# display at random position the summary, avoiding fix position on the OLED
#-------------------------------------------------------------------------

def screen_saver(duration):
	
	# Clear display.
	oled.fill(0)
	oled.show()
		
	min_x=2*BORDER
	max_x=oled.width - 7*font_saver_size
	min_y=2*BORDER
	max_y=oled.height - 5*font_saver_size
	
	# Get drawing object to draw on image.
	#draw = ImageDraw.Draw(image)	
	
	start_time = time.time()
	end_time=start_time
	
	while (end_time-start_time < duration):
		# Draw a black background
		draw.rectangle((0, 0, oled.width, oled.height), outline=0, fill=0)	
		x=random.randint(min_x, max_x)
		y=random.randint(min_y, max_y)
		ytop=y
		
		if has_found_device:
			text_to_display=device_name_for_oled[0:12]
			draw.text((x, ytop), text_to_display, font=font_saver, fill=255)
			ytop=ytop+font_saver_size
		
			draw.text((x, ytop), str(last_raw_value), font=font_saver, fill=255)
				
			ytop=ytop+font_saver_size
					
			if last_raw_value_days:
				text_to_display=str(last_raw_value_days)+screen_saver_str_1
			else:
				if last_raw_value_minutes > 60:
					text_to_display=str(int(last_raw_value_minutes/60))+"h"+str(last_raw_value_minutes-int(last_raw_value_minutes/60)*60)
				else:				
					text_to_display=str(last_raw_value_minutes)+"mins"
						
			draw.text((x, ytop), text_to_display, font=font_saver, fill=255)
			ytop=ytop+font_saver_size
				
			if sensor_type=='capacitive':
				draw.text((x, ytop), capacitive_soil_condition, font=font_saver, fill=255)
			else:
				draw.text((x, ytop), tensiometer_soil_condition, font=font_saver, fill=255)	

			ytop=ytop+font_saver_size
			
			if value_index_from_iiwa==True:
				draw.text((x, ytop), "IIWA", font=font_saver, fill=255)	
			
			# Draw a white rectangle
			draw.rectangle((x+67, y+14, x+67+15, y+4+3*visual_bar_height), outline=255, fill=0)

			if sensor_type=='capacitive':
				if value_index_capacitive==0:
					draw.text((x+67+4, y+20), "!", font=font_saver, fill=255)
				elif value_index_capacitive<0:	
					draw.text((x+67+4, y+20), "?", font=font_saver, fill=255)					
				else:
					for i in range(value_index_capacitive):
						draw.rectangle((x+67+2, y+4+3*visual_bar_height-6-i*6, x+67+15-2, y+4+3*visual_bar_height-2-i*6), outline=255, fill=255)

			if sensor_type=='tensiometer':
				if value_index_tensiometer==0:
					draw.text((x+67+4, y+20), "!", font=font_saver, fill=255)
				elif value_index_tensiometer<0:	
					draw.text((x+67+4, y+20), "?", font=font_saver, fill=255)
				else:
					for i in range(value_index_tensiometer):
						draw.rectangle((x+67+2, y+4+3*visual_bar_height-6-i*6, x+67+15-2, y+4+3*visual_bar_height-2-i*6), outline=255, fill=255)
												
		else:
			draw.text((x, y), "no device, create one", font=font, fill=255)
			draw.text((x, y+1*font_saver_size), "w/ SEN0308 or WM200", font=font, fill=255)		
			draw.text((x, y+2*font_saver_size), "in sensor name", font=font, fill=255)
			draw.text((x, y+3*font_saver_size), "or sensor kind", font=font, fill=255)
				
		# Display image
		oled.image(image)
		oled.show()		
		time.sleep(2)
		oled.fill(0)
		oled.show()
		end_time = time.time()
		
#--------------------------------------------------------------------------
#get last stored value for a given device id: key_device.device_id_for_oled
#--------------------------------------------------------------------------		
						
def get_last_raw_value(device_id):
	WaziGate_url='http://localhost/devices/'+device_id+'/sensors/temperatureSensor_0'
	try:
		response = requests.get(WaziGate_url, headers=WaziGate_headers, timeout=30)
		print ('oled-service: returned msg from server is '),
		print (response.status_code)
		print (response.reason)
		
		if 200 <= response.status_code < 300:
			print ('oled-service: GET success')
			print (response.text)
			device_json=json.loads(response.text)
			global last_raw_value
			last_raw_value=device_json["value"]
			print(last_raw_value, device_json["time"])

			fmt1 = '%Y-%m-%dT%H:%M:%S.%f'
			fmt2 = '%Y-%m-%dT%H:%M:%S.%fZ'
			fmt3 = '%Y-%m-%dT%H:%M:%SZ'
			d1 = datetime.strptime(datetime.utcnow().isoformat(), fmt1)
			#here time stored in database is in UTC
			if "." in device_json["time"]:
				d2 = datetime.strptime(device_json["time"], fmt2)
			else:
				d2 = datetime.strptime(device_json["time"], fmt3)

			diff = d1-d2
			global last_raw_value_days
			last_raw_value_days=diff.days
			
			global last_raw_value_minutes
			# convert days to minutes
			last_raw_value_minutes = int(diff.days * 24 * 60 + (diff.seconds/60))
			
			print (str(last_raw_value_minutes)+"mins")
			
		else:
			print ('oled-service: bad request')
			print (response.text)			
			
	except requests.exceptions.RequestException as e:
		print (e)
		print ('oled-service: requests command failed')
		
	print ('=========================================')	

#--------------------------------------------------------------------------
#determine the soil condition string indication for capacitive
#--------------------------------------------------------------------------

def get_capacitive_soil_condition(device_id, raw_value):

	global value_index_capacitive
	global value_index_from_iiwa
	
	value_index_from_iiwa=False
	
	#we first compute value_index on our own
	value_interval=int(key_device.capacitive_sensor_dry_max/key_device.capacitive_sensor_n_interval)
	value_index_capacitive=int(raw_value/value_interval)	
	#in case the sensed value is greater than the maximum value defined
	if value_index_capacitive >= key_device.capacitive_sensor_n_interval:
		value_index_capacitive = key_device.capacitive_sensor_n_interval-1	
			
	#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-saturated
	#so for capacitive we need to invert the index
	value_index_capacitive=key_device.capacitive_sensor_n_interval-1-value_index_capacitive
	
	print ('oled-service: value_index_capacitive is ', value_index_capacitive)	

	if key_device.set_value_index_in_local_database:
		my_token="hello"
		#get the token first
		WaziGate_url='http://localhost/auth/token'
		try:
			pload = '{"username":"admin","password":"loragateway"}'
			response = requests.post(WaziGate_url, headers=WaziGate_headers, data=pload, timeout=30)
			print ('oled-service: returned msg from server is '),
			print (response.status_code)
			print (response.reason)
		
			if 200 <= response.status_code < 300:
				print ('oled-service: POST success')
				print (response.text)
				my_token=response.text
			else:
				print ('oled-service: bad request')
				print (response.text)			
			
		except requests.exceptions.RequestException as e:
			print (e)
			print ('oled-service: requests command failed')
		
		print ('=========================================')	
				
		WaziGate_url='http://localhost/devices/'+device_id+'/sensors/temperatureSensor_0/meta'
		try:
			pload = '{"value_index":' + str(value_index_capacitive)+'}'
			WaziGate_headers_auth['Authorization']='Bearer'+my_token[1:-2]
			response = requests.post(WaziGate_url, headers=WaziGate_headers_auth, data=pload, timeout=30)
			print ('oled-service: returned msg from server is '),
			print (response.status_code)
			print (response.reason)
		
			if 200 <= response.status_code < 300:
				print ('oled-service: POST success')
				print (response.text)
			else:
				print ('oled-service: bad request')
				print (response.text)			
			
		except requests.exceptions.RequestException as e:
			print (e)
			print ('oled-service: requests command failed')
			
		print ('=========================================')
		
	if key_device.get_value_index_from_local_database:
		WaziGate_url='http://localhost/devices/'+device_id+'/sensors/temperatureSensor_0'
		try:
			response = requests.get(WaziGate_url, headers=WaziGate_headers, timeout=30)
			print ('oled-service: returned msg from server is '),
			print (response.status_code)
			print (response.reason)
		
			if 200 <= response.status_code < 300:
				print ('oled-service: GET success')
				print (response.text)
				device_json=json.loads(response.text)
				
				if "value_index_iiwa" in response.text:
					if "." in device_json["time"]:
						d_value = datetime.strptime(device_json["time"], '%Y-%m-%dT%H:%M:%S.%fZ')
					else:
						d_value = datetime.strptime(device_json["time"], '%Y-%m-%dT%H:%M:%SZ')

					d_iiwa = datetime.strptime(device_json["meta"]["value_index_iiwa_time"], '%Y-%m-%dT%H:%M:%S.%fZ')
					
					#check that value_index_iiwa is a recent value, in case IIWA is not running anymore	
					if d_iiwa >= d_value:
						print('oled-service: value_index_iiwa date is newer than last value date')
						value_index_capacitive=device_json["meta"]["value_index_iiwa"]
						print('oled-service: using value_index_iiwa')
						value_index_from_iiwa=True
						print(value_index_capacitive)
					else:
						print('oled-service: value_index_iiwa date is older than last value date')
						print('oled-service: IIWA may not be running')						
			else:
				print ('oled-service: bad request')
				print (response.text)			
			
		except requests.exceptions.RequestException as e:
			print (e)
			print ('oled-service: requests command failed')
		
		print ('=========================================')	
					
	global capacitive_soil_condition
	capacitive_soil_condition=key_device.capacitive_sensor_soil_condition[value_index_capacitive]

#--------------------------------------------------------------------------
#determine the soil condition string indication for tensiometer
#--------------------------------------------------------------------------
	
def get_tensiometer_soil_condition(device_id, raw_value): 

	global value_index_tensiometer
	global value_index_from_iiwa
	
	value_index_from_iiwa=False	
	
	#we first compute value_index on our own
	if key_device.use_irrometer_interval_for_tensiometer:
		#from irrometer: https://www.irrometer.com/basics.html
		#0-10 Centibars = Saturated soil
		#10-30 Centibars = Soil is adequately wet (except coarse sands, which are drying)
		#30-60 Centibars = Usual range for irrigation (most soils)
		#60-100 Centibars = Usual range for irrigation in heavy clay
		#100-200 Centibars = Soil is becoming dangerously dry- proceed with caution!
		#	
		#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-very wet/saturated
		
		if raw_value == 255:
			value_index_tensiometer=-1
		elif raw_value == 240:
			value_index_tensiometer=-2		 			
		elif raw_value > 100:
			value_index_tensiometer=0	 
		elif raw_value > 60:
			value_index_tensiometer=1	
		elif raw_value > 30:
			value_index_tensiometer=2	
		elif raw_value > 10:
			value_index_tensiometer=4	
		else:
			value_index_tensiometer=5												
	else:
		value_interval=int(key_device.tensiometer_sensor_dry_max/key_device.tensiometer_sensor_n_interval)
		value_index_tensiometer=int(raw_value/value_interval)
		#in case the sensed value is greater than the maximum value defined
		if value_index_tensiometer >= key_device.tensiometer_sensor_n_interval:
			value_index_tensiometer = key_device.tensiometer_sensor_n_interval-1		
		
		#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-very wet/saturated
		#so for tensiometer we need to invert the index
		value_index_tensiometer=key_device.tensiometer_sensor_n_interval-1-value_index_tensiometer	
		
	print ('oled-service: value_index_tensiometer is ', value_index_tensiometer)	

	if key_device.set_value_index_in_local_database:
		my_token="hello"
		#get the token first
		WaziGate_url='http://localhost/auth/token'
		try:
			pload = '{"username":"admin","password":"loragateway"}'
			response = requests.post(WaziGate_url, headers=WaziGate_headers, data=pload, timeout=30)
			print ('oled-service: returned msg from server is '),
			print (response.status_code)
			print (response.reason)
		
			if 200 <= response.status_code < 300:
				print ('oled-service: POST success')
				print (response.text)
				my_token=response.text
			else:
				print ('oled-service: bad request')
				print (response.text)			
			
		except requests.exceptions.RequestException as e:
			print (e)
			print ('oled-service: requests command failed')
		
		print ('=========================================')	
				
		WaziGate_url='http://localhost/devices/'+device_id+'/sensors/temperatureSensor_0/meta'
		try:
			pload = '{"value_index":' + str(value_index_tensiometer)+'}'
			WaziGate_headers_auth['Authorization']='Bearer'+my_token[1:-2]
			response = requests.post(WaziGate_url, headers=WaziGate_headers_auth, data=pload, timeout=30)
			print ('oled-service: returned msg from server is '),
			print (response.status_code)
			print (response.reason)
		
			if 200 <= response.status_code < 300:
				print ('oled-service: POST success')
				print (response.text)
			else:
				print ('oled-service: bad request')
				print (response.text)			
			
		except requests.exceptions.RequestException as e:
			print (e)
			print ('oled-service: requests command failed')
			
		print ('=========================================')
		
	if key_device.get_value_index_from_local_database:
		WaziGate_url='http://localhost/devices/'+device_id+'/sensors/temperatureSensor_0'
		try:
			response = requests.get(WaziGate_url, headers=WaziGate_headers, timeout=30)
			print ('oled-service: returned msg from server is '),
			print (response.status_code)
			print (response.reason)
		
			if 200 <= response.status_code < 300:
				print ('oled-service: GET success')
				print (response.text)
				device_json=json.loads(response.text)
				
				if "value_index_iiwa" in response.text:
					if "." in device_json["time"]:
						d_value = datetime.strptime(device_json["time"], '%Y-%m-%dT%H:%M:%S.%fZ')
					else:
						d_value = datetime.strptime(device_json["time"], '%Y-%m-%dT%H:%M:%SZ')
					d_iiwa = datetime.strptime(device_json["meta"]["value_index_iiwa_time"], '%Y-%m-%dT%H:%M:%S.%fZ')
					
					#check that value_index_iiwa is a recent value, in case IIWA is not running anymore	
					if d_iiwa >= d_value:
						print('oled-service: value_index_iiwa date is newer than last value date')
						value_index_tensiometer=device_json["meta"]["value_index_iiwa"]
						print('oled-service: using value_index_iiwa')
						value_index_from_iiwa=True
						print(value_index_tensiometer)
					else:
						print('oled-service: value_index_iiwa date is older than last value date')
						print('oled-service: IIWA may not be running')
			else:
				print ('oled-service: bad request')
				print (response.text)			
			
		except requests.exceptions.RequestException as e:
			print (e)
			print ('oled-service: requests command failed')
		
		print ('=========================================')	
						
	global tensiometer_soil_condition
	if value_index_tensiometer==-1:
		tensiometer_soil_condition='no sensor'
	elif value_index_tensiometer==-2:
		tensiometer_soil_condition='err'
	else:			
		tensiometer_soil_condition=key_device.tensiometer_sensor_soil_condition[value_index_tensiometer]	
										
#------------------------------------------------------------
#main loop
#------------------------------------------------------------

device_index=0
	
while True:

	if always_init_oled:
		i2c = board.I2C()
		oled = adafruit_ssd1306.SSD1306_I2C(WIDTH, HEIGHT, i2c, addr=0x3C, reset=oled_reset)
		
	# Clear display.
	oled.fill(0)
	oled.show()

	# Draw a white background
	draw.rectangle((0, 0, oled.width, oled.height), outline=255, fill=255)
	
	if oled_wifi_qrcode_display_duration: 
		for x in range(oled.height):
			for y in range(oled.height):
				draw.point((x+32,y),fill=int(image_bw.getpixel((x,y))))

		# Display image
		oled.image(image)
		oled.show()
		time.sleep(oled_wifi_qrcode_display_duration)	
		# Clear display.
		oled.fill(0)
		oled.show()
		# Draw a white background
		draw.rectangle((0, 0, oled.width, oled.height), outline=255, fill=255)		

	# Draw a smaller inner rectangle
	draw.rectangle(
			(BORDER, BORDER, oled.width - BORDER - 1, oled.height - BORDER - 1),
			outline=0,
			fill=0,
	)

	padding = -2
	top = padding
	bottom = oled.height-padding
	# Move left to right keeping track of the current x position for drawing shapes.
	x = 2*BORDER

	#cmd= "date +%Y-%m-%dT%H:%M:%S"
	#date = subprocess.check_output(cmd, shell = True )

	# Write lines of text.
	ttop=top
	ttop=ttop+font_size

	draw.text((x, ttop), "INTEL-IRRIS WaziGate", font=font, fill=255)
	ttop=ttop+font_size
	
	if key_device.cyclic_show_all_device:
		get_all_devices_id_list()
		
		if has_found_device:
			if device_index >= len(all_devices_id_list):
				device_index=0
			device_id_for_oled=all_devices_id_list[device_index]	

	if check_for_device(device_id_for_oled):
		print ('oled-service: found device')
	else:
		print ('oled-service: device not found!')
		find_new_default_device()	
			
	if has_found_device:
		if key_device.get_sensor_type_from_local_database:
			print ('oled-service: get sensor type and model from local database')
			get_sensor_type_from_local_database(device_id_for_oled)
		else:
			print ('oled-service: get sensor type and model from key_device.py')
			sensor_type=key_device.sensor_type
			sensor_model=key_device.sensor_model

		print(sensor_model, sensor_type)
			
		print ('oled-service: device id '+device_id_for_oled)	
		#text_to_display=sensor_model+" "+sensor_type
		text_to_display=device_name_for_oled
		draw.text((x, ttop), text_to_display, font=font, fill=255)
		ttop=ttop+font_size

		#we read the last value of the device
		get_last_raw_value(device_id_for_oled)
	
		if last_raw_value_days:
			text_to_display=str(last_raw_value_days)+main_screen_str_1
		else:
			if last_raw_value_minutes > 60:
				text_to_display=str(int(last_raw_value_minutes/60))+"h"+str(last_raw_value_minutes-int(last_raw_value_minutes/60)*60)+main_screen_str_2
			else:				
				text_to_display=str(last_raw_value_minutes)+main_screen_str_3
			
		draw.text((x, ttop), text_to_display, font=font, fill=255)
		ttop=ttop+font_size
		
		if sensor_type=='capacitive':
			text_to_display=main_screen_str_4+str(last_raw_value)
		else:	
			text_to_display=main_screen_str_4cb+str(last_raw_value)
									
		draw.text((x, ttop), text_to_display, font=font, fill=255)
		ttop=ttop+font_size
	
		if sensor_type=='capacitive':
			get_capacitive_soil_condition(device_id_for_oled, last_raw_value)
			text_to_display=main_screen_str_5+str(capacitive_soil_condition)
		else:
			get_tensiometer_soil_condition(device_id_for_oled, last_raw_value)
			text_to_display=main_screen_str_5+str(tensiometer_soil_condition)	

		draw.text((x, ttop), text_to_display, font=font, fill=255)
		ttop=ttop+font_size
	else:
		draw.text((x, ttop), "no device with", font=font, fill=255)
		ttop=ttop+font_size
		draw.text((x, ttop), "SEN0308 or", font=font, fill=255)
		ttop=ttop+font_size
		draw.text((x, ttop), "WM200 sensor", font=font, fill=255)
		ttop=ttop+font_size
		draw.text((x, ttop), "need to create one", font=font, fill=255)
		
	# Display image
	oled.image(image)
	oled.show()
	
	if len(all_devices_id_list)>1:
		#duration of screen saver mode
		oled_display_timer=12
		#duration of the full information screen
		oled_display_duration=5
	else:
		#duration of screen saver mode
		oled_display_timer=30
		#duration of the full information screen
		oled_display_duration=6			
	
	time.sleep(oled_display_duration)
	
	screen_saver(oled_display_timer)
	
	device_index=device_index+1	
