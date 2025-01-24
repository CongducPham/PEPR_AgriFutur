#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include <WiFi.h>

//#define ORIGINAL_CONFIG

#ifdef ORIGINAL_CONFIG
    #define WITH_WEB_SERVER
#else
    #define WITH_WEB_SERVER
    #define WITH_CUSTOM_CAM
    #define WITH_LORA_MODULE
#endif

// if the XIAO_ESP32S3_SENSE is not automatically detected
#define MY_XIAO_ESP32S3_SENSE
// if the FREENOVE_ESP32S3_CAM is not automatically detected
//#define MY_FREENOVE_ESP32S3_CAM
// if the FREENOVE_ESP32_CAM_DEV is not automatically detected, board v1.6
//#define MY_FREENOVE_ESP32_CAM_DEV
// if the NONAME_ESP32_CAM_DEV is not automatically detected, similar to FREENOVE_ESP32_CAM_DEV
//#define MY_NONAME_ESP32_CAM_DEV

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
#define BOOT_START_MSG  "\nNewGen Camera Sensor – Jan. 22nd, 2025\n"

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
// ENCRYPTION CONFIGURATION AND KEYS FOR LORAWAN
#if defined LORAWAN && defined CUSTOM_LORAWAN
  #ifndef WITH_AES
    #define WITH_AES
  #endif
#endif
#ifdef WITH_AES
  #include "local_lorawan.h"
#endif

// create a library class instance called LT
// to handle LoRa radio communications

#ifdef SX126X
SX126XLT LT;
#endif

#ifdef SX127X
SX127XLT LT;
#endif

#ifdef SX128X
SX128XLT LT;
#endif

// keep track of the number of successful transmissions
uint32_t TXPacketCount=0;
#endif

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
  while(!Serial);//Ensure that the serial port is enabled
  //Serial.setDebugOutput(true);
  Serial.println();

#ifdef ESP32
  Serial.print("ESP32 detected\n");
#endif
#if defined ESP32S3_DEV || defined MY_FREENOVE_ESP32S3_CAM
  Serial.print("FREENOVE_ESP32S3_CAM variant (test)\n");
#endif
#if defined MY_FREENOVE_ESP32_CAM_DEV || defined MY_NONAME_ESP32_CAM_DEV
  Serial.print("ESP32_CAM_DEV variant\n");
#endif
#if defined MY_XIAO_ESP32S3_SENSE
  Serial.print("XIAO-ESP32S3-Sense variant with camera board\n");
#endif

#ifdef WITH_LORA_MODULE
#if defined MY_XIAO_ESP32S3 || defined MY_XIAO_ESP32S3_SENSE
  pinMode(2, OUTPUT); //GPIO 2 = D1
  pinMode(1, OUTPUT); //GPIO 1 = D0
  pinMode(4, INPUT);  //GPIO 4 = D3
  pinMode(5, OUTPUT); //GPIO 5 = D4
  delay(10); 
  Serial.print("Setting SPI pins for XIAO-ESP32S3-Sense\n");
  //        SCK, MISO, MOSI, CS/SS 
  SPI.begin(1,   4,    5,    2);
#elif defined MY_FREENOVE_ESP32S3_CAM
  pinMode(21, OUTPUT); //GPIO 21 = 21
  pinMode(47, OUTPUT); //GPIO 47 = 47
  pinMode(42, INPUT);  //GPIO 42 = 42
  pinMode(41, OUTPUT); //GPIO 41 = 41
  delay(10);
  Serial.print("Setting SPI pins for FREENOVE-ESP32S3-CAM\n");
  //        SCK, MISO,  MOSI, CS/SS 
  SPI.begin(47,  42,    41,   21);
 #elif MY_FREENOVE_ESP32_CAM_DEV || defined MY_NONAME_ESP32_CAM_DEV
  pinMode(32, OUTPUT);
  pinMode(12, OUTPUT); 
  pinMode(13, INPUT);  
  pinMode(14, OUTPUT);
  delay(10); 
  //        SCK, MISO,  MOSI, CS/SS 
  SPI.begin(12,  13,    14,   32);   
#else
  //start SPI bus communication
  SPI.begin();
#endif

  //setup hardware pins used by device, then check if device is found
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
  }
  else
  {
    Serial.print("No device responding\n");
    while (1){ }
  }

/*******************************************************************************************************
  Based from SX12XX example - Stuart Robinson 
*******************************************************************************************************/

  //The function call list below shows the complete setup for the LoRa device using the
  // information defined in the Settings.h file.
  //The 'Setup LoRa device' list below can be replaced with a single function call;
  //LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup LoRa device
  //***************************************************************************************************
  //got to standby mode to configure device
  LT.setMode(MODE_STDBY_RC);
#ifdef SX126X
  LT.setRegulatorMode(USE_DCDC);
  LT.setPaConfig(0x04, PAAUTO, LORA_DEVICE);
  LT.setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
  LT.calibrateDevice(ALLDevices);                //is required after setting TCXO
  LT.calibrateImage(DEFAULT_CHANNEL);
  LT.setDIO2AsRfSwitchCtrl();
#endif
#ifdef SX128X
  LT.setRegulatorMode(USE_LDO);
#endif
  //set for LoRa transmissions                              
  LT.setPacketType(PACKET_TYPE_LORA);
  //set the operating frequency                 
#ifdef MY_FREQUENCY
  LT.setRfFrequency(MY_FREQUENCY, Offset);
#else  
  LT.setRfFrequency(DEFAULT_CHANNEL, Offset);                   
#endif
//run calibration after setting frequency
#ifdef SX127X
  LT.calibrateImage(0);
#endif
  //set LoRa modem parameters
#if defined SX126X || defined SX127X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif                                     
  //where in the SX buffer packets start, TX and RX
  LT.setBufferBaseAddress(0x00, 0x00);
  //set packet parameters
#if defined SX126X || defined SX127X                     
  LT.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
#endif
#ifdef SX128X
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
#endif
  //syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34 
#if defined SX126X || defined SX127X
#if defined PUBLIC_SYNCWORD || defined LORAWAN
  LT.setSyncWord(LORA_MAC_PUBLIC_SYNCWORD);              
#else
  LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
#endif
  //set for highest sensitivity at expense of slightly higher LNA current
  LT.setHighSensitivity();  //set for maximum gain
#endif
#ifdef SX126X
  //set for IRQ on TX done and timeout on DIO1
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
  //set for IRQ on RX done
  LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);
  LT.setPA_BOOST(PA_BOOST);
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif   

  if (IQ_Setting==LORA_IQ_INVERTED) {
    LT.invertIQ(true);
    Serial.print("Invert I/Q on RX\n");
  }
  else {
    LT.invertIQ(false);
    Serial.print("Normal I/Q\n");
  }  
   
  //***************************************************************************************************

  Serial.println();
  //reads and prints the configured LoRa settings, useful check
  LT.printModemSettings();                               
  Serial.println();
  //reads and prints the configured operating settings, useful check
  LT.printOperatingSettings();                           
  Serial.println();
  Serial.println();
#if defined SX126X || defined SX127X  
  //print contents of device registers, normally 0x00 to 0x4F
  LT.printRegisters(0x00, 0x4F);
#endif                       
#ifdef SX128X
  //print contents of device registers, normally 0x900 to 0x9FF 
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
#endif //#ifdef WITH_LORA_MODULE

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
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
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
}

void loop() {
  // Do nothing. Everything is done in another task by the web server
  delay(10000);

#ifdef WITH_CUSTOM_CAM
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

  //here we pass buf as image buffer to our encoding procedure
  encode_image(buf, false);

  free(buf);
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
  uint64_t fr_end = esp_timer_get_time();
#endif
  log_i("BMP: %llums, %uB", (uint64_t)((fr_end - fr_start) / 1000), buf_len);
#endif
}
