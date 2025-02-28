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
	
# Create blank image for drawing.
# Make sure to create image with mode '1' for 1-bit color.
image = Image.new("1", (oled.width, oled.height))

# Get drawing object to draw on image.
draw = ImageDraw.Draw(image)

# Clear display.
oled.fill(0)
oled.show()

# Draw a white background
draw.rectangle((0, 0, oled.width, oled.height), outline=255, fill=255)

image2 = Image.open("wifi.png")
image_r = image2.resize((oled.height,oled.height), Image.BICUBIC)
image_bw = image_r.convert("1")

for x in range(oled.height):
	for y in range(oled.height):
		draw.point((x+32,y),fill=int(image_bw.getpixel((x,y))))

# Display image
oled.image(image)
oled.show()	