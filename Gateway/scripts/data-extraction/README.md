README for extract_values.py
This python script can be run **on the WaziGate** or **remotely**.
It extracts a dataset for a selection of sensors in a specified format.
The first argument composes the access url to the Wazigate. `localhost` will only work if the script is run on the Wazigate. 

#############################
# Usage
#############################
One need to first build a selection of sensors, and then to do the extraction

#############################
# Syntaxes
#############################
## 1 - Preparation of json file *devices_to_export.json* listing devices and sensors in a human readable format.
`python extract_values.py localhost devices`

## 2 - Same as **1**, additional display in terminal.
`python extract_values.py wazigate.local devices print`

## 3 - Export to JSON, can be read by *Datasets/plot_scripts/plot_json_generic.py*.
`python extract_values.py 192.168.0.107 devices_to_export.json json`

## 4 - Export to CSV, single time column, can be read by *Datasets/plot_scripts/plot_csv_Bondy.py*.
`python extract_values.py 10.42.0.1 devices_to_export.json csv_Bondy`

## 5 - Export to CSV, WG all_in_one format, can be read by *Datasets/plot_scripts/plot_csv_WG_aio.py*.
`python extract_values.py wazigate.local devices_to_export.json csv`


