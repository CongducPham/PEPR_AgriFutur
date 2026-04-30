You can try the following example:

	> ./decode_to_bmp 2025-02-28-11-00-41_image_dat.txt 128x128-ESP32S3-test.bmp 

to produce decode-2025-02-28-11-00-41_image_dat.txt-P49-S1699.bmp

On the gateway, `decode_to_bmp` is the only tool that is needed to decode the encoded image file.

If you go into `gw-images` folder and you have received images on the gateway, you can try:

	> cd gw-images  
	> python ../../get_last_image_dat.py localhost 67b6fe8568f3190a22e91c6f ..
  
assuming that your LoRaCAM-AI device's id is `67b6fe8568f3190a22e91c6f`. The default LoRaCAM-AI is `LoRaCAM-AI-DEV-2DAA` on the gateway dashboard.

Enjoy
C. Pham   