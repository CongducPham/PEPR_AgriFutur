# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

"""
This demo will fill the screen with white, draw a black box on top
and then print Hello World! in the center of the display

This example is for use on (Linux) computers that are using CPython with
Adafruit Blinka to support CircuitPython libraries. CircuitPython does
not support PIL/pillow (python imaging library)!
"""

import sys
import time
import subprocess

import board
import digitalio
from PIL import Image, ImageDraw, ImageFont
import adafruit_ssd1306

# Define the Reset Pin
oled_reset = None

# Change these
# to the right size for your display!
WIDTH = 128
HEIGHT = 64  # Change to 64 if needed
BORDER = 2

# Use for I2C.
i2c = board.I2C()
oled = adafruit_ssd1306.SSD1306_I2C(WIDTH, HEIGHT, i2c, addr=0x3C, reset=oled_reset)

# Use for SPI
# spi = board.SPI()
# oled_cs = digitalio.DigitalInOut(board.D5)
# oled_dc = digitalio.DigitalInOut(board.D6)
# oled = adafruit_ssd1306.SSD1306_SPI(WIDTH, HEIGHT, spi, oled_dc, oled_reset, oled_cs)

# Clear display.
oled.fill(0)
oled.show()

# Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
image = Image.new("1", (oled.width, oled.height))

# Get drawing object to draw on image.
draw = ImageDraw.Draw(image)

# Draw a white background
draw.rectangle((0, 0, oled.width, oled.height), outline=255, fill=255)

# Draw a smaller inner rectangle
draw.rectangle(
    (BORDER, BORDER, oled.width - BORDER - 1, oled.height - BORDER - 1),
    outline=0,
    fill=0,
)

# Load default font.
font = ImageFont.load_default()

# Draw some shapes.
# First define some constants to allow easy resizing of shapes.
padding = -2
top = padding
bottom = oled.height-padding
# Move left to right keeping track of the current x position for drawing shapes.
x = 2*BORDER

# Shell scripts for system monitoring from here : https://unix.stackexchange.com/questions/119126/command-to-display-memory-usage-disk-usage-and-cpu-load
cmd= "date +%Y-%m-%dT%H:%M:%S"
date = subprocess.check_output(cmd, shell = True )

# Write lines of text.
ttop=top

#draw.text((x, ttop), str(date),  font=font, fill=255)
ttop=ttop+8

draw.text((x, ttop), "GenericSensorGateway", font=font, fill=255)
ttop=ttop+8

ttop=ttop+8

ttop=ttop+8

draw.text((x, ttop), "hello", font=font, fill=255)
ttop=ttop+8

draw.text((x, ttop), "from", font=font, fill=255)
ttop=ttop+8

draw.text((x, ttop), "Generic-Sensor-Gateway", font=font, fill=255)
ttop=ttop+8

# Draw Some Text
#text = "Hello World!"
#(font_width, font_height) = font.getsize(text)
#draw.text(
#    (oled.width // 2 - font_width // 2, oled.height // 2 - font_height // 2),
#    text,
#    font=font,
#    fill=255,
#)

# Display image
oled.image(image)
oled.show()
