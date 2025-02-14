#ifndef CONFIG_SETTINGS
#define CONFIG_SETTINGS

//#define ORIGINAL_CONFIG

#ifdef ORIGINAL_CONFIG
    #define WITH_WEB_SERVER
#else
    #define TEST_IN_PROGRESS 10 // in minutes
    //#define TEST_ENERGY
    //#define WITH_WEB_SERVER
    #define WITH_CUSTOM_CAM
    #define WITH_LORA_MODULE
    //#define WAIT_FOR_SERIAL_INPUT
    // replace LOW_POWER as an external microcontroller will control the power cycling
    #define RUN_AS_SLAVE
    // with RUN_AS_SAVE, will indicate activity with this pin set to HIGH
    //#define ACTIVITY_PIN1 41 // GPIO 41 = D12    
    //#define ACTIVITY_PIN2 42 // GPIO 42 = D11
    #define ACTIVITY_PIN3 3 // GPIO 3 = D2/A2    
    // incompatible with WITH_WEB_SERVER and WAIT_FOR_SERIAL_INPUT, but better to keep it even with RUN_AS_SLAVE
    #define LOW_POWER 
    #define LOW_POWER_DEEP_SLEEP       
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