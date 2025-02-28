# Import QRCode from pyqrcode
import pyqrcode
import png
from pyqrcode import QRCode

# String which represents the QR code
s = "WIFI:S:WAZIGATE_B827EBD1B236;T:WPA2;P:loragateway;;"

# Generate QR code
url = pyqrcode.create(s)

# Create and save the svg file naming "saved_qr.svg"
#url.svg("saved_qr.svg", scale = 8)

# Create and save the png file naming "saved_qr.png"
url.png('wifi.png', scale = 6)