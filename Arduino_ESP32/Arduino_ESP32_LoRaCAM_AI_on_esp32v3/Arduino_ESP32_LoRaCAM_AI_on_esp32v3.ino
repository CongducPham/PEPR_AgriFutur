/*
 *  LoRaCAM-AI takes picture, encode, process and send image with LoRa 
 *  
 *  Will integrate AI image recognition for various agriculture applications
 *
 *  Copyright (C) 2025 Congduc Pham
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 *
 *  Version:                1.0
 *  Based on ESP32 CameraWebServer and Arduino_LoRa_ucamII (https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html)
 *  Design:                 C. Pham
 *  Implementation:         C. Pham
 *  Last update:            Fev. 14th, 2025
 *
 *  With #define WAIT_FOR_SERIAL_INPUT, for testing purposes, no deep sleep, a command starts with /@ and ends with #: /@Z40#Q40#
 *    "Z64#" -> sets the MSS size to 64, default is 90 for LoRa
 *    "Q15#" -> sets the quality factor to 15, default is 20 for LoRa
 *    "F30000#" -> sets inter-capture time to 30000ms, i.e. 30s, fps is then 1/30, default is 60000ms, i.e. 60s
 *
 *  Otherwise (should be considered as default setting)
 *    - periodic capture & LoRa transmission every hour
 *    - deep sleep between capture, similar to full reset (deep sleep of camera not fully validated yet)
 *    - to flash a new code, connect board and upload before device goes in deep sleep.
 *      a 30s window is set for such purpose before the first deep sleep period
 *    - for real low power mode, we use an external Arduino Pro Mini with a MOSFET to power cycle the LoRaCAM-AI
 *      uncomment RUN_AS_SLAVE in ConfigSettings.h. Then define and connect ACTIVITY_PIN accordingly
 */

#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include <WiFi.h>

#include "ConfigSettings.h"

////////////////////////////////////////////////////////////////////
// CameraWebServer example

// ===================
// Select camera model
// ===================
// --> CAMERA_MODEL_WROVER_KIT: Freenove ESP32 WROVER DEV Board v1.6 & v3.0
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE  // Has PSRAM
// --> CAMERA_MODEL_ESP32S3_EYE: Freenove ESP32S3 WROOM Board
//#define CAMERA_MODEL_ESP32S3_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_CAMS3_UNIT  // Has PSRAM
// --> CAMERA_MODEL_AI_THINKER: ESP32-CAM board
//#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM
// --> CAMERA_MODEL_XIAO_ESP32S3: XIAO ESP32S3 Sense
#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
// ** Espressif Internal Boards **
//#define CAMERA_MODEL_ESP32_CAM_BOARD
//#define CAMERA_MODEL_ESP32S2_CAM_BOARD
//#define CAMERA_MODEL_ESP32S3_CAM_LCD
//#define CAMERA_MODEL_DFRobot_FireBeetle2_ESP32S3 // Has PSRAM
//#define CAMERA_MODEL_DFRobot_Romeo_ESP32S3 // Has PSRAM
#include "camera_pins.h"

// ===========================
// Enter your WiFi credentials
// ===========================
const char *ssid = "freebox_TYWCSM";   
const char *password = "copernic";

//const char *ssid = "iPhoneD";   
//const char *password = "345hello";

//app_httpd.cpp
void startCameraServer();
void setupLedFlash(int pin);

////////////////////////////////////////////////////////////////////
// LoRa + custom cam with optimized image encoding

////////////////////////////////////////////////////////////////////
#define BOOT_START_MSG  "\nNewGen LoRaCAM-AI Sensor – Feb. 14th, 2025. C. Pham, UPPA, France\n"

#ifdef WITH_CUSTOM_CAM
#include "custom_cam.h"
#endif

#ifdef WITH_LORA_MODULE
#include "RadioSettings.h"

////////////////////////////////////////////////////////////////////
// Frequency band - do not change in SX12XX_RadioSettings.h anymore
// if using a native LoRaWAN module such as RAK3172, also select band in RadioSettings.h
#define EU868
// #define AU915
// #define EU433
// #define AS923_2

#ifdef WITH_SPI_COMMANDS
  #include <SPI.h>
  // this is the standard behaviour of library, use SPI Transaction switching
  #define USE_SPI_TRANSACTION
#endif

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

///////////////////////////////////////////////////////////////////
// CHANGE HERE THE NODE ADDRESS IN NO LORAWAN MODE
uint8_t node_addr=8;
//////////////////////////////////////////////////////////////////

/********************************************************************
  _        ___    __      ___   _  _ 
 | |   ___| _ \__ \ \    / /_\ | \| |
 | |__/ _ \   / _` \ \/\/ / _ \| .` |
 |____\___/_|_\__,_|\_/\_/_/ \_\_|\_|
********************************************************************/
                                     
#ifdef LORAWAN
///////////////////////////////////////////////////////////////////
// ENTER HERE your Device Address from the TTN device info (same order, i.e. msb). Example for 0x12345678
// unsigned char DevAddr[4] = {0x26, 0x01, 0x2D, 0xAA};
///////////////////////////////////////////////////////////////////
// The default address in ABP mode for image sensor devices is 26012DAA
// if you need a different address for another image sensor device, use AB, AC,..., AF instead
unsigned char DevAddr[4] = {0x26, 0x01, 0x2D, 0xAA};
#else
///////////////////////////////////////////////////////////////////
// DO NOT CHANGE HERE
unsigned char DevAddr[4] = { 0x00, 0x00, 0x00, node_addr };
///////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////

// create a library class instance called LT
// to handle LoRa radio communications
////////////////////////////////////////////
#ifdef SX126X
SX126XLT LT;
#endif

#ifdef SX127X
SX127XLT LT;
#endif

#ifdef SX128X
SX128XLT LT;
#endif

#endif //WITH_LORA_MODULE

/*****************************
 _____           _      
/  __ \         | |     
| /  \/ ___   __| | ___ 
| |    / _ \ / _` |/ _ \
| \__/\ (_) | (_| |  __/
 \____/\___/ \__,_|\___|
*****************************/ 

///////////////////////////////////////////////////////////////////
#if defined LOW_POWER && defined LOW_POWER_DEEP_SLEEP

#define mS_TO_uS_FACTOR 1000ULL /* Conversion factor for milli seconds to micro seconds */
RTC_DATA_ATTR int bootCount = 0;

void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Wakeup caused by external signal using RTC_IO");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            Serial.println("Wakeup caused by external signal using RTC_CNTL");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            Serial.println("Wakeup caused by touchpad");
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            Serial.println("Wakeup caused by ULP program");
            break;
        default:
            Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
            break;
    }
}

void lowPower(unsigned long time_ms) {
    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    // Print the wakeup reason for ESP32
    print_wakeup_reason();
    esp_sleep_enable_timer_wakeup(time_ms * mS_TO_uS_FACTOR);  // set wake up source
    //Serial.println("Setup ESP32 to sleep for every " + String(idlePeriodInMin) + " minutes and " +
    //               String(idlePeriodInSec) + " Seconds");
    Serial.println("Going to deep sleep now");
    Serial.flush();
    esp_deep_sleep_start();
}
#endif // LOW_POWER

///////////////////////////////////////////////////////////////////
#ifdef WAIT_FOR_SERIAL_INPUT

// BEWARE, we limit the size of the received cmd to avoid strange packets arriving. 
// So avoid to have to long cmd, use multiple cmd is necessary
#define MAX_CMD_LENGTH           60

uint8_t* rcv_data = NULL;
uint8_t rcv_data_len=0;

long getCmdValue(int &i, char* strBuff=NULL) {
      
      // a maximum of 16 characters plus the null-terminited character, can store a 64-bit MAC address
      char seqStr[17]="****************";

      int j=0;
      // character '#' will indicate end of cmd value
      while ((char)rcv_data[i]!='#' && (i < rcv_data_len) && j<strlen(seqStr)) {
              seqStr[j]=(char)rcv_data[i];
              i++;
              j++;
      }
      
      // put the null character at the end
      seqStr[j]='\0';
      
      if (strBuff) {
              strcpy(strBuff, seqStr);        
      }
      else
              return (atol(seqStr));
}   
#endif //WAIT_FOR_SERIAL_INPUT

void blinkLed(uint8_t n, uint16_t t) {
    for (int i = 0; i < n; i++) {
#if defined ARDUINO_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3_SENSE
        //XIAO ESP32S3 has inverted logic for the built-in led
        digitalWrite(LED_BUILTIN, LOW);  
        delay(t);                      
        digitalWrite(LED_BUILTIN, HIGH);
        delay(t);      
#else
        digitalWrite(LED_BUILTIN, HIGH);  
        delay(t);;                      
        digitalWrite(LED_BUILTIN, LOW);
        delay(t);
#endif   
    }
}

/*****************************
 _____      _               
/  ___|    | |              
\ `--.  ___| |_ _   _ _ __  
 `--. \/ _ \ __| | | | '_ \ 
/\__/ /  __/ |_| |_| | |_) |
\____/ \___|\__|\__,_| .__/ 
                     | |    
                     |_|    
******************************/

void setup() {  
  Serial.begin(115200);
  //while (!Serial);  // Ensure that the serial port is enabled
  // Serial.setDebugOutput(true);
#ifdef TEST_IN_PROGRESS  
  delay(2000);
#endif  
  Serial.println();

#ifdef RUN_AS_SLAVE
  Serial.print("Run as slave, set activity pins to HIGH\n");
#ifdef ACTIVITY_PIN1   
  pinMode(ACTIVITY_PIN1, OUTPUT);
  digitalWrite(ACTIVITY_PIN1, HIGH);
#endif  
#ifdef ACTIVITY_PIN2   
  pinMode(ACTIVITY_PIN2, OUTPUT);
  digitalWrite(ACTIVITY_PIN2, HIGH);
#endif 
#ifdef ACTIVITY_PIN3   
  pinMode(ACTIVITY_PIN3, OUTPUT);
  digitalWrite(ACTIVITY_PIN3, HIGH);
#endif 
#endif    

#ifdef ARDUINO_ARCH_ESP32
    Serial.print("ESP32 detected\n");
#endif
#if defined ARDUINO_ESP32S3_DEV || defined MY_FREENOVE_ESP32S3_CAM
    Serial.print("FREENOVE_ESP32S3_CAM variant (test)\n");
#endif
#if defined MY_FREENOVE_ESP32_CAM_DEV || defined MY_NONAME_ESP32_CAM_DEV
    Serial.print("ESP32_CAM_DEV variant\n");
#endif
#if defined ARDUINO_XIAO_ESP32S3
    Serial.print("XIAO-ESP32S3\n");
#endif
#if defined MY_XIAO_ESP32S3_SENSE
    Serial.print("XIAO-ESP32S3-Sense variant with camera board\n");
#endif

#ifdef WITH_LORA_MODULE
#if defined ARDUINO_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3_SENSE
    pinMode(2, OUTPUT);  // GPIO 2 = D1
    pinMode(1, OUTPUT);  // GPIO 1 = D0
    pinMode(4, INPUT);   // GPIO 4 = D3
    pinMode(5, OUTPUT);  // GPIO 5 = D4
    delay(10);
    Serial.print("Setting SPI pins for XIAO-ESP32S3-Sense\n");
    //        SCK, MISO, MOSI, CS/SS
    SPI.begin(1, 4, 5, 2);
#elif defined MY_FREENOVE_ESP32S3_CAM
    pinMode(21, OUTPUT);  // GPIO 21 = 21
    pinMode(47, OUTPUT);  // GPIO 47 = 47
    pinMode(42, INPUT);   // GPIO 42 = 42
    pinMode(41, OUTPUT);  // GPIO 41 = 41
    delay(10);
    Serial.print("Setting SPI pins for FREENOVE-ESP32S3-CAM\n");
    //        SCK, MISO,  MOSI, CS/SS
    SPI.begin(47, 42, 41, 21);
#elif MY_FREENOVE_ESP32_CAM_DEV || defined MY_NONAME_ESP32_CAM_DEV
    pinMode(32, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(13, INPUT);
    pinMode(14, OUTPUT);
    delay(10);
    //        SCK, MISO,  MOSI, CS/SS
    SPI.begin(12, 13, 14, 32);
#else
    // start SPI bus communication
    SPI.begin();
#endif

    // setup hardware pins used by device, then check if device is found
#ifdef SX126X
    if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, SW, LORA_DEVICE))
#endif

#ifdef SX127X
    if (LT.begin(NSS, NRESET, DIO0, DIO1, DIO2, LORA_DEVICE))
#endif

#ifdef SX128X
    if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, LORA_DEVICE))
#endif
        {
            Serial.print("LoRa Device found\n");
            delay(1000);
        } else {
            Serial.print("No device responding\n");
            while (1) {
            }
        }

    /*******************************************************************************************************
      Based from SX12XX example - Stuart Robinson
    *******************************************************************************************************/

    // The function call list below shows the complete setup for the LoRa device using the
    //  information defined in the Settings.h file.
    // The 'Setup LoRa device' list below can be replaced with a single function call;
    // LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

    //***************************************************************************************************
    // Setup LoRa device
    //***************************************************************************************************
    // got to standby mode to configure device
    LT.setMode(MODE_STDBY_RC);
#ifdef SX126X
    LT.setRegulatorMode(USE_DCDC);
    LT.setPaConfig(0x04, PAAUTO, LORA_DEVICE);
    LT.setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
    LT.calibrateDevice(ALLDevices);  // is required after setting TCXO
    LT.calibrateImage(DEFAULT_CHANNEL);
    LT.setDIO2AsRfSwitchCtrl();
#endif
#ifdef SX128X
    LT.setRegulatorMode(USE_LDO);
#endif
    // set for LoRa transmissions
    LT.setPacketType(PACKET_TYPE_LORA);
    // set the operating frequency
#ifdef MY_FREQUENCY
    LT.setRfFrequency(MY_FREQUENCY, Offset);
#else
    LT.setRfFrequency(DEFAULT_CHANNEL, Offset);
#endif
// run calibration after setting frequency
#ifdef SX127X
    LT.calibrateImage(0);
#endif
    // set LoRa modem parameters
#if defined SX126X || defined SX127X
    LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
    LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif
    // where in the SX buffer packets start, TX and RX
    LT.setBufferBaseAddress(0x00, 0x00);
    // set packet parameters
#if defined SX126X || defined SX127X
    LT.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
#endif
#ifdef SX128X
    LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
#endif
    // syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34
#if defined SX126X || defined SX127X
#if defined PUBLIC_SYNCWORD || defined LORAWAN
    LT.setSyncWord(LORA_MAC_PUBLIC_SYNCWORD);
#else
    LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
#endif
    // set for highest sensitivity at expense of slightly higher LNA current
    LT.setHighSensitivity();  // set for maximum gain
#endif
#ifdef SX126X
    // set for IRQ on TX done and timeout on DIO1
    LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
    // set for IRQ on RX done
    LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);
    LT.setPA_BOOST(PA_BOOST);
#endif
#ifdef SX128X
    LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif

    if (IQ_Setting == LORA_IQ_INVERTED) {
        LT.invertIQ(true);
        Serial.print("Invert I/Q on RX\n");
    } else {
        LT.invertIQ(false);
        Serial.print("Normal I/Q\n");
    }

    //***************************************************************************************************

    Serial.println();
    // reads and prints the configured LoRa settings, useful check
    LT.printModemSettings();
    Serial.println();
    // reads and prints the configured operating settings, useful check
    LT.printOperatingSettings();
    Serial.println();
    Serial.println();
#if defined SX126X || defined SX127X
    // print contents of device registers, normally 0x00 to 0x4F
    LT.printRegisters(0x00, 0x4F);
#endif
#ifdef SX128X
    // print contents of device registers, normally 0x900 to 0x9FF
    LT.printRegisters(0x900, 0x9FF);
#endif

    /*******************************************************************************************************
      End from SX12XX example - Stuart Robinson
    *******************************************************************************************************/

#ifdef SX126X
    Serial.print("SX126X");
#endif
#ifdef SX127X
    Serial.print("SX127X");
#endif
#ifdef SX128X
    Serial.print("SX128X");
#endif

    // Print a success message
    Serial.print(" successfully configured\n");
#endif  //WITH_LORA_MODULE

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
#ifdef ORIGINAL_CONFIG
    config.xclk_freq_hz = 20000000;
#else
    // C.PHAM – Jan. 20th, 2025
    config.xclk_freq_hz = 10000000;
#endif
    config.frame_size = FRAMESIZE_UXGA;
#ifdef ORIGINAL_CONFIG
    config.pixel_format = PIXFORMAT_JPEG;  // for streaming
#else
    // C.PHAM – Jan. 20th, 2025
    config.pixel_format = PIXFORMAT_GRAYSCALE;  // for getting BMP
#endif
    // config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
#ifdef ORIGINAL_CONFIG
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
#else
    // C.PHAM – Jan. 20th, 2025
    config.grab_mode = CAMERA_GRAB_LATEST;
#endif
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if (config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
            config.jpeg_quality = 10;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        // Best option for face detection/recognition
#if defined ORIGINAL_CONFIG || defined CAM_RES240X240
        config.frame_size = FRAMESIZE_240X240;
#else
        config.frame_size = FRAMESIZE_128X128;
#endif

#if CONFIG_IDF_TARGET_ESP32S3
        config.fb_count = 2;
#endif
    }

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    
    /*
    // TEST: set back to enable the camera after the standby mode
    // not sure that it works
    if (s->id.PID == OV2640_PID) {
        Serial.println("OV2640 - use register normal mode");
        s->set_reg(s, 0x109, 0x10, 0x00);   //camera to ready
        delay(1000);
    }
    if (s->id.PID == OV3660_PID || s->id.PID == OV5640_PID) {
        Serial.println("OV3660|OV5640 - use register normal mode");
        s->set_reg(s, 0x3008, 0x40, 0x00);   //camera to ready
        delay(1000);
    }      
    */

    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);        // flip it back
        s->set_brightness(s, 1);   // up the brightness just a bit
        s->set_saturation(s, -2);  // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    if (config.pixel_format == PIXFORMAT_JPEG) {
        s->set_framesize(s, FRAMESIZE_QVGA);
    }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
    s->set_vflip(s, 1);
#endif

// Setup LED FLash if LED pin is defined in camera_pins.h
#if defined(LED_GPIO_NUM)
    setupLedFlash(LED_GPIO_NUM);
#endif

    // Print a start message
    Serial.println(BOOT_START_MSG);

#if defined(WITH_WEB_SERVER)
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    Serial.print("WiFi connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    startCameraServer();

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
#endif

#if defined(WITH_CUSTOM_CAM)
    init_custom_cam();
#endif

#ifdef WITH_AES
    local_lorawan_init();
#endif

    pinMode(LED_BUILTIN, OUTPUT);
}

/*****************************
  _                      
 | |    ___   ___  _ __  
 | |   / _ \ / _ \| '_ \ 
 | |__| (_) | (_) | |_) |
 |_____\___/ \___/| .__/ 
                  |_|    
*****************************/

void loop() {

#ifdef WITH_CUSTOM_CAM

  unsigned long interCamCaptureTime;
  unsigned long nextCamCaptureTime;

#ifdef RUN_AS_SLAVE
  Serial.print("Run as slave, re-set activity pins to HIGH\n");
#ifdef ACTIVITY_PIN1   
  digitalWrite(ACTIVITY_PIN1, HIGH);
#endif  
#ifdef ACTIVITY_PIN2   
  digitalWrite(ACTIVITY_PIN2, HIGH);
#endif 
#ifdef ACTIVITY_PIN3   
  digitalWrite(ACTIVITY_PIN3, HIGH);
#endif 
#endif  

#ifdef WAIT_FOR_SERIAL_INPUT
  //60s
  interCamCaptureTime=DEFAULT_INTER_SNAPSHOT_TIME*1000;
  nextCamCaptureTime=millis()+interCamCaptureTime;

  do {        
      boolean receivedCmd=false;
      uint8_t scmd[80];

      if (Serial.available()) {
          int sb=0;  
      
          while (Serial.available() && sb<80) {
            scmd[sb]=Serial.read();
            sb++;
            delay(50);
          }
          
          scmd[sb]='\0';
            
          Serial.print("Received from serial: ");

          rcv_data = scmd;
          rcv_data_len = sb;    
          receivedCmd=true;     
      }

      if (receivedCmd) {
          for (int f=0; f < rcv_data_len; f++) {
              Serial.print((char)rcv_data[f]);
            }
          
          int i=0;                        
          int cmdValue;
          long longCmdValue;

          // we use prefix '/@' to indicate a command string
          if ((char)rcv_data[i]=='/' && (char)rcv_data[i+1]=='@') {
          
              Serial.println(F("\nParsing command string"));
              
              i=2;
      
              // parse the received command
              // typical cmd is "C1#Q40#S1#" for instance
              while (i < rcv_data_len-1 && rcv_data_len<MAX_CMD_LENGTH) {
                                            
                  switch ((char)rcv_data[i]) {
                      // set the pkt size for binary mode, default is 100
                      // "Z50#"
                      case 'Z':
                      
                              i++;
                              cmdValue=getCmdValue(i);
                              // cannot set pkt size greater than MAX_PKT_SIZE
                              if (cmdValue > MAX_PKT_SIZE)
                                      cmdValue = MAX_PKT_SIZE;
                              // cannot set pkt size smaller than MAX_PKT_SIZE
                              if (cmdValue < MIN_PKT_SIZE)
                                      cmdValue = MIN_PKT_SIZE;
                              // set new pkt size        
                              set_mss(cmdValue); 
                              
                              Serial.print(F("Set MSS to "));
                              Serial.println(cmdValue);
                      break;      
                      
                      // set the quality factor, default is 50
                      // "Q40#"
                      case 'Q':
                      
                              i++;
                              cmdValue=getCmdValue(i);
                              // QualityFactor is 100 maximum
                              if (cmdValue > 100)
                                      cmdValue = 100;
                              // set minimum value to 5
                              if (cmdValue < 5)
                                      cmdValue = 5;
                              // set new QualityFactor        
                              set_quality_factor(cmdValue); 
                              
                              Serial.print(F("Set Q to "));
                              Serial.println(cmdValue);
                              
                              break;  

                      // set inter-frame time, in ms, default is 30s                                                                                                                                      
                      case 'F':

                              i++;
                              longCmdValue=getCmdValue(i);
                              // cannot set an inter snapshot time smaller than MIN_INTER_SNAPSHOT_TIME
                              if (longCmdValue < MIN_INTER_SNAPSHOT_TIME)
                                      longCmdValue = MIN_INTER_SNAPSHOT_TIME;
                              // set new inter snapshot time        
                              interCamCaptureTime=longCmdValue; 
                              
                              Serial.print(F("Set inter-snapshot time to "));
                              Serial.println(interCamCaptureTime);
                                      
                              break;
                  }
                  // go to next command if any.
                  i++;       
              }
          }
          else {
              Serial.println(F("\nHave not recognized a command prefix /@"));
          }                      
      }
      //avoid wrapping
      delay(1000);
      interCamCaptureTime -= 1000;    
  } while (interCamCaptureTime>0);
#else // WAIT_FOR_SERIAL_INPUT

#ifdef TEST_IN_PROGRESS
  //60s * TEST_IN_PROGRESS
  interCamCaptureTime=DEFAULT_INTER_SNAPSHOT_TIME*TEST_IN_PROGRESS*1000;
#else
  //increase the default 60s to 60mins
  interCamCaptureTime=DEFAULT_INTER_SNAPSHOT_TIME*60*1000;  
#endif   

  nextCamCaptureTime=millis()+interCamCaptureTime;
#endif // WAIT_FOR_SERIAL_INPUT 

  camera_fb_t *fb = NULL;
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  uint64_t fr_start = esp_timer_get_time();
#endif
  fb = esp_camera_fb_get();
  if (!fb) {
      log_e("Camera capture failed");
  }

  char ts[32];
  snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);

  uint8_t *buf = NULL;
  size_t buf_len = 0;
  bool converted = frame2bmp(fb, &buf, &buf_len);

  esp_camera_fb_return(fb);

  if (!converted) {
      log_e("BMP Conversion failed");
  }

#if defined LOW_POWER && defined LOW_POWER_DEEP_SLEEP
  Serial.print("Set camera in deep sleep power saving mode\n");

  // esp_camera_deinit() NOT WORKING on the XIAO ESP32S3 because the power down pin is not connected
  // see https://forum.seeedstudio.com/t/xiao-esp32s3-sense-camera-sleep-current/271258/40
  if (PWDN_GPIO_NUM == -1) {
      Serial.println("PWDN_GPIO_NUM - esp_camera_deinit() not working - too bad");
      sensor_t *s = esp_camera_sensor_get();
      // put the camera in standby mode
      if (s->id.PID == OV2640_PID) {         
          // https://github.com/espressif/esp32-camera/issues/672
          Serial.println("OV2640 - use register standby");
          Serial.println("sorry nothing works for now :-(");
          //s->set_reg(s, 0x109, 0x10, 0x10);
          // just to keep this information in case we need it
          // s->set_reg(s, 0x109, 0x10, 0x00);   //camera to ready
          //delay(1000);
      }
      if (s->id.PID == OV3660_PID || s->id.PID == OV5640_PID) {
          Serial.println("OV3660|OV5640 - use register standby");
          Serial.println("but not yet validated as working");
          // from https://forum.seeedstudio.com/t/xiao-esp32s3-sense-camera-sleep-current/271258/40
          // but seems not to work as the camera is still heating :-(
          //s->set_reg(s, 0x3008, 0xFF, 0x42);   //camera to standby
          // just to keep this information in case we need it
          // s->set_reg(s, 0x3008, 0xFF, 0x02);   //camera to ready
          //delay(1000);
      }  
  }
  else {
      // close camera
      esp_err_t err = esp_camera_deinit();
      if (err != ESP_OK)
      Serial.printf("Camera deinit failed with error 0x%x", err);
  }    
#endif

  blinkLed(2, 400);
  ///////////////////////////////////////////////////////////////////
  //here we pass buf as image buffer to our encoding procedure
  //second parameter true/false=enable/disable transmission of packets
  bool with_transmission = true;
  encode_image(buf, with_transmission);
  //
  ///////////////////////////////////////////////////////////////////

  free(buf);

  blinkLed(2, 400);

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  uint64_t fr_end = esp_timer_get_time();
#endif
  log_i("BMP: %llums, %uB", (uint64_t)((fr_end - fr_start) / 1000), buf_len);

  ///////////////////////////////////////////////////////////////////
  // LOW-POWER BLOCK - DO NOT EDIT
  ///////////////////////////////////////////////////////////////////

#ifdef RUN_AS_SLAVE
  Serial.print("Run as slave, set activity pin to LOW\n");
#ifdef ACTIVITY_PIN1   
  digitalWrite(ACTIVITY_PIN1, LOW);
#endif  
#ifdef ACTIVITY_PIN2   
  digitalWrite(ACTIVITY_PIN2, LOW);
#endif 
#ifdef ACTIVITY_PIN3   
  digitalWrite(ACTIVITY_PIN3, LOW);
#endif 
#endif

  unsigned long now_millis=millis();

  Serial.println(now_millis);
  Serial.println(nextCamCaptureTime);

  // Even if we run as slave, we keep the ESP32 deep sleep just in case
  //
#if defined LOW_POWER && defined LOW_POWER_DEEP_SLEEP
  Serial.print("Preparing for deep sleep power saving mode\n");

#ifdef WITH_LORA_MODULE
  //CONFIGURATION_RETENTION=RETAIN_DATA_RAM on SX128X
  //parameter is ignored on SX127X
  LT.setSleep(CONFIGURATION_RETENTION);
  Serial.println("Set LoRa module in deep sleep");
#endif
  
  unsigned long waiting_time;

  //how much do we still have to wait, in millisec?
  if (now_millis > nextCamCaptureTime) {
      // In case of wrapping
      Serial.print("Something wrong with sleep time, back to default\n");
#ifdef TEST_IN_PROGRESS
      //60s * TEST_IN_PROGRESS
      interCamCaptureTime=DEFAULT_INTER_SNAPSHOT_TIME*TEST_IN_PROGRESS*1000;
#else
      //60mins
      interCamCaptureTime=DEFAULT_INTER_SNAPSHOT_TIME*60*1000;  
#endif    
      nextCamCaptureTime=now_millis+interCamCaptureTime;
  }
 
  waiting_time = nextCamCaptureTime-now_millis;

  // When testing in deep sleep mode, we wait 30s to allow flashing a new code if necessary
  // otherwise, deep sleep disconnects the serial port and Arduino IDE cannot flash anymore
  if (bootCount == 0) {
      Serial.println("First start, delay of 30s for uploading program if necessary");
#ifdef TEST_ENERGY        
      blinkLed(1, 800);
#endif             
      delay(30000);
#ifdef TEST_ENERGY        
      blinkLed(1, 800);
#endif   
      waiting_time = nextCamCaptureTime-millis();
  }

  Serial.printf("Go to deep sleep for %ld ms\n", waiting_time);
  lowPower(waiting_time);
  
  // In deep sleep mode, this code will never happen
  Serial.print("Wake from power saving mode\n");
  LT.wake();   
#else // LOW_POWER_DEEP_SLEEP
  Serial.print("Use delay() as power saving mode\n");
  waiting_time = nextCamCaptureTime-millis();
  Serial.printf("Wait for %ld ms\n", waiting_time);
  delay(waiting_time);
#endif // LOW_POWER_DEEP_SLEEP

#else // WITH_CUSTOM_CAM
  // The original CameraWebServer example to test the camera
  // Do nothing. Everything is done in another task by the web server
  delay(10000);
#endif // WITH_CUSTOM_CAM
}
