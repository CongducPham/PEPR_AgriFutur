
#ifndef wazidev_h
#define wazidev_h

#include "SX1272.h"
#include "Utils.h"

#define ERR_LORAWAN_MIC -1 // The calculated MIC is wrong, the sender and receive migth have different keys.
#define ERR_LORAWAN_PAYLOAD -3 // The LoRaWAN payload is mal formmatted and might be corrupted.
#define ERR_LORA_CRC 1 // A LoRa frame has been received on air, but the CRC was wrong. The message might be corrupted.
#define ERR_LORA_TIMEOUT 2 // No frame was received withing the tiomeout period.

#define LORA_CR_5 0x01
#define LORA_CR_6 0x02
#define LORA_CR_7 0x03
#define LORA_CR_8 0x04

#define LORA_SF_6 0x06
#define LORA_SF_7 0x07
#define LORA_SF_8 0x08
#define LORA_SF_9 0x09
#define LORA_SF_10 0x0A
#define LORA_SF_11 0x0B
#define LORA_SF_12 0x0C

#define LORA_BW_7_8 0x00
#define LORA_BW_10_4 0x01
#define LORA_BW_15_6 0x02
#define LORA_BW_20_8 0x03
#define LORA_BW_31_25 0x04
#define LORA_BW_41_7 0x05
#define LORA_BW_62_5 0x06
#define LORA_BW_125 0x07
#define LORA_BW_250 0x08
#define LORA_BW_500 0x09

class WaziDev
{

public:
    WaziDev();

    // Initialize the LoRa hardware radio chip and store the key and the devAddr.
    uint8_t setupLoRaWAN(const uint8_t *devAddr, const uint8_t *key);
    uint8_t setupLoRaWAN(const uint8_t *devAddr, const uint8_t *appSkey, const uint8_t *nwkSkey);

    // Initialize the LoRa hardware radio chip. 
    uint8_t setupLoRa();

    // Change the LoRa (and LoRaWAN) frequency in Hz.
    // Default: 868100000 (868.1 MHz).
    uint8_t setLoRaFreq(uint32_t freq);
    // Change the LoRa (and LoRaWAN) coding rate.
    // Default: 4/5 (LORA_CR_5)
    // Use LORA_CR_5, LORA_CR_6, LORA_CR_7 or LORA_CR_8
    uint8_t setLoRaCR(uint8_t cr);
    // Change the LoRa (and LoRaWAN) bandwidth.
    // Default: 125 kHz (LORA_BW_125)
    // Use LORA_BW_7_8 (7.8 kHz), LORA_BW_10_4 (10.4 kHz), LORA_BW_15_6 (15.6 kHz), LORA_BW_20_8 (20.8 kHz), LORA_BW_31_25 (31.25 kHz), LORA_BW_41_7 (41.7 kHz), LORA_BW_62_5 (62.5 kHz), LORA_BW_125 (125 kHz), LORA_BW_250 (250 kHz) or LORA_BW_500 (500 kHz).
    uint8_t setLoRaBW(uint16_t bw);
    // Change the LoRa (and LoRaWAN) spreading factor.
    // Default: SF12 (LORA_SF_12)
    // LORA_SF_6 (SF6), LORA_SF_7 (SF7), ..., LORA_SF_12 (SF12).
    uint8_t setLoRaSF(uint8_t sf);

    // Send a decrypted LoRaWAN message on air.
    uint8_t sendLoRaWAN(void *pl, uint8_t len);
    uint8_t sendLoRaWAN(void *pl, uint8_t len, bool invert);

    // Send a raw LoRa message on air.
    uint8_t sendLoRa(void *pl, uint8_t len);
    uint8_t sendLoRa(void *pl, uint8_t len, bool invert);

    // Receives raw LoRa data on air.
    // Returns an error (if any):
    // 0: The command has been executed with no errors.
    // 1: There has been an error while executing the command.
    // 2: No data received within the timeout period.
    uint8_t receiveLoRa(void *pl, uint8_t *len, uint16_t timeout);
    uint8_t receiveLoRa(void *pl, uint8_t *len, uint16_t timeout, bool invert);

    // Receives LoRaWAN data on air and decrypts the message.
    // Returns an error != 0 (if any): ERR_LORA_CRC, ERR_LORA_TIMEOUT
    uint8_t receiveLoRaWAN(void *pl, uint8_t *offs, uint8_t *len, uint16_t timeout);
    uint8_t receiveLoRaWAN(void *pl, uint8_t *offs, uint8_t *len, uint16_t timeout, bool invert);

    // LoRa SNR (signal-to-noise ratio) value in dB. Higher means better.
    // It will be -20dB (below noise floor) to +10dB (above noise floor).
    // The noise floor is an area of all unwanted interfering signal sources which can corrupt the transmitted signal.
    // It is updated after receiveLoRa() (or LoRaWAN) completed.
    // Learn more: https://lora.readthedocs.io/en/latest/#snr
    int8_t loRaSNR;
    
    // LoRa rssi (received signal strength indication) value in dBm. Higher means better.
    // It will be -120dBm (weak signal) to -30dBm (strong signal).
    // It is updated after receiveLoRa() (or LoRaWAN) completed.
    // Learn more: https://lora.readthedocs.io/en/latest/#snr
    int8_t loRaRSSI;
};

int printBase64(const void *buf, int len);

#endif
