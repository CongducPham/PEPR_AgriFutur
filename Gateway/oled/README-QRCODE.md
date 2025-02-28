QR with Python
----

	> pip3 install pyqrcode
	> pip3 install pypng
	
then run Python program to generate `wifi.png`	

QR with qrencode
----

	> sudo apt-get install qrencode
	> qrencode -t PNG -o wifi.png "WIFI:S:WAZIGATE_B827EBD1B236;T:WPA2;P:loragateway;;"
	
convert to bitmap (not necessary actually)
----
	> sudo apt-get install imagemagick
	> convert wifi.png -alpha off wifi.bmp
	
		
	