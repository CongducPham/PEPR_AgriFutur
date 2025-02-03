#ifndef CONFIG_SETTINGS
#define CONFIG_SETTINGS

//#define ORIGINAL_CONFIG

#ifdef ORIGINAL_CONFIG
    #define WITH_WEB_SERVER
#else
    #define TEST_IN_PROGRESS 10 // in minutes
    //#define WITH_WEB_SERVER
    #define WITH_CUSTOM_CAM
    #define WITH_LORA_MODULE
    //#define WAIT_FOR_SERIAL_INPUT
    #define LOW_POWER // incompatible with WITH_WEB_SERVER and WAIT_FOR_SERIAL_INPUT
    #define LOW_POWER_DEEP_SLEEP
    //LOW_POWER_LIGHT_SLEEP not tested yet
    //#define LOW_POWER_LIGHT_SLEEP        
#endif

// if the XIAO_ESP32S3_SENSE is not automatically detected
#define MY_XIAO_ESP32S3_SENSE
// if the FREENOVE_ESP32S3_CAM is not automatically detected
//#define MY_FREENOVE_ESP32S3_CAM
// if the FREENOVE_ESP32_CAM_DEV is not automatically detected, board v1.6
//#define MY_FREENOVE_ESP32_CAM_DEV
// if the NONAME_ESP32_CAM_DEV is not automatically detected, similar to FREENOVE_ESP32_CAM_DEV
//#define MY_NONAME_ESP32_CAM_DEV

#endif