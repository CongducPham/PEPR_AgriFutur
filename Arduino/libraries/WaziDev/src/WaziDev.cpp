#include "WaziDev.h"
// #include <SPI.h>
#include <EEPROM.h>
#include <LowPower.h>
#include <Base64.h>
#include "local_lorawan.h"


WaziDev::WaziDev() {}

unsigned char DevAddr[4];
unsigned char AppSkey[16];
unsigned char NwkSkey[16];

struct EEPROMConfig
{
    uint16_t head;
    uint8_t packetNumber;
};

uint8_t WaziDev::setupLoRaWAN(const uint8_t *devAddr, const uint8_t *key)
{
    return setupLoRaWAN(devAddr, key, key);
}

uint8_t WaziDev::setupLoRaWAN(const uint8_t *devAddr, const uint8_t *appSkey, const uint8_t *nwkSkey)
{
    memcpy(AppSkey, appSkey, 16);
    memcpy(NwkSkey, nwkSkey, 16);
    memcpy(DevAddr, devAddr, 4);
    return setupLoRa();
}

uint8_t WaziDev::setupLoRa()
{
    // Power ON the module
    int e = sx1272.ON();
    if (e != 0) return e;

    // get config from EEPROM
    EEPROMConfig c;
    EEPROM.get(0, c);

    // found a valid config?
    if (c.head == 0x1234)
    {
        // set sequence number for SX1272 library
        sx1272._packetNumber = c.packetNumber;
    }

    sx1272.setLORA();
    sx1272.setCR(CR_5);   // CR = 4/5
    sx1272.setSF(SF_12);  // SF = 12
    sx1272.setBW(BW_125); // BW = 125 KHz

    // set frequencie 868.1 MHz
    sx1272.setChannel(CH_18_868);

    // set the sync word to the LoRaWAN sync word which is 0x34
    sx1272.setSyncWord(0x34);

    // enable carrier sense
    sx1272._enableCarrierSense = true;

    // TODO: with low power, when setting the radio module in sleep mode
    // there seem to be some issue with RSSI reading
    sx1272._RSSIonSend = false;

    sx1272._needPABOOST = true;

    sx1272.setPowerDBM(14);

    sx1272._rawFormat = true;

    delay(500);
    return 0;
}

#define REG_INVERT_IQ 0x33
#define REG_INVERT_IQ2 0x3B

void setLoRaInversion(bool inv)
{
    if (inv)
    {
        sx1272.writeRegister(REG_INVERT_IQ, 0x66);
        sx1272.writeRegister(REG_INVERT_IQ2, 0x19);
    }
    else
    {
        sx1272.writeRegister(REG_INVERT_IQ, 0x27);
        sx1272.writeRegister(REG_INVERT_IQ2, 0x1D);
    }
}

uint8_t WaziDev::setLoRaFreq(uint32_t freq)
{
    uint32_t chan = (uint64_t (freq)) << 19 / 32000000;
    return sx1272.setChannel(chan);
}

uint8_t WaziDev::setLoRaCR(uint8_t cr)
{
    return sx1272.setCR(cr);
}

uint8_t WaziDev::setLoRaBW(uint16_t bw)
{
    return sx1272.setBW(bw);
}

uint8_t WaziDev::setLoRaSF(uint8_t sf)
{
    return sx1272.setSF(sf);
}

uint8_t WaziDev::sendLoRa(void *pl, uint8_t len, bool invert)
{
    sx1272._payloadlength = len;
    setLoRaInversion(invert);
    uint8_t e = sx1272.setPacket(0, (char *) pl);
    if (e != 0) return e;
    return sx1272.sendWithTimeout();
}

uint8_t WaziDev::sendLoRa(void *pl, uint8_t len)
{
    return sendLoRa(pl, len, false);
}

uint8_t WaziDev::sendLoRaWAN(void *pl, uint8_t len)
{
    return sendLoRaWAN(pl, len, false);
}

uint8_t WaziDev::sendLoRaWAN(void *pl, uint8_t len, bool invert)
{
    len = local_aes_lorawan_create_pkt((uint8_t *) pl, len, 0);
    return sendLoRa(pl, len, invert);
}

uint8_t WaziDev::receiveLoRa(void *pl, uint8_t* len, uint16_t timeout)
{
    return receiveLoRa(pl, len, timeout, true);
}

uint8_t WaziDev::receiveLoRa(void *pl, uint8_t* len, uint16_t timeout, bool invert)
{
    setLoRaInversion(invert);
    int e = sx1272.receiveAll(timeout);
    if (e == 0)
    {
        *len = sx1272._payloadlength;
        memcpy(pl, sx1272.packet_received.data, sx1272._payloadlength);
        sx1272.getSNR();
        loRaSNR = sx1272._SNR;
        sx1272.getRSSI();
        loRaRSSI = sx1272._RSSI;
    }
    return e;
}

uint8_t WaziDev::receiveLoRaWAN(void *pl, uint8_t* offs, uint8_t* len, uint16_t timeout, bool invert)
{
    int e = receiveLoRa(pl, len, timeout, invert);
    if (e == 0)
    {
        e = local_lorawan_decode_pkt((uint8_t*) pl, *len);
        if (e >= 0) {
            *len -= 4; // remove 4 byte MIC
            ((char*) pl)[*len] = 0; // null terminate
            *offs = e;
            *len -= e; // skip offset
            e = 0;
        }
    }
    return e;
}

uint8_t WaziDev::receiveLoRaWAN(void *pl, uint8_t* offs, uint8_t* len, uint16_t timeout)
{
    return receiveLoRaWAN(pl, offs, len, timeout, true);
}


int printBase64(const void *buf, int len)
{
    int encLen = base64_enc_len(len);
    char* h = (char*) malloc(encLen);
    base64_encode(h, buf, len); 
    Serial.print(h);
    free(h);
    return encLen;
}
