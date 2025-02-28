#default configuration, based on PRIMA INTEL-IRRIS project
#https://intel-irris.eu/ and https://github.com/CongducPham/PRIMA-Intel-IrriS
##
#first device on gateway SD card image – capacitive sensor or tensiometer sensor
device_id_for_oled='62286d72f06c4c0001eba943'

get_sensor_type_from_local_database=True
#set to True to use value_index_iiwa computed from IIWA WaziApp when available
#set to False to have this value_index computed by oled_service, mainly for testing when IIWA is not available yet
get_value_index_from_local_database=True
#set to True to write value_index in sensor's meta data (oled-service will not write to value_index_iiwa)
set_value_index_in_local_database=True

use_irrometer_interval_for_tensiometer=True
cyclic_show_all_device=True

sensor_type='capacitive'
sensor_model='SEN0308'

#sensor_type="tensiometer"
#sensor_model="WM200"

#change to 'en' for english
lang='fr'

#for clay soil (argileux) 	capacitive_sensor_dry_max=400
#for sandy soil (sableux) 	capacitive_sensor_dry_max=700
#for silty soil () 					capacitive_sensor_dry_max=500
#for peaty soil (tourbeux) 	capacitive_sensor_dry_max=500
#for chalky soil (calcaire) capacitive_sensor_dry_max=500
#for loamy soil (limoneux) 	capacitive_sensor_dry_max=500
#
##but IIWA will set value_index_iiwa accordingly

#by default we used an intermediate value of 600. In air, it should be above 780.
capacitive_sensor_dry_max=550
capacitive_sensor_wet_max=0
capacitive_sensor_n_interval=6
capacitive_sensor_soil_condition=[]
#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-saturated

if lang=="fr":
	capacitive_sensor_soil_condition.append('très sec')
	capacitive_sensor_soil_condition.append('sec')
	capacitive_sensor_soil_condition.append('sec')
	capacitive_sensor_soil_condition.append('hum')
	capacitive_sensor_soil_condition.append('hum')
	capacitive_sensor_soil_condition.append('saturé')
else:
	capacitive_sensor_soil_condition.append('very dry')
	capacitive_sensor_soil_condition.append('dry')
	capacitive_sensor_soil_condition.append('dry')
	capacitive_sensor_soil_condition.append('wet')
	capacitive_sensor_soil_condition.append('wet')
	capacitive_sensor_soil_condition.append('saturated')	

tensiometer_sensor_dry_max=120
tensiometer_sensor_wet_max=0
tensiometer_sensor_n_interval=6
tensiometer_sensor_soil_condition=[]
#we adopt the following rule: 0:very dry; 1:dry; 2:dry-wet 3-wet-dry; 4-wet; 5-saturated

if lang=="fr":
	tensiometer_sensor_soil_condition.append('très sec')
	tensiometer_sensor_soil_condition.append('sec')
	tensiometer_sensor_soil_condition.append('sec')
	tensiometer_sensor_soil_condition.append('hum')
	tensiometer_sensor_soil_condition.append('hum')
	tensiometer_sensor_soil_condition.append('saturé')
else:
	tensiometer_sensor_soil_condition.append('very dry')
	tensiometer_sensor_soil_condition.append('dry')
	tensiometer_sensor_soil_condition.append('dry')
	tensiometer_sensor_soil_condition.append('wet')
	tensiometer_sensor_soil_condition.append('wet')
	tensiometer_sensor_soil_condition.append('saturated')	