#ifndef CUSTOM_CAM_H
#define CUSTOM_CAM_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include "ConfigSettings.h"
#include "RadioSettings.h"

#ifdef SX126X
  #include <SX126XLT.h>
  #include "SX126X_RadioSettings.h"
#endif

#ifdef SX127X
  #include <SX127XLT.h>
  #include "SX127X_RadioSettings.h"
#endif

#ifdef SX128X
  #include <SX128XLT.h>
  #include "SX128X_RadioSettings.h"
#endif

#ifdef SX126X
extern SX126XLT LT;
#endif

#ifdef SX127X
extern SX127XLT LT;
#endif

#ifdef SX128X
extern SX128XLT LT;
#endif

extern unsigned char DevAddr[4];
extern void blinkLed(uint8_t n, uint16_t t);

#define UCAM_ADDR1 0x2D
#define UCAM_ADDR2 0xAA
//by default
#define CAM_RES128X128
//#define CAM_RES240X240
//#define ALLOCATE_DEDICATED_INIMAGE_BUFFER
#define SHORT_COMPUTATION

//#define USEREFIMAGE
// use the new image encoding scheme, DO NOT CHANGE
#define CRAN_NEW_CODING
#define DISPLAY_PKT
//#define DISPLAY_FILLPKT
//#define DISPLAY_BLOCK
// get and print stats for image encoding
#define DISPLAY_ENCODE_STATS
// get and print stats for each packet sent
#define DISPLAY_PACKETIZATION_SEND_STATS
// get and print stats for eack packet packetized
//#define DISPLAY_PACKETIZATION_STATS
// will periodically take a snapshot and transmit the image
//#define PERIODIC_IMG_TRANSMIT
// will periodically take a new reference image
//#define PERIODIC_REF_UPDATE
// this define will enable a snapshot at startup that will be used as reference image
// in this case, the behavior is fully automatic
//#define GET_PICTURE_ON_SETUP
// take into account light change between snapshot, recommended.
#define LUM_HISTO
//#define TEST_LUMINOSITY
#define DARK_THRESHOLD          20
// will insert the source addr in the image packet
//#define WITH_SRC_ADDR 

// use esp_light_sleep_start() instead of delay()
// use in "production mode" as the serial port is not available in light sleep
// so flashing needs to manually put the ESP32S3 in boot mode
//#define DELAY_WITH_LIGHT_SLEEP

// to get image transmission statistique on a dedicated device
// if LoRaCAM-AI addr is 2DAA, then the image stat device is by default 2EAA
#define TRANSMIT_IMAGE_INDICATION_WAZIGATE

#ifdef TRANSMIT_IMAGE_INDICATION_WAZIGATE
#define USE_XLPP
#include <xlpp.h>
#endif

#ifdef WITH_SRC_ADDR 
#define PREAMBLE_SIZE            5
#else
#define PREAMBLE_SIZE            4
#endif

#define MIN_PKT_SIZE                      32
#define DEFAULT_MSS                       90

#define MIN_INTER_PKT_TIME       500 // in ms
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// if the inter-pkt time is set to MIN_INTER_PKT_TIME, then packets are sent back-to-back, as fast as possible
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_INTER_PKT_TIME   MIN_INTER_PKT_TIME+4500
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MIN_INTER_SNAPSHOT_TIME  5000 // in ms

#ifdef PERIODIC_IMG_TRANSMIT
// 1 image / min
// will be increased to 1 image / hour in main program
#define DEFAULT_INTER_SNAPSHOT_TIME  60 // in s
#else
// here it means that image will be sent only on intrusion detection
#define DEFAULT_INTER_SNAPSHOT_TIME  60 // in s
#endif

#define DEFAULT_QUALITY_FACTOR        20

#ifdef WITH_LORA_MODULE
    #define MAX_PKT_SIZE                  238
    #define DEFAULT_LORA_MSS              40 //MAX_PKT_SIZE
    #define DEFAULT_LORA_QUALITY_FACTOR   DEFAULT_QUALITY_FACTOR
#endif

// the size of data to store one line
// currently, 240x240 is the maximum
#ifdef CAM_RES240X240
#define CAMDATA_LINE_SIZE     240
#else
#define CAMDATA_LINE_SIZE     128
#endif

#define CAMDATA_LINE_NUMBER   1
#define DISPLAY_CAM_DATA_SIZE 12

// CHANGE HERE THE THRESHOLD TO TUNE THE INTRUSION DETECTION PROCESS
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PIX_THRES              55
#define INTRUSION_THRES        500
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// From https://github.com/espressif/esp32-camera/blob/master/conversions/to_bmp.c
//
static const int BMP_HEADER_LEN = 54;

typedef struct {
    uint32_t filesize;
    uint32_t reserved;
    uint32_t fileoffset_to_pixelarray;
    uint32_t dibheadersize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsperpixel;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t ypixelpermeter;
    uint32_t xpixelpermeter;
    uint32_t numcolorspallette;
    uint32_t mostimpcolor;
} bmp_header_t;

///////////////////////////////////////////////////////////////////////////////////

void init_custom_cam();
void set_mss(uint8_t mss);
void set_quality_factor(uint8_t Q);
int encode_image(uint8_t* buf, bool transmit);
#endif
