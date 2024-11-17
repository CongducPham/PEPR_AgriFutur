#include "xlpp.h"
#include <stdio.h>

XLPP::XLPP(uint8_t cap) : cap(len)
{
    buf = (uint8_t *)malloc(cap);
    offset = 0;
    len = 0;
}

XLPP::~XLPP(void)
{
    free(buf);
}

uint8_t XLPP::getSize()
{
    return len;
}

////////////////////

#define int24_t int32_t
#define uint24_t uint32_t

#define _WRITE(v) buf[len++] = (v)&0xff;
#define WRITE_uint8_t(v) _WRITE(v);
#define WRITE_uint16_t(v) \
    _WRITE(v >> 8);       \
    _WRITE(v);
#define WRITE_uint24_t(v) \
    _WRITE(v >> 16);      \
    _WRITE(v >> 8);       \
    _WRITE(v);
#define WRITE_uint32_t(v) \
    _WRITE(v >> 24);      \
    _WRITE(v >> 16);      \
    _WRITE(v >> 8);       \
    _WRITE(v);

#define WRITE_int8_t(v) WRITE_uint8_t(uint8_t(v));
#define WRITE_int16_t(v) WRITE_uint16_t(uint16_t(v));
#define WRITE_int24_t(v) WRITE_uint24_t(uint24_t(v));
#define WRITE_int32_t(v) WRITE_uint32_t(uint32_t(v));

#define _READ buf[offset++]
#define READ_uint8_t _READ
#define READ_uint16_t (uint16_t(_READ) << 8) + uint16_t(_READ)
#define READ_uint24_t (uint24_t(_READ) << 16) + (uint24_t(_READ) << 8) + uint24_t(_READ)
#define READ_uint32_t (uint32_t(_READ) << 24) + (uint32_t(_READ) << 16) + (uint32_t(_READ) << 8) + uint32_t(_READ)

#define READ_int8_t int8_t(READ_uint8_t)
#define READ_int16_t int16_t(READ_uint16_t)
#define READ_int24_t int24_t(READ_uint24_t)
#define READ_int32_t int32_t(READ_uint32_t)

#define XLPP_(NAME, TYPE, VALUE_T, MULTI, WIRE_T)        \
    void XLPP::add##NAME(uint8_t channel, VALUE_T value) \
    {                                                    \
        buf[len++] = channel;                            \
        add##NAME(value);                                \
    }                                                    \
    void XLPP::add##NAME(VALUE_T value)                  \
    {                                                    \
        buf[len++] = TYPE;                               \
        WIRE_T v = value * MULTI;                        \
        WRITE_##WIRE_T(v);                               \
    }                                                    \
    VALUE_T XLPP::get##NAME()                            \
    {                                                    \
        return VALUE_T(READ_##WIRE_T) / MULTI;           \
    }

////////////////////

void XLPP::reset(void)
{
    offset = 0;
    len = 0;
}

uint8_t *XLPP::getBuffer(void)
{
    return buf + offset;
}

//

uint8_t XLPP::getChannel()
{
    return buf[offset++];
}

uint8_t XLPP::getType()
{
    return buf[offset++];
}

////////////////////////////////////////////////////////////////////////////////

XLPP_(DigitalInput, LPP_DIGITAL_INPUT, uint8_t, 1, uint8_t);
XLPP_(DigitalOutput, LPP_DIGITAL_OUTPUT, uint8_t, 1, uint8_t);
XLPP_(AnalogInput, LPP_ANALOG_INPUT, float, 100, int16_t);
XLPP_(AnalogOutput, LPP_ANALOG_OUTPUT, float, 100, int16_t);
XLPP_(Luminosity, LPP_LUMINOSITY, uint16_t, 1, uint16_t);
XLPP_(Presence, LPP_PRESENCE, uint8_t, 1, uint8_t);
XLPP_(Temperature, LPP_TEMPERATURE, float, 10, int16_t);
XLPP_(RelativeHumidity, LPP_RELATIVE_HUMIDITY, float, 2, int8_t);

//

void XLPP::addAccelerometer(uint8_t channel, float x, float y, float z)
{
    buf[len++] = channel;
    addAccelerometer(x, y, z);
}

void XLPP::addAccelerometer(float x, float y, float z)
{
    buf[len++] = LPP_ACCELEROMETER;
    int16_t vx = int16_t(x * 1000);
    WRITE_int16_t(vx);
    int16_t vy = int16_t(y * 1000);
    WRITE_int16_t(vy);
    int16_t vz = int16_t(z * 1000);
    WRITE_int16_t(vz);
}

Accelerometer XLPP::getAccelerometer()
{
    Accelerometer a;
    a.x = float(READ_int16_t) / 1000;
    a.y = float(READ_int16_t) / 1000;
    a.z = float(READ_int16_t) / 1000;
    return a;
}

//

XLPP_(BarometricPressure, LPP_BAROMETRIC_PRESSURE, float, 10, int16_t);

//

void XLPP::addGyrometer(uint8_t channel, float x, float y, float z)
{
    buf[len++] = channel;
    addGyrometer(x, y, z);
}

void XLPP::addGyrometer(float x, float y, float z)
{
    buf[len++] = LPP_GYROMETER;
    int16_t vx = int16_t(x * 100);
    WRITE_int16_t(vx);
    int16_t vy = int16_t(y * 100);
    WRITE_int16_t(vy);
    int16_t vz = int16_t(z * 100);
    WRITE_int16_t(vz);
}

Gyrometer XLPP::getGyrometer()
{
    Gyrometer v;
    v.x = float(READ_int16_t) / 100;
    v.y = float(READ_int16_t) / 100;
    v.z = float(READ_int16_t) / 100;
    return v;
}

//

void XLPP::addGPS(uint8_t channel, float latitude, float longitude, float altitude)
{
    buf[len++] = channel;
    addGPS(latitude, longitude, altitude);
}

void XLPP::addGPS(float latitude, float longitude, float altitude)
{
    buf[len++] = LPP_GPS;
    int32_t lat = int32_t(latitude * 10000);
    WRITE_int24_t(lat);
    int32_t lon = int32_t(longitude * 10000);
    WRITE_int24_t(lon);
    int32_t alt = int32_t(altitude * 100);
    WRITE_int24_t(alt);
}

GPS XLPP::getGPS()
{
    GPS v;
    v.latitude = float(READ_int24_t) / 10000;
    v.longitude = float(READ_int24_t) / 10000;
    v.altitude = float(READ_int24_t) / 100;
    return v;
}

////////////////////

XLPP_(Voltage, LPP_VOLTAGE, float, 100, uint16_t);
XLPP_(Current, LPP_CURRENT, float, 1000, uint16_t);
XLPP_(Frequency, LPP_FREQUENCY, uint32_t, 1, uint32_t);
XLPP_(Percentage, LPP_PERCENTAGE, uint8_t, 1, uint8_t);
XLPP_(Altitude, LPP_ALTITUDE, float, 1, uint16_t);
XLPP_(Concentration, LPP_CONCENTRATION, uint16_t, 1, uint16_t);
XLPP_(Power, LPP_POWER, uint16_t, 1, uint16_t);
XLPP_(Distance, LPP_DISTANCE, float, 1000, uint32_t);
XLPP_(Energy, LPP_ENERGY, float, 1000, uint32_t);
XLPP_(Direction, LPP_DIRECTION, float, 1, uint16_t);
XLPP_(UnixTime, LPP_UNIXTIME, uint32_t, 1, uint32_t);
XLPP_(Switch, LPP_SWITCH, uint8_t, 1, uint8_t);

//

void XLPP::addColour(uint8_t channel, uint8_t r, uint8_t g, uint8_t b)
{
    buf[len++] = channel;
    addColour(r, g, b);
}

void XLPP::addColour(uint8_t r, uint8_t g, uint8_t b)
{
    buf[len++] = LPP_COLOUR;
    WRITE_uint8_t(r);
    WRITE_uint8_t(g);
    WRITE_uint8_t(b);
}

Colour XLPP::getColour()
{
    Colour c;
    c.r = READ_uint8_t;
    c.g = READ_uint8_t;
    c.b = READ_uint8_t;
    return c;
}

////////////////////////////////////////////////////////////////////////////////

void XLPP::addInteger(uint8_t channel, int64_t i)
{
    buf[len++] = channel;
    addInteger(i);
}

void XLPP::addInteger(int64_t i)
{
    buf[len++] = XLPP_INTEGER;
    uint64_t ui = uint64_t(i) << 1;
    if (i < 0)
        ui = ~ui;
    while (ui >= 0x80)
    {
        WRITE_uint8_t(uint8_t(ui) | 0x80);
        ui >>= 7;
    }
    WRITE_uint8_t(uint8_t(ui));
}

int64_t XLPP::getInteger()
{
    uint64_t ui = 0;
    uint8_t s = 0;
    for (int i = 0;; i++)
    {
        if (i == 10)
        {
            return 0; // overflow
        }
        uint8_t b = READ_uint8_t;
        if (b < 0x80)
        {
            if (i == 9 && b > 1)
            {
                return 0; // overflow
            }
            ui |= uint64_t(b) << s;
            break;
        }
        ui |= uint64_t(b & 0x7f) << s;
        s += 7;
    }

    int64_t i = int64_t(ui >> 1);
    if (ui & 1)
    {
        i = ~i;
    }
    return i;
}

//

void XLPP::addString(uint8_t channel, const char *str)
{
    buf[len++] = channel;
    addString(str);
}

void XLPP::addString(const char *str)
{
    buf[len++] = XLPP_STRING;
    strcpy((char *)buf + len, str);
    len += strlen(str) + 1;
}

void XLPP::getString(char *str)
{
    strcpy(str, (const char *)buf + offset);
    offset += strlen(str) + 1;
}

size_t XLPP::getString(char *str, size_t limit)
{
    size_t n = 0;
    for (; buf[offset] != 0 && n < limit; n++)
        str[n] = buf[offset++];
    if (n == limit)
    {
        while (buf[offset++])
        {
            // skip remaining bytes
        }
    }
    else
    {
        for (; n < limit; n++)
            str[n] = 0;
        offset++;
    }
    return n;
}

//

void XLPP::addBool(uint8_t channel, bool b)
{
    buf[len++] = channel;
    addBool(b);
}

void XLPP::addBool(bool b)
{
    buf[len++] = b ? XLPP_BOOL_TRUE : XLPP_BOOL_FALSE;
}

bool XLPP::getBool()
{
    return READ_uint8_t != 0;
}

//

void XLPP::beginObject(uint8_t channel)
{
    buf[len++] = channel;
    beginObject();
}

void XLPP::beginObject()
{
    buf[len++] = XLPP_OBJECT;
}

void XLPP::addObjectKey(const char *key)
{
    strcpy((char *)buf + len, key);
    len += strlen(key) + 1;
}

void XLPP::getObjectKey(char *key)
{
    getString(key);
}

void XLPP::endObject()
{
    buf[len++] = XLPP_END_OF_OBJECT;
}

//

void XLPP::beginArray(uint8_t channel)
{
    buf[len++] = channel;
    beginArray();
}

void XLPP::beginArray()
{
    buf[len++] = XLPP_ARRAY;
}

void XLPP::endArray()
{
    buf[len++] = XLPP_END_OF_ARRAY;
}

//

void XLPP::addBinary(uint8_t channel, const void *data, size_t s)
{
    buf[len++] = channel;
    addBinary(data, s);
}

void XLPP::addBinary(const void *data, size_t s)
{
    buf[len++] = XLPP_BINARY;
    size_t i = s;
    while (s >= 0x80)
    {
        WRITE_uint8_t(uint8_t(s) | 0x80);
        s >>= 7;
    }
    WRITE_uint8_t(uint8_t(s));
    memcpy(buf + len, data, s);
    len += s;
}

size_t XLPP::getBinary(void *data)
{
    uint64_t s = 0;
    uint8_t d = 0;
    for (int i = 0;; i++)
    {
        if (i == 10)
        {
            return 0; // overflow
        }
        uint8_t b = READ_uint8_t;
        if (b < 0x80)
        {
            if (i == 9 && b > 1)
            {
                return 0; // overflow
            }
            s |= uint64_t(b) << s;
            break;
        }
        s |= uint64_t(b & 0x7f) << s;
        s += 7;
    }

    memcpy(data, buf + offset, s);
    offset += s;
    return s;
}

//

void XLPP::addNull(uint8_t channel)
{
    buf[len++] = channel;
    addNull();
}

void XLPP::addNull()
{
    buf[len++] = XLPP_NULL;
}

void XLPP::getNull()
{
    // NOP
}