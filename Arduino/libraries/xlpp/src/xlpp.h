#ifndef xlpp_h
#define xlpp_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LPP_DIGITAL_INPUT 0         // 1 byte
#define LPP_DIGITAL_OUTPUT 1        // 1 byte
#define LPP_ANALOG_INPUT 2          // 2 bytes, 0.01 signed
#define LPP_ANALOG_OUTPUT 3         // 2 bytes, 0.01 signed
#define LPP_GENERIC_SENSOR 100      // 4 bytes, unsigned
#define LPP_LUMINOSITY 101          // 2 bytes, 1 lux unsigned
#define LPP_PRESENCE 102            // 1 byte, bool
#define LPP_TEMPERATURE 103         // 2 bytes, 0.1°C signed
#define LPP_RELATIVE_HUMIDITY 104   // 1 byte, 0.5% unsigned
#define LPP_ACCELEROMETER 113       // 2 bytes per axis, 0.001G
#define LPP_BAROMETRIC_PRESSURE 115 // 2 bytes 0.1hPa unsigned
#define LPP_GYROMETER 134     // 2 bytes per axis, 0.01 °/s
#define LPP_GPS 136           // 3 byte lon/lat 0.0001 °, 3 bytes alt 0.01 meter

#define LPP_VOLTAGE 116       // 2 bytes 0.01V unsigned
#define LPP_CURRENT 117       // 2 bytes 0.001A unsigned
#define LPP_FREQUENCY 118     // 4 bytes 1Hz unsigned
#define LPP_PERCENTAGE 120    // 1 byte 1-100% unsigned
#define LPP_ALTITUDE 121      // 2 byte 1m signed
#define LPP_CONCENTRATION 125 // 2 bytes, 1 ppm unsigned
#define LPP_POWER 128         // 2 byte, 1W, unsigned
#define LPP_DISTANCE 130      // 4 byte, 0.001m, unsigned
#define LPP_ENERGY 131        // 4 byte, 0.001kWh, unsigned
#define LPP_DIRECTION 132     // 2 bytes, 1deg, unsigned
#define LPP_UNIXTIME 133      // 4 bytes, unsigned
#define LPP_COLOUR 135        // 1 byte per RGB Color
#define LPP_SWITCH 142        // 1 byte, 0/1

#define XLPP_INTEGER 51    // n-byte (variable, variant integer), signed
#define XLPP_STRING 52     // n-byte (string length + 1), null-terminated C string
#define XLPP_BOOL 53       // 1 byte, bool true/false
#define XLPP_BOOL_TRUE 54  // 0 byte, always bool true
#define XLPP_BOOL_FALSE 55 // 0 byte, always bool false

#define XLPP_OBJECT 123    // n-byte, key-valus pairs
#define XLPP_END_OF_OBJECT 0

#define XLPP_ARRAY 91 // n-byte, list of values
#define XLPP_END_OF_ARRAY 93

#define XLPP_FLAGS 56 // n-byte (1 byte per 8 flags), concatenated bools
#define XLPP_BINARY 57 // n-byte (variable, variant length prefixed)
#define XLPP_NULL 58 // 0 byte, no data

#define XLPP_MAX_CAP 255

struct Accelerometer
{
    float x;
    float y;
    float z;
};

struct Gyrometer
{
    float x;
    float y;
    float z;
};

struct GPS
{
    float latitude;
    float longitude;
    float altitude;
};

struct Colour
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class XLPP
{

public:
    XLPP(uint8_t len);
    ~XLPP();

    void reset(void);
    uint8_t getSize(void);
    uint8_t *getBuffer(void);

    // Original LPPv1 data types
    void addDigitalInput(uint8_t channel, uint8_t value);
    void addDigitalInput(uint8_t value);

    void addDigitalOutput(uint8_t channel, uint8_t value);
    void addDigitalOutput(uint8_t value);

    void addAnalogInput(uint8_t channel, float value);
    void addAnalogInput(float value);

    void addAnalogOutput(uint8_t channel, float value);
    void addAnalogOutput(float value);

    void addLuminosity(uint8_t channel, uint16_t value);
    void addLuminosity(uint16_t value);

    void addPresence(uint8_t channel, uint8_t value);
    void addPresence(uint8_t value);

    void addTemperature(uint8_t channel, float value);
    void addTemperature(float value);

    void addRelativeHumidity(uint8_t channel, float value);
    void addRelativeHumidity(float value);

    void addAccelerometer(uint8_t channel, float x, float y, float z);
    void addAccelerometer(float x, float y, float z);

    void addBarometricPressure(uint8_t channel, float value);
    void addBarometricPressure(float value);

    void addGyrometer(uint8_t channel, float x, float y, float z);
    void addGyrometer(float x, float y, float z);

    void addGPS(uint8_t channel, float latitude, float longitude, float altitude);
    void addGPS(float latitude, float longitude, float altitude);

    // Additional data types
    void addUnixTime(uint8_t channel, uint32_t value);
    void addUnixTime(uint32_t value);

    void addGenericSensor(uint8_t channel, float value);
    void addGenericSensor(float value);

    void addVoltage(uint8_t channel, float value);
    void addVoltage(float value);

    void addCurrent(uint8_t channel, float value);
    void addCurrent(float value);

    void addFrequency(uint8_t channel, uint32_t value);
    void addFrequency(uint32_t value);

    void addPercentage(uint8_t channel, uint8_t value);
    void addPercentage(uint8_t value);

    void addAltitude(uint8_t channel, float value);
    void addAltitude(float value);

    void addPower(uint8_t channel, uint16_t value);
    void addPower(uint16_t value);

    void addDistance(uint8_t channel, float value);
    void addDistance(float value);

    void addEnergy(uint8_t channel, float value);
    void addEnergy(float value);

    void addDirection(uint8_t channel, float value);
    void addDirection(float value);

    void addSwitch(uint8_t channel, uint8_t value);
    void addSwitch(uint8_t value);

    void addConcentration(uint8_t channel, uint16_t value);
    void addConcentration(uint16_t value);

    void addColour(uint8_t channel, uint8_t r, uint8_t g, uint8_t b);
    void addColour(uint8_t r, uint8_t g, uint8_t b);

    //

    void addInteger(uint8_t channel, int64_t i);
    void addInteger(int64_t i);

    void addString(uint8_t channel, const char* s);
    void addString(const char* s);

    void addBool(uint8_t channel, bool b);
    void addBool(bool b);

    void beginObject(uint8_t channel);
    void beginObject();
    void addObjectKey(const char* key);
    void getObjectKey(char* key);
    void endObject();

    void beginArray(uint8_t channel);
    void beginArray();
    void endArray();

    // void addFlags(uint8_t channel, bool flag, ...);
    // void addFlags(bool flag, ...);

    void addBinary(uint8_t channel, const void* data, size_t s);
    void addBinary(const void* data, size_t s);

    void addNull(uint8_t channel);
    void addNull();

    //

    uint8_t getChannel();
    uint8_t getType();

    //
    uint8_t getDigitalInput();
    uint8_t getDigitalOutput();
    float getAnalogInput();
    float getAnalogOutput();
    uint16_t getLuminosity();
    uint8_t getPresence();
    float getTemperature();
    float getRelativeHumidity();
    Accelerometer getAccelerometer();
    float getBarometricPressure();
    Gyrometer getGyrometer();
    GPS getGPS();

    uint32_t getUnixTime();
    float getGenericSensor();
    float getVoltage();
    float getCurrent();
    uint32_t getFrequency();
    uint8_t getPercentage();
    float getAltitude();
    uint16_t getPower();
    float getDistance();
    float getEnergy();
    float getDirection();
    uint8_t getSwitch();
    uint16_t getConcentration();
    Colour getColour();

    int64_t getInteger();
    void getString(char* s);
    size_t getString(char* s, size_t limit);
    bool getBool();
    // void getFlags(bool *flags, int l);
    size_t getBinary(void* data);
    void getNull();

    uint8_t *buf;
    uint8_t len;
    uint8_t offset;
    const uint8_t cap;
};

#endif