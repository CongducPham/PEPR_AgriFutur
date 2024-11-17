#include <stdio.h>
#include <string.h>
#include "../../src/xlpp.h"

int printXLPP(XLPP &xlpp);
int printSingleValue(XLPP &xlpp);

int main() {
	int e;
	XLPP xlpp(250);

	//
	xlpp.addDigitalInput(1, 0x12);
	xlpp.addDigitalOutput(2, 0x34);

	//
	xlpp.addAnalogInput(2, -1.23);
	xlpp.addAnalogOutput(3, 45.67);

	// 1000 lux is a normal room illuminance value
	xlpp.addLuminosity(4, 1000); // lux

	// 1 = presence, 0 = no presence
	xlpp.addPresence(5, 1);

	// 
	xlpp.addTemperature(6, 10.2); // °C
	xlpp.addTemperature(7, 67.7); // °C

	// 40.0 ~ 50.0 is a perfect relative humidity for health and comfort
	xlpp.addRelativeHumidity(8, 44.5); // %

	//
	xlpp.addAccelerometer(9, 0.024, 1.202, -0.501); // G
	xlpp.addAccelerometer(10, 0, 0, -1); // G = 9,807 m/s²

	// 1 bar = 1000 hPa ≈ 14.7 psi, normal atmospheric pressure (at sea level)
	xlpp.addBarometricPressure(11, 1000);
	// 35 psi ≈ 2.413 bar ≈ 2413.2 hPa, good tire pressure
	xlpp.addBarometricPressure(12, 2413.2);

	// This xlpp can hold 250 bytes, so we print the buffer now
	// and reset it. 
	e = printXLPP(xlpp);
	if (e != 0) return e;
	xlpp.reset();

	// 
	xlpp.addVoltage(1, 1.23); // V
	xlpp.addVoltage(2, 14.11); // V

	//
	xlpp.addCurrent(3, 2.312); // A

	// human audible spectrum: 20 Hz to 20 kHz
	xlpp.addFrequency(4, 20);
	xlpp.addFrequency(5, 20000);

	//
	xlpp.addPercentage(6, 20); // %

	// Rome, Italiy, 37 m above sea level
	xlpp.addAltitude(7, 37.0); // m
	// Mount Everest
	xlpp.addAltitude(8, 8849.0); // m

	// 250~300ppm CO2 level indoor, good value
	xlpp.addConcentration(9, 300); // ppm
	// very bad Co2 indoor level, workplace exposure limit
	xlpp.addConcentration(10, 5000); // ppm

	//
	xlpp.addPower(11, 113); // W

	//
	xlpp.addDistance(12, 2.246); // m
	xlpp.addDistance(13, 0.030); // m, 1ft

	//
	xlpp.addEnergy(14, 0.060); // a 60W light bulb, one hour
	xlpp.addEnergy(15, 0.006); // a 6W LED, one hour

	// This xlpp can hold 250 bytes, so we print the buffer now
	// and reset it.
	e = printXLPP(xlpp);
	if (e != 0) return e;
	xlpp.reset();

	//
	xlpp.addDirection(1, 135.0); // 135deg

	// 01/05/2021 @ 11:23am (UTC)
	// Time for some coffee.
	xlpp.addUnixTime(2, 1609845780);

	//
	xlpp.addGyrometer(3, 0.12, 1.01, 0.05); // (x,y,z) °/s

	// #0388fc, a nice blue
	xlpp.addColour(4, 3, 136, 252); // (R,G,B)

	//
	xlpp.addGPS(5, 41.8912, 12.4922, 35.00); // Rome, Italy

	// Switch, 1 == on, 0 == off
	xlpp.addSwitch(6, 1);

	//
	xlpp.addInteger(7, 27);
	xlpp.addInteger(8, 1352);
	// xlpp.addInteger(9, 34568234954L);

	// This xlpp can hold 250 bytes, so we print the buffer now
	// and reset it.
	e = printXLPP(xlpp);
	if (e != 0) return e;
	xlpp.reset();

	//
	xlpp.addString(9, "AAAA");

	//
	xlpp.addBool(10, true);
	xlpp.addBool(11, false);

	//
	xlpp.beginObject(12);
	xlpp.addObjectKey("level");
	xlpp.addPercentage(45.2);
	xlpp.addObjectKey("shaky");
	xlpp.addBool(false);
	xlpp.endObject();

	//
	xlpp.beginArray(13);
	xlpp.addDigitalInput(4);
	xlpp.addDigitalInput(25);
	xlpp.addDigitalInput(6);
	xlpp.addDigitalInput(33);
	xlpp.endArray();

	e = printXLPP(xlpp);
	return e;
}


int printSingleValue(XLPP &xlpp)
{
	int e = 0;

	uint8_t type = xlpp.getType();

	switch (type) {
	case LPP_DIGITAL_INPUT:
	{
		uint8_t v = xlpp.getDigitalOutput();
		printf("Digital Input: %hu (0x%02x)\n", v, v);
		break;
	}
	case LPP_DIGITAL_OUTPUT:
	{
		uint8_t v = xlpp.getDigitalInput();
		printf("Digital Output: %hu (0x%02x)\n", v, v);
		break;
	}
	case LPP_ANALOG_INPUT:
	{
		float f = xlpp.getAnalogInput();
		printf("Analog Input: %.2f\n", f);
		break;
	}
	case LPP_ANALOG_OUTPUT:
	{
		float f = xlpp.getAnalogOutput();
		printf("Analog Output: %.2f\n", f);
		break;
	}
	case LPP_LUMINOSITY:
	{
		uint16_t w = xlpp.getLuminosity();
		printf("Luminosity: %hu lux\n", w);
		break;
	}
	case LPP_PRESENCE:
	{
		uint8_t v = xlpp.getPresence();
		printf("Presence: %s\n", v?"yes":"no");
		break;
	}
	case LPP_TEMPERATURE:
	{
		float f = xlpp.getTemperature();
		printf("Temperature: %.1f °C\n", f);
		break;
	}
	case LPP_RELATIVE_HUMIDITY:
	{
		float f = xlpp.getRelativeHumidity();
		printf("Relative Humidity: %.1f %\n", f);
		break;
	}
	case LPP_ACCELEROMETER:
	{
		Accelerometer a = xlpp.getAccelerometer();
		printf("Accelerometer: X %.3f g, Y %.3f g, Z %.3f g\n", a.x, a.y, a.z);
		break;
	}
	case LPP_BAROMETRIC_PRESSURE:
	{
		float f = xlpp.getBarometricPressure();
		printf("Barometric Pressure: %.1f hPa\n", f);
		break;
	}
	case LPP_VOLTAGE:
	{
		float f = xlpp.getVoltage();
		printf("Voltage: %.2f V\n", f);
		break;
	}
	case LPP_CURRENT:
	{
		float f = xlpp.getCurrent();
		printf("Current: %.3f A\n", f);
		break;
	}
	case LPP_FREQUENCY:
	{
		uint32_t u = xlpp.getFrequency();
		printf("Frequency: %d Hz\n", u);
		break;
	}
	case LPP_PERCENTAGE:
	{
		uint32_t u = xlpp.getPercentage();
		printf("Percentage: %d %%\n", u);
		break;
	}
	case LPP_ALTITUDE:
	{
		float f = xlpp.getAltitude();
		printf("Altitude: %.0f m\n", f);
		break;
	}
	case LPP_CONCENTRATION:
	{
		uint16_t w = xlpp.getConcentration();
		printf("Concentration: %d ppm\n", w);
		break;
	}
	case LPP_POWER:
	{
		uint16_t w = xlpp.getPower();
		printf("Power: %d W\n", w);
		break;
	}
	case LPP_DISTANCE:
	{
		float f = xlpp.getDistance();
		printf("Distance: %.3f m\n", f);
		break;
	}
	case LPP_ENERGY:
	{
		float f = xlpp.getEnergy();
		printf("Energy: %.3f kWh\n", f);
		break;
	}
	case LPP_DIRECTION:
	{
		float f = xlpp.getDirection();
		printf("Direction: %.0f deg\n", f);
		break;
	}
	case LPP_UNIXTIME:
	{
		uint32_t u = xlpp.getUnixTime();
		printf("Unix Time: %d\n", u);
		break;
	}
	case LPP_GYROMETER:
	{
		Gyrometer g = xlpp.getGyrometer();
		printf("Gyrometer: X %.2f °/s, Y %.2f °/s, Z %.2f °/s\n", g.x, g.y, g.z);
		break;
	}
	case LPP_COLOUR:
	{
		Colour c = xlpp.getColour();
		printf("Colour: R %d, G %d, B %d (#%02X%02X%02X)\n", c.r, c.g, c.b, c.r, c.g, c.b);
		break;
	}
	case LPP_GPS:
	{
		GPS p = xlpp.getGPS();
		printf("GPS: lat %.4f, lon %.4f, alt %2f\n", p.latitude, p.longitude, p.altitude);
		printf("See http://www.google.com/maps/place/%.4f,%.4f\n", p.latitude, p.longitude);
		break;
	}
	case LPP_SWITCH:
	{
		uint8_t v = xlpp.getSwitch();
		printf("Switch: %s\n", v?"on":"off");
		break;
	}
	case XLPP_INTEGER:
	{
		int64_t i = xlpp.getInteger();
		printf("Integer: %ld\n", i);
		break;
	}
	case XLPP_STRING:
	{
		char str[25];
		xlpp.getString(str);
		if (str[24] == 0)
			printf("String: \"%s\"\n", str);
		else
			printf("String (unterminated): \"%.25s\"\n", str);
		break;
	}
	case XLPP_BOOL:
	{
		bool b = xlpp.getBool();
		printf("Bool: %s\n", b?"true":"false");
		break;
	}
	case XLPP_BOOL_TRUE:
	{
		printf("Bool: true\n");
		break;
	}
	case XLPP_BOOL_FALSE:
	{
		printf("Bool: false\n");
		break;
	}
	case XLPP_OBJECT:
	{
		char key[25];
		printf("Object {\n");
		while (true) {
			xlpp.getObjectKey(key);
			if (strcmp(key, "") == 0)
				break;
			printf("  \"%s\": ", key);
			e = printSingleValue(xlpp);
			if (e != 0) return e;
		}
		printf("}\n");
		break;
	}
	case XLPP_ARRAY:
	{
		printf("Array [\n");
		while (true) {
			e = printSingleValue(xlpp);
			if (e == XLPP_END_OF_ARRAY) break;
			if (e != 0) return e;
		}
		printf("]\n");
		break;
	}
	case XLPP_END_OF_ARRAY:
		return XLPP_END_OF_ARRAY;

	case XLPP_FLAGS:
		// TODO

	case XLPP_BINARY:
	{
		uint8_t data[25];
		size_t s = xlpp.getBinary(data);
		printf("Binary: 0x");
		for (int i = 0; i < s; i++)
		{
			printf("%02X", data[i]);
		}
		printf("\n");
		break;
	}
	case XLPP_NULL:
		printf("Null\n");
		break;

	default:	
		printf("Unknown\n");
		return 256;
	}

	return 0;
}

int printXLPP(XLPP &xlpp)
{
	int e = 0;
	printf("HEX ");
	for (int i = xlpp.offset; i < xlpp.len; i++)
	{
		printf("%02X", xlpp.buf[i]);
	}
	printf("\n");

	while (xlpp.offset < xlpp.len)
	{
		uint8_t chan = xlpp.getChannel();
		printf("Chan %2d: ", chan);

		e = printSingleValue(xlpp);
		if (e != 0) return e;
	}
	return 0;
}