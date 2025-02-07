#include "custom_cam.h"
#include "mqc.h"
#include "ConfigSettings.h" 

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

#ifdef LORA_UCAM
unsigned int MSS = DEFAULT_LORA_MSS;
#endif

// Note: we are slightly changing the packet format as previously defined in
// https://cpham.perso.univ-pau.fr/WSN-MODEL/tool-html/imagesensor.html
#ifdef WITH_SRC_ADDR
// 0xFE 0x50->0x54 for image packets, node addr, sequence number, quality factor and packet size
uint8_t pktPreamble[PREAMBLE_SIZE] = {0xFE, 0x50, UCAM_ADDR, 0x00, 0x00, 0x00};
#else
// 0xFF 0x50->0x54 for image packets, sequence number, quality factor and packet size
uint8_t pktPreamble[PREAMBLE_SIZE] = {0xFF, 0x50, 0x00, 0x00, 0x00};
#endif

// default behavior is to use framing bytes
bool with_framing_bytes = true;
uint8_t currentCam=0;

#ifdef USEREFIMAGE
// if true, try to allocate memory space to store the reference image, if not enough memory then
// stop
bool useRefImage = true;
#else
bool useRefImage = false;
#endif

bool send_image_on_intrusion = true;
bool transmitting_data = false;
bool new_ref_on_intrusion = true;

// for the incoming data from uCam
InImageStruct inImage;
// for the reference images
InImageStruct refImage;

#ifdef CRAN_NEW_CODING
InImageStruct outImage;
#else
OutImageStruct outImage;
#endif

int nbIntrusion = 0;
bool detectedIntrusion = false;

#ifdef LUM_HISTO
// to store the image histogram
short histoRefImage[255];
short histoInImage[255];
long refImageLuminosity;
long inImageLuminosity;
#endif

unsigned long totalPacketizationDuration=0;
unsigned int packetcount = 0;
long count = 0L;
unsigned int QualityFactor;

// will wrap at 255
uint8_t nbSentPackets = 0;
unsigned long lastSentTime = 0;
unsigned long lastSendDuration = 0;
unsigned long inter_binary_pkt=DEFAULT_INTER_PKT_TIME;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMAGE ENCODING METHOD FROM CRAN LABORATORY
///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct position {
    uint8_t row;
    uint8_t col;
} ZigzagCoordinates[8 * 8] =  // Matrice Zig-Zag
    {0, 0, 0, 1, 1, 0, 2, 0, 1, 1, 0, 2, 0, 3, 1, 2, 2, 1, 3, 0, 4, 0, 3, 1, 2, 2,
     1, 3, 0, 4, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1, 5, 0, 6, 0, 5, 1, 4, 2, 3, 3, 2, 4,
     1, 5, 0, 6, 0, 7, 1, 6, 2, 5, 3, 4, 4, 3, 5, 2, 6, 1, 7, 0, 7, 1, 6, 2, 5, 3,
     4, 4, 3, 5, 2, 6, 1, 7, 2, 7, 3, 6, 4, 5, 5, 4, 6, 3, 7, 2, 7, 3, 6, 4, 5, 5,
     4, 6, 3, 7, 4, 7, 5, 6, 6, 5, 7, 4, 7, 5, 6, 6, 5, 7, 6, 7, 7, 6, 7, 7};

uint8_t **AllocateUintMemSpace(int Horizontal, int Vertical) {
    uint8_t **Array;

    if ((Array = (uint8_t **)calloc(Vertical, sizeof(uint8_t *))) == NULL) return NULL;

#ifdef ALLOCATE_DEDICATED_INIMAGE_BUFFER
    for (int i = 0; i < Vertical; i++) {
        Array[i] = (uint8_t *)calloc(Horizontal, sizeof(uint8_t));
        if (Array[i] == NULL) return NULL;
    }
#endif
    return Array;
}

float **AllocateFloatMemSpace(int Horizontal, int Vertical) {
    float **Array;

    if ((Array = (float **)calloc(Vertical, sizeof(float *))) == NULL) return NULL;

    for (int i = 0; i < Vertical; i++) {
        Array[i] = (float *)calloc(Horizontal, sizeof(float));
        if (Array[i] == NULL) return NULL;
    }

    return Array;
}

#ifdef SHORT_COMPUTATION
short **AllocateShortMemSpace(int Horizontal, int Vertical) {
    short **Array;

    if ((Array = (short **)calloc(Vertical, sizeof(short *))) == NULL) return NULL;

    for (int i = 0; i < Vertical; i++) {
        Array[i] = (short *)calloc(Horizontal, sizeof(short));
        if (Array[i] == NULL) return NULL;
    }

    return Array;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// NEW CRAN ENCODING, WITH PACKET CREATION ON THE FLY
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef CRAN_NEW_CODING

float CordicLoefflerScalingFactor[8] = {0.35355339, 0.35355339, 0.31551713, 0.5,
                                        0.35355339, 0.5,        0.31551713, 0.35355339};

short OriginalLuminanceJPEGTable[8][8] = {
    16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
    14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
    18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};

short LuminanceJPEGTable[8][8] = {16,  11,  10,  16,  24, 40, 51,  61,  12,  12,  14,  19,  26,
                                  58,  60,  55,  14,  13, 16, 24,  40,  57,  69,  56,  14,  17,
                                  22,  29,  51,  87,  80, 62, 18,  22,  37,  56,  68,  109, 103,
                                  77,  24,  35,  55,  64, 81, 104, 113, 92,  49,  64,  78,  87,
                                  103, 121, 120, 101, 72, 92, 95,  98,  112, 100, 103, 99};

opj_mqc_t mqobjet, mqbckobjet, *objet = NULL;
uint8_t buffer[MQC_NUMCTXS], bckbuffer[MQC_NUMCTXS];
uint8_t packet[MQC_NUMCTXS];
int packetsize, packetoffset, buffersize;

void QTinitialization(int Quality) {
    float Qs, scale;

    if (Quality <= 0) Quality = 1;
    if (Quality > 100) Quality = 100;
    if (Quality < 50)
        Qs = 50.0 / (float)Quality;
    else
        Qs = 2.0 - (float)Quality / 50.0;

    // Calcul des coefficients de la table de quantification
    for (int u = 0; u < 8; u++)
        for (int v = 0; v < 8; v++) {
            scale = (float)OriginalLuminanceJPEGTable[u][v] * Qs;

            if (scale < 1.0) scale = 1.0;

            LuminanceJPEGTable[u][v] = (short)round(
                scale / (CordicLoefflerScalingFactor[u] * CordicLoefflerScalingFactor[v]));
        }

    return;
}

void JPEGencoding(int Block[8][8]) {
    int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int tmp10, tmp11, tmp12, tmp13, tmp20, tmp23;
    int z11, z12, z21, z22;

    // On calcule la DCT, puis on quantifie
    for (int u = 0; u < 8; u++) {
        tmp0 = Block[u][0] + Block[u][7];
        tmp7 = Block[u][0] - Block[u][7];
        tmp1 = Block[u][1] + Block[u][6];
        tmp6 = Block[u][1] - Block[u][6];
        tmp2 = Block[u][2] + Block[u][5];
        tmp5 = Block[u][2] - Block[u][5];
        tmp3 = Block[u][3] + Block[u][4];
        tmp4 = Block[u][3] - Block[u][4];

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        Block[u][0] = tmp10 + tmp11;
        Block[u][4] = tmp10 - tmp11;
        z11 = tmp13 + tmp12;
        z12 = tmp13 - tmp12;
        z21 = z11 + (z12 >> 1);
        z22 = z12 - (z11 >> 1);
        Block[u][2] = z21 - (z22 >> 4);
        Block[u][6] = z22 + (z21 >> 4);

        z11 = tmp4 + (tmp7 >> 1);
        z12 = tmp7 - (tmp4 >> 1);
        z21 = z11 + (z12 >> 3);
        z22 = z12 - (z11 >> 3);
        z21 = z21 - (z21 >> 3);
        z22 = z22 - (z22 >> 3);
        tmp10 = z21 + (z21 >> 6);
        tmp13 = z22 + (z22 >> 6);
        z11 = tmp5 + (tmp6 >> 3);
        z12 = tmp6 - (tmp5 >> 3);
        tmp11 = z11 + (z12 >> 4);
        tmp12 = z12 - (z11 >> 4);

        tmp20 = tmp10 + tmp12;
        Block[u][5] = tmp10 - tmp12;
        tmp23 = tmp13 + tmp11;
        Block[u][3] = tmp13 - tmp11;
        Block[u][1] = tmp23 + tmp20;
        Block[u][7] = tmp23 - tmp20;
    }

    // On attaque ensuite colonne par colonne
    for (int v = 0; v < 8; v++) {
        // 1Ëre Ètape
        tmp0 = Block[0][v] + Block[7][v];
        tmp7 = Block[0][v] - Block[7][v];
        tmp1 = Block[1][v] + Block[6][v];
        tmp6 = Block[1][v] - Block[6][v];
        tmp2 = Block[2][v] + Block[5][v];
        tmp5 = Block[2][v] - Block[5][v];
        tmp3 = Block[3][v] + Block[4][v];
        tmp4 = Block[3][v] - Block[4][v];

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        Block[0][v] = tmp10 + tmp11;
        Block[4][v] = tmp10 - tmp11;
        z11 = tmp13 + tmp12;
        z12 = tmp13 - tmp12;
        z21 = z11 + (z12 >> 1);
        z22 = z12 - (z11 >> 1);
        Block[2][v] = z21 - (z22 >> 4);
        Block[6][v] = z22 + (z21 >> 4);

        z11 = tmp4 + (tmp7 >> 1);
        z12 = tmp7 - (tmp4 >> 1);
        z21 = z11 + (z12 >> 3);
        z22 = z12 - (z11 >> 3);
        z21 = z21 - (z21 >> 3);
        z22 = z22 - (z22 >> 3);

        tmp10 = z21 + (z21 >> 6);
        tmp13 = z22 + (z22 >> 6);
        z11 = tmp5 + (tmp6 >> 3);
        z12 = tmp6 - (tmp5 >> 3);
        tmp11 = z11 + (z12 >> 4);
        tmp12 = z12 - (z11 >> 4);

        tmp20 = tmp10 + tmp12;
        Block[5][v] = tmp10 - tmp12;
        tmp23 = tmp13 + tmp11;
        Block[3][v] = tmp13 - tmp11;
        Block[1][v] = tmp23 + tmp20;
        Block[7][v] = tmp23 - tmp20;
    }

    // on centre sur l'interval [-128, 127]
    Block[0][0] -= 8192;

    // Quantification
#ifdef DISPLAY_BLOCK
    Serial.println(F("JPEGencoding:"));

    for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
            Block[u][v] = (int)round((float)Block[u][v] / (float)LuminanceJPEGTable[u][v]);

            Serial.print(Block[u][v]);
            Serial.print(F("\t"));
        }
        Serial.println("");
    }
#else

    for (int u = 0; u < 8; u++)
        for (int v = 0; v < 8; v++)
            Block[u][v] = (int)round((float)Block[u][v] / (float)LuminanceJPEGTable[u][v]);

#endif

    return;
}

void CreateNewPacket(unsigned int BlockOffset) {
    // On initialise le codeur MQ
    objet = &mqobjet;
    for (int x = 0; x < MQC_NUMCTXS; x++) buffer[x] = 0;

    mqc_init_enc(objet, buffer);
    mqc_resetstates(objet);
    packetoffset = BlockOffset;
    packetsize = 0;
    mqc_backup(objet, &mqbckobjet, bckbuffer);
    mqc_flush(objet);
}

void SendPacket() {
    if (packetsize == 0) return;

#ifdef DISPLAY_PKT
    Serial.print(F("00"));
    Serial.print(packetsize + 2, HEX);
    Serial.print(F(" "));
    Serial.print(F("00 "));
    if (packetoffset < 0x10) Serial.print(F("0"));
    Serial.print(packetoffset, HEX);

    for (int x = 0; x < packetsize; x++) {
        Serial.print(F(" "));

        if (packet[x] < 0x10) Serial.print(F("0"));

        Serial.print(packet[x], HEX);
    }
    Serial.println(F(" "));
#endif

    if (transmitting_data) {
        //digitalWrite(capture_led, LOW);
        // here we transmit data

        if (inter_binary_pkt != MIN_INTER_PKT_TIME) {
            unsigned long now_millis = millis();

            Serial.println(now_millis);
            Serial.println(lastSentTime);

            if ((now_millis - lastSentTime) < inter_binary_pkt) {
                Serial.print("Wait for ");
                Serial.println(inter_binary_pkt - (now_millis - lastSentTime));
                delay(inter_binary_pkt - (now_millis - lastSentTime));
            }
        }

        // just the maximum pkt size plus some more bytes
        uint8_t myBuff[260];
        uint8_t dataSize = 0;
        long startSendTime, stopSendTime, previousLastSendTime;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// for instance, if Q=20 (0x14) and the 9th (seq number 8, 0x08) image packet is: 
//    003F 00E4C1129592397FDDBFF13790E9D5DC473DBDCDFDAA2031705C6D283EC3A4B316415CB9CD281CEFBF1B44A296533ED59645E1D33DF4824A5C4D514A8C0CF7
// then the generated payload is (with framing bytes and without WITH_SRC_ADDR)
//    FF 50 08 14 3F 00E4C1129592397FDDBFF13790E9D5DC473DBDCDFDAA2031705C6D283EC3A4B316415CB9CD281CEFBF1B44A296533ED59645E1D33DF4824A5C4D514A8C0CF7
// in LoRaWAN mode, the payload will be first encrypted (using AppSkey and NwkSkey in local_lorawan.cpp)
//    42B05B3C5BDA5AAD4A428D1D1E1517E5D74C4E3AC7464B295F20954CB01FB65CA10A37D841800B8B1BE175509025EFE7AC0D0ED7AD2EE439F954304BEC4E356749E2EE0E
// and put in a LoRaWAN packet with the LoRaWAN header
//    MHDR[1] | DevAddr[4] | FCtrl[1] | FCnt[2] | FPort[1] | EncryptedPayload | MIC[4]
//    40 AA2D0126 00 0800 01 42B05B3C5BDA5AAD4A428D1D1E1517E5D74C4E3AC7464B295F20954CB01FB65CA10A37D841800B8B1BE175509025EFE7AC0D0ED7AD2EE439F954304BEC4E356749E2EE0E 15893440
/////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (with_framing_bytes) {
            myBuff[0] = pktPreamble[0];
            myBuff[1] = 0x50 + currentCam;

#ifdef WITH_SRC_ADDR
            myBuff[2] = pktPreamble[2];
            // set the sequence number
            myBuff[3] = (uint8_t)packetcount;
            // set the Quality Factor
            myBuff[4] = (uint8_t)QualityFactor;
            // set the packet size
            myBuff[5] = (uint8_t)(packetsize + 2);
#else
            // set the sequence number
            myBuff[2] = (uint8_t)packetcount;
            // set the Quality Factor
            myBuff[3] = (uint8_t)QualityFactor;
            // set the packet size
            myBuff[4] = (uint8_t)(packetsize + 2);
#endif
            dataSize += sizeof(pktPreamble);
        }
#ifdef DISPLAY_PKT
        Serial.print(F("Building packet : "));
        Serial.println(packetcount);
#endif
        // high part
        myBuff[dataSize++] = packetoffset >> 8 & 0xff;
        // low part
        myBuff[dataSize++] = packetoffset & 0xff;

        for (int x = 0; x < packetsize; x++) {
            myBuff[dataSize + x] = (uint8_t)packet[x];
        }

#ifdef DISPLAY_PKT
        for (int x = 0; x < dataSize + packetsize; x++) {
            if (myBuff[x] < 0x10) Serial.print(F("0"));
            Serial.print(myBuff[x], HEX);
        }
        Serial.println("");

        Serial.print(F("Sending packet : "));
        Serial.println(packetcount);
#endif

#ifdef LORA_UCAM        
        int pl;

        pl = dataSize + packetsize;
        Serial.print("payload size is ");
        Serial.println(pl);   

#ifdef RAW_LORA
        uint8_t p_type=PKT_TYPE_DATA;
#if defined WITH_AES
        // indicate that payload is encrypted
        p_type = p_type | PKT_FLAG_DATA_ENCRYPTED;
#endif
#if defined WITH_ACK
        p_type=PKT_TYPE_DATA | PKT_FLAG_ACK_REQ;
        Serial.println("Will request an ACK");      
#endif  
#endif

///////////////////////////////////
//use AES (LoRaWAN-like) encrypting
///////////////////////////////////
#ifdef WITH_AES
#ifdef LORAWAN
        Serial.print("LoRaCAM uses native LoRaWAN packet format\n");
#else
        Serial.print("LoRaCAM uses encapsulated LoRaWAN packet format only for encryption\n");
#endif
        pl=local_aes_lorawan_create_pkt(myBuff, pl, 0);
#endif
        Serial.flush();
        
        previousLastSendTime = lastSentTime;

        startSendTime = millis();

        // we set lastSendTime before the call to the sending procedure in order to be able to send
        // packets back to back since the sending procedure can introduce a delay
        lastSentTime = (unsigned long)startSendTime;

#ifdef CUSTOM_LORAWAN
        // will return sent packet length if OK, otherwise 0 if transmission error
        // we use raw format for LoRaWAN
        if (LT.transmit(myBuff, pl, 10000, MAX_DBM, WAIT_TX))
#else
        // will return packet length sent if OK, otherwise 0 if transmit error
        if (LT.transmitAddressed(myBuff, pl, p_type, DEFAULT_DEST_ADDR, node_addr, 10000, MAX_DBM, WAIT_TX))
#endif
        {
            stopSendTime = millis();
            nbSentPackets++;;
            blinkLed(1, 200);

#if defined RAW_LORA && defined WITH_SPI_COMMANDS
            uint16_t localCRC = LT.CRCCCITT(myBuff, pl, 0xFFFF);
            Serial.printf("CRC %.4X\n", localCRC);

            if (LT.readAckStatus()) {
                Serial.printf("Received ACK from %d\n", LT.readRXSource());
                Serial.printf("SNR of transmitted pkt is %d\n", LT.readPacketSNRinACK());
            }
#endif
        }
        else // transmission error
        {
            stopSendTime=millis();
            blinkLed(4, 80);

#if defined RAW_LORA && defined WITH_SPI_COMMANDS
            // if here there was an error transmitting packet
            uint16_t IRQStatus;
            IRQStatus = LT.readIrqStatus();
            Serial.printf("SendError,IRQreg,%.4X\n", IRQStatus);
            LT.printIrqStatus();
#endif
        }
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

#ifdef DISPLAY_PKT

        Serial.print(F("Packet Sent in "));
        lastSendDuration = stopSendTime - startSendTime;
        Serial.println(lastSendDuration);

#ifdef LORA_UCAM
        //TODO: something else to do?
#endif
#endif

#ifdef DISPLAY_PACKETIZATION_SEND_STATS
        Serial.print(packetsize);
        Serial.print(" ");
        Serial.println(stopSendTime - startSendTime);
        // Serial.println("ms. ");
#endif
    }

    count += packetsize;
    packetcount++;
}

int FillPacket(int Block[8][8], bool *full) {
    unsigned int index, q, r, K;

    mqc_restore(objet, &mqbckobjet, bckbuffer);

    long startFillPacket = millis();

    // On cherche où se trouve le dernier coef <> 0 selon le zig-zag
    K = 63;

    while ((Block[ZigzagCoordinates[K].row][ZigzagCoordinates[K].col] == 0) && (K > 0)) K--;

    K++;

    // On code la valeur de K, nombre de coefs encodé dans le bloc
    q = K / 2;
    r = K % 2;

    for (int x = 0; x < q; x++) mqc_encode(objet, 1);

    mqc_encode(objet, 0);
    mqc_encode(objet, r);

    // On code chaque coef significatif par Golomb-Rice puis par MQ
    for (int x = 0; x < K; x++) {
        if (Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col] >= 0) {
            index = 2 * Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col];
        } else {
            index = 2 * abs(Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]) - 1;
        }

        // Golomb
        q = index / 2;
        r = index % 2;

        for (int x = 0; x < q; x++) mqc_encode(objet, 1);

        mqc_encode(objet, 0);
        mqc_encode(objet, r);
    }

    // On regarde si le paquet est plein
    mqc_backup(objet, &mqbckobjet, bckbuffer);
    mqc_flush(objet);
    buffersize = mqc_numbytes(objet);

    // On déborde (il faut tenir compte du champ offset (2 octets) dans le paquet
    if (buffersize > (MSS - 2)) {
        totalPacketizationDuration += millis() - startFillPacket;
        return -1;
    }

#ifdef DISPLAY_FILLPKT
    Serial.println(F("filling pkt\n"));
#endif
    packetsize = buffersize;

    for (int x = 0; x < packetsize; x++) {
#ifdef DISPLAY_FILLPKT
        if (buffer[x] < 0x10) Serial.print(F("0"));

        Serial.print(buffer[x], HEX);
        Serial.print(F(" "));
#endif
        packet[x] = buffer[x];
    }

#ifdef DISPLAY_FILLPKT
    Serial.println("");
#endif

    if (buffersize < (MSS - 6)) {
        *full = false;
    } else {
        *full = true;
    }

    totalPacketizationDuration += millis() - startFillPacket;

    return 0;
}

int encode_ucam_file_data() {
    unsigned int err, offset;
    bool RTS;
    float CompressionRate;
    int Block[8][8];
    int row, col, row_mix, col_mix, N;
    int i, j, w;

    long startCamGlobalEncodeTime = 0;
    long startEncodeTime = 0;
    long totalEncodeDuration = 0;
    long totalSendDuration = 0;
    totalPacketizationDuration = 0;

    Serial.print(F("Encoding picture data, Quality Factor is : "));
    Serial.println(QualityFactor);

    Serial.print(F("MSS for packetization is : "));
    Serial.println(MSS);

    startCamGlobalEncodeTime = millis();

    // Initialisation de la matrice de quantification
    QTinitialization(QualityFactor);

#ifdef DISPLAY_ENCODE_STATS
    Serial.print(F("Q: "));
    Serial.print(millis() - startCamGlobalEncodeTime);
#endif

    offset = 0;
    CreateNewPacket(offset);

    Serial.println(F("QT ok"));

    // N=16 for 128x128 image
    N = CAMDATA_LINE_SIZE / 8;

    for (row = 0; row < N; row++)
        // for a given row, we will have 2 main row_mix*8 values separated by 64 lines
        // then, for a row_mix value, we need to be able to browse 8 lines
        // Therefore we need to read 2x8lines in memory from SD file
        // read_image_block=true;

        for (col = 0; col < N; col++) {
            // determine starting point
            row_mix = ((row * 5) + (col * 8)) % N;
            col_mix = ((row * 8) + (col * 13)) % N;

            row_mix = row_mix * 8;
            col_mix = col_mix * 8;

#ifdef DISPLAY_BLOCK
            for (i = 0; i < 8; i++) {
                for (j = 0; j < 8; j++) {
                    Block[i][j] = (int)inImage.data[row_mix + i][col_mix + j];

                    if (Block[i][j] < 0x10) Serial.print(F("0"));

                    Serial.print(Block[i][j], HEX);
                    Serial.print(F(" "));
                }
                Serial.println("");
            }
#else
            startEncodeTime = millis();

            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++) Block[i][j] = (int)inImage.data[row_mix + i][col_mix + j];
#endif
            // Encodage JPEG du bloc 8x8
            JPEGencoding(Block);

            totalEncodeDuration += millis() - startEncodeTime;

#ifdef DISPLAY_BLOCK
            Serial.println(F("encode_ucam_file_data():"));

            for (int u = 0; u < 8; u++) {
                for (int v = 0; v < 8; v++) {
                    Serial.print(Block[u][v]);
                    Serial.print(F("\t"));
                }
                Serial.println("");
            }

#endif
            err = FillPacket(Block, &RTS);

            if (err == -1) {
#ifdef DISPLAY_FILLPKT
                Serial.println(F("err"));
#endif
                SendPacket();
                totalSendDuration += lastSendDuration;
                CreateNewPacket(offset);
                FillPacket(Block, &RTS);
            }

            offset++;

            if (RTS == true) {
                SendPacket();
                totalSendDuration += lastSendDuration;
                CreateNewPacket(offset);
                RTS = false;
            }
        }

    SendPacket();
    totalSendDuration += lastSendDuration;

    Serial.printf("Total encode+packetization+transmission (ms): %ld\n", millis() - startCamGlobalEncodeTime);
    Serial.printf("Encode (ms): %ld\n", totalEncodeDuration);
    Serial.printf("Packetisation (ms): %ld\n", totalPacketizationDuration);
    Serial.printf("Transmission (ms): %ld\n", totalSendDuration);    

    CompressionRate = (float)count * 8.0 / (CAMDATA_LINE_SIZE * CAMDATA_LINE_SIZE);
    Serial.printf("Compression rate (bpp) : %f\n", CompressionRate);
    Serial.printf("Packets : %d %.2X\n", packetcount, packetcount);
    Serial.printf("Q : %d %.2X\n", QualityFactor, QualityFactor);
    Serial.printf("H : %d %.2X\n", CAMDATA_LINE_SIZE, CAMDATA_LINE_SIZE);
    Serial.printf("V : %d %.2X\n", CAMDATA_LINE_SIZE, CAMDATA_LINE_SIZE);
    Serial.printf("Real encoded image file size : %d\n", count);

    // reset
    packetcount = 0L;
    count = 0L;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// OLD CRAN ENCODING, NEED MORE MEMORY
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#else

#define a4 1.38703984532215
#define a7 -0.275899379282943
#define a47 0.831469612302545
#define a5 1.17587560241936
#define a6 -0.785694958387102
#define a56 0.98078528040323
#define a2 1.84775906502257
#define a3 0.765366864730179
#define a23 0.541196100146197
#define a32 1.306562964876376
#define rc2 1.414213562373095

float OriginalLuminanceJPEGTable[8][8] = {
    16.0,  11.0,  10.0,  16.0,  24.0, 40.0, 51.0,  61.0,  12.0,  12.0,  14.0,  19.0,  26.0,
    58.0,  60.0,  55.0,  14.0,  13.0, 16.0, 24.0,  40.0,  57.0,  69.0,  56.0,  14.0,  17.0,
    22.0,  29.0,  51.0,  87.0,  80.0, 62.0, 18.0,  22.0,  37.0,  56.0,  68.0,  109.0, 103.0,
    77.0,  24.0,  35.0,  55.0,  64.0, 81.0, 104.0, 113.0, 92.0,  49.0,  64.0,  78.0,  87.0,
    103.0, 121.0, 120.0, 101.0, 72.0, 92.0, 95.0,  98.0,  112.0, 100.0, 103.0, 99.0};

float LuminanceJPEGTable[8][8] = {
    16.0,  11.0,  10.0,  16.0,  24.0, 40.0, 51.0,  61.0,  12.0,  12.0,  14.0,  19.0,  26.0,
    58.0,  60.0,  55.0,  14.0,  13.0, 16.0, 24.0,  40.0,  57.0,  69.0,  56.0,  14.0,  17.0,
    22.0,  29.0,  51.0,  87.0,  80.0, 62.0, 18.0,  22.0,  37.0,  56.0,  68.0,  109.0, 103.0,
    77.0,  24.0,  35.0,  55.0,  64.0, 81.0, 104.0, 113.0, 92.0,  49.0,  64.0,  78.0,  87.0,
    103.0, 121.0, 120.0, 101.0, 72.0, 92.0, 95.0,  98.0,  112.0, 100.0, 103.0, 99.0};

void QTinitialization(int Quality) {
    float Qs;

    if (Quality <= 0) Quality = 1;
    if (Quality > 100) Quality = 100;
    if (Quality < 50)
        Qs = 50.0 / (float)Quality;
    else
        Qs = 2.0 - (float)Quality / 50.0;

    // Calcul des coefficients de la table de quantification
    for (int u = 0; u < 8; u++)
        for (int v = 0; v < 8; v++) {
            LuminanceJPEGTable[u][v] = OriginalLuminanceJPEGTable[u][v] * Qs;
            if (LuminanceJPEGTable[u][v] < 1.0) LuminanceJPEGTable[u][v] = 1.0;
            if (LuminanceJPEGTable[u][v] > 255.0) LuminanceJPEGTable[u][v] = 255.0;
        }

    return;
}

void JPEGencoding(InImageStruct *InputImage, OutImageStruct *OutputImage) {
    float Block[8][8];
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp10, tmp11, tmp12, tmp13, tmp20, tmp23;
    float tmp;

    // Encodage bloc par bloc
    for (int i = 0; i < InputImage->imageVsize; i = i + 8)
        for (int j = 0; j < InputImage->imageHsize; j = j + 8) {
            for (int u = 0; u < 8; u++)
                for (int v = 0; v < 8; v++) Block[u][v] = InputImage->data[i + u][j + v];

            // On calcule la DCT, puis on quantifie
            for (int u = 0; u < 8; u++) {
                // 1ère étape
                tmp0 = Block[u][0] + Block[u][7];
                tmp7 = Block[u][0] - Block[u][7];
                tmp1 = Block[u][1] + Block[u][6];
                tmp6 = Block[u][1] - Block[u][6];  // soit 8 ADD
                tmp2 = Block[u][2] + Block[u][5];
                tmp5 = Block[u][2] - Block[u][5];
                tmp3 = Block[u][3] + Block[u][4];
                tmp4 = Block[u][3] - Block[u][4];
                // 2ème étape: Partie paire
                tmp10 = tmp0 + tmp3;
                tmp13 = tmp0 - tmp3;  // soit 4 ADD
                tmp11 = tmp1 + tmp2;
                tmp12 = tmp1 - tmp2;
                // 3ème étape: Partie paire
                Block[u][0] = tmp10 + tmp11;
                Block[u][4] = tmp10 - tmp11;
                tmp = (tmp12 + tmp13) * a23;  // soit 3 MULT et 5 ADD
                Block[u][2] = tmp + (a3 * tmp13);
                Block[u][6] = tmp - (a2 * tmp12);
                // 2ème étape: Partie impaire
                tmp = (tmp4 + tmp7) * a47;
                tmp10 = tmp + (a7 * tmp7);
                tmp13 = tmp - (a4 * tmp4);
                tmp = (tmp5 + tmp6) * a56;  // soit 6 MULT et 6 ADD
                tmp11 = tmp + (a6 * tmp6);
                tmp12 = tmp - (a5 * tmp5);
                // 3ème étape: Partie impaire
                tmp20 = tmp10 + tmp12;
                tmp23 = tmp13 + tmp11;
                Block[u][7] = tmp23 - tmp20;  // soit 2 MULT et 6 ADD
                Block[u][1] = tmp23 + tmp20;
                Block[u][3] = (tmp13 - tmp11) * rc2;
                Block[u][5] = (tmp10 - tmp12) * rc2;
            }

            for (int v = 0; v < 8; v++) {
                // 1ère étape
                tmp0 = Block[0][v] + Block[7][v];
                tmp1 = Block[1][v] + Block[6][v];
                tmp2 = Block[2][v] + Block[5][v];
                tmp3 = Block[3][v] + Block[4][v];
                tmp4 = Block[3][v] - Block[4][v];
                tmp5 = Block[2][v] - Block[5][v];
                tmp6 = Block[1][v] - Block[6][v];
                tmp7 = Block[0][v] - Block[7][v];
                // 2ème étape: Partie paire
                tmp10 = tmp0 + tmp3;
                tmp13 = tmp0 - tmp3;
                tmp11 = tmp1 + tmp2;
                tmp12 = tmp1 - tmp2;
                // 3ème étape: Partie paire
                Block[0][v] = tmp10 + tmp11;
                Block[4][v] = tmp10 - tmp11;
                tmp = (tmp12 + tmp13) * a23;
                Block[2][v] = tmp + (a3 * tmp13);
                Block[6][v] = tmp - (a2 * tmp12);
                // 2ème étape: Partie impaire
                tmp = (tmp4 + tmp7) * a47;
                tmp10 = tmp + (a7 * tmp7);
                tmp13 = tmp - (a4 * tmp4);
                tmp = (tmp5 + tmp6) * a56;
                tmp11 = tmp + (a6 * tmp6);
                tmp12 = tmp - (a5 * tmp5);
                // 3ème étape: Partie impaire
                tmp20 = tmp10 + tmp12;
                tmp23 = tmp13 + tmp11;
                Block[7][v] = tmp23 - tmp20;
                Block[1][v] = tmp23 + tmp20;
                Block[3][v] = (tmp13 - tmp11) * rc2;
                Block[5][v] = (tmp10 - tmp12) * rc2;
            }

            // on centre sur l'interval [-128, 127]
            Block[0][0] -= 8192.0;

            // Quantification
            for (int u = 0; u < 8; u++)
                for (int v = 0; v < 8; v++)
                    Block[u][v] = round(Block[u][v] / (LuminanceJPEGTable[u][v] * 8.0));

            // On range le résultat dans l'image de sortie
            for (int u = 0; u < 8; u++)
                for (int v = 0; v < 8; v++)
#ifdef SHORT_COMPUTATION
                    OutputImage->data[i + u][j + v] = (short)Block[u][v];
#else
                    OutputImage->data[i + u][j + v] = Block[u][v];
#endif
        }
    return;
}

unsigned int JPEGpacketization(OutImageStruct *InputImage, unsigned int BlockOffset) {
    int Block[8][8], row, col, row_mix, col_mix;
    short packetsize, packetoffset, buffersize;
    unsigned int index, q, r, K;
    opj_mqc_t mqobjet, mqbckobjet, *objet = NULL;
    unsigned char buffer[MQC_NUMCTXS], bckbuffer[MQC_NUMCTXS];
    unsigned char packet[MQC_NUMCTXS];
    bool loop = true, overhead = true;

    // On crée et on initialise le codeur MQ
    objet = &mqobjet;
    for (int x = 0; x < MQC_NUMCTXS; x++) buffer[x] = 0;
    mqc_init_enc(objet, buffer);
    mqc_resetstates(objet);
    packetoffset = BlockOffset;

    while ((loop == true) &&
           (BlockOffset != (InputImage->imageHsize * InputImage->imageVsize / 64))) {
        // On lit le bloc
        row = (BlockOffset * 8) / InputImage->imageHsize * 8;
        col = (BlockOffset * 8) % InputImage->imageHsize;
        row_mix = ((row * 5) + (col * 8)) % (InputImage->imageHsize);
        col_mix = ((row * 8) + (col * 13)) % (InputImage->imageVsize);

        // printf("(%d %d) ", row_mix, col_mix);

        for (int u = 0; u < 8; u++)
            for (int v = 0; v < 8; v++)
#ifdef SHORT_COMPUTATION
                Block[u][v] = (short)InputImage->data[row_mix + u][col_mix + v];
#else
                Block[u][v] = (int)round(InputImage->data[row_mix + u][col_mix + v]);
#endif
        // On cherche où se trouve le dernier coef <> 0 selon le zig-zag
        K = 63;

        while ((Block[ZigzagCoordinates[K].row][ZigzagCoordinates[K].col] == 0) && (K > 0)) K--;

        K++;

        // On code la valeur de K, nombre de coefs encodé dans le bloc
        q = K / 2;
        r = K % 2;

        for (int x = 0; x < q; x++) mqc_encode(objet, 1);

        mqc_encode(objet, 0);
        mqc_encode(objet, r);

        // On code chaque coef significatif par Golomb-Rice puis par MQ
        for (int x = 0; x < K; x++) {
            if (Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col] >= 0) {
                index = 2 * Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col];
            } else {
                index = 2 * abs(Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]) - 1;
            }

            // Golomb
            q = index / 2;
            r = index % 2;
            for (int x = 0; x < q; x++) mqc_encode(objet, 1);
            mqc_encode(objet, 0);
            mqc_encode(objet, r);
        }
        mqc_backup(objet, &mqbckobjet, bckbuffer);
        mqc_flush(objet);
        // On compte combien il y a de bits dans le code (octets entiers).
        buffersize = mqc_numbytes(objet);
        if (buffersize < (MSS - 2)) {
            overhead = false;
            packetsize = buffersize;
            for (int x = 0; x < packetsize; x++) packet[x] = buffer[x];
            BlockOffset++;
            mqc_restore(objet, &mqbckobjet, bckbuffer);
        } else {
            loop = false;
            if (overhead == true) {
                BlockOffset++;
                // break;
                return (BlockOffset);
            }
        }
    }

#ifdef DISPLAY_PKT
    Serial.print(F("00"));
    Serial.print(packetsize + 2, HEX);
    Serial.print(F(" "));
    Serial.print(F("00 "));
    if (packetoffset < 0x10) Serial.print(F("0"));
    Serial.print(packetoffset, HEX);

    for (int x = 0; x < packetsize; x++) {
        Serial.print(F(" "));

        if (packet[x] < 0x10) Serial.print(F("0"));

        Serial.print(packet[x], HEX);
    }
    Serial.println(F(" "));
#endif

    if (BlockOffset == (InputImage->imageHsize * InputImage->imageVsize / 64)) {
        Serial.println(F("last packet"));
    }

    if (transmitting_data) {
        //digitalWrite(capture_led, LOW);
        // here we transmit data

        if (inter_binary_pkt != MIN_INTER_PKT_TIME)
            while ((millis() - lastSentTime) < inter_binary_pkt);

        // just the maximum pkt size plus some more bytes
        uint8_t myBuff[260];
        uint8_t dataSize = 0;
        long startSendTime, stopSendTime, previousLastSendTime;

        if (with_framing_bytes) {
            myBuff[0] = pktPreamble[0];
            myBuff[1] = 0x50 + currentCam;

#ifdef WITH_SRC_ADDR
            myBuff[2] = pktPreamble[2];
            // set the sequence number
            myBuff[3] = (uint8_t)packetcount;
            // set the Quality Factor
            myBuff[4] = (uint8_t)QualityFactor;
            // set the packet size
            myBuff[5] = (uint8_t)(packetsize + 2);
#else
            // set the sequence number
            myBuff[2] = (uint8_t)packetcount;
            // set the Quality Factor
            myBuff[3] = (uint8_t)QualityFactor;
            // set the packet size
            myBuff[4] = (uint8_t)(packetsize + 2);
#endif
            dataSize += sizeof(pktPreamble);
        }
#ifdef DISPLAY_PKT
        Serial.print(F("Building packet : "));
        Serial.println(packetcount);
#endif
        // high part
        myBuff[dataSize++] = packetoffset >> 8 & 0xff;
        // low part
        myBuff[dataSize++] = packetoffset & 0xff;

        for (int x = 0; x < packetsize; x++) {
            myBuff[dataSize + x] = (uint8_t)packet[x];
        }

#ifdef DISPLAY_PKT
        for (int x = 0; x < dataSize + packetsize; x++) {
            if (myBuff[x] < 0x10) Serial.print("0");
            Serial.print(myBuff[x], HEX);
        }
        Serial.println("");

        Serial.print(F("Sending packet : "));
        Serial.println(packetcount);
#endif

#ifdef LORA_UCAM        
        int pl;

        pl = dataSize + packetsize;
        Serial.print("payload size is ");
        Serial.println(pl);   

#ifdef RAW_LORA
        uint8_t p_type=PKT_TYPE_DATA;
#if defined WITH_AES
        // indicate that payload is encrypted
        p_type = p_type | PKT_FLAG_DATA_ENCRYPTED;
#endif
#if defined WITH_ACK
        p_type=PKT_TYPE_DATA | PKT_FLAG_ACK_REQ;
        Serial.println("Will request an ACK");      
#endif  
#endif

///////////////////////////////////
//use AES (LoRaWAN-like) encrypting
///////////////////////////////////
#ifdef WITH_AES
#ifdef LORAWAN
        Serial.print("LoRaCAM uses native LoRaWAN packet format\n");
#else
        Serial.print("LoRaCAM uses encapsulated LoRaWAN packet format only for encryption\n");
#endif
        pl=local_aes_lorawan_create_pkt(myBuff, pl, 0);
#endif

        previousLastSendTime = lastSentTime;

        startSendTime = millis();

        // we set lastSendTime before the call to the sending procedure in order to be able to send
        // packets back to back since the sending procedure can introduce a delay
        lastSentTime = (unsigned long)startSendTime;

#ifdef CUSTOM_LORAWAN
        // will return sent packet length if OK, otherwise 0 if transmission error
        // we use raw format for LoRaWAN
        if (LT.transmit(myBuff, pl, 10000, MAX_DBM, WAIT_TX))
#else
        // will return packet length sent if OK, otherwise 0 if transmit error
        if (LT.transmitAddressed(myBuff, pl, p_type, DEFAULT_DEST_ADDR, node_addr, 10000, MAX_DBM, WAIT_TX))
#endif
        {
            stopSendTime = millis();
            nbSentPackets++;;

#if defined RAW_LORA && defined WITH_SPI_COMMANDS
            uint16_t localCRC = LT.CRCCCITT(myBuff, pl, 0xFFFF);
            Serial.printf("CRC %.4X\n", localCRC);

            if (LT.readAckStatus()) {
                Serial.printf("Received ACK from %d\n", LT.readRXSource());
                Serial.printf("SNR of transmitted pkt is %d\n", LT.readPacketSNRinACK());
            }
#endif
        }
        else // transmission error
        {
            stopSendTime=millis();
        
#if defined RAW_LORA && defined WITH_SPI_COMMANDS
            // if here there was an error transmitting packet
            uint16_t IRQStatus;
            IRQStatus = LT.readIrqStatus();
            Serial.printf("SendError,IRQreg,%.4X\n", IRQStatus);
            LT.printIrqStatus();
#endif
        }
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

#ifdef DISPLAY_PKT

        Serial.print(F("Packet Sent in "));
        lastSendDuration = stopSendTime - startSendTime;
        Serial.println(lastSendDuration);

#ifdef LORA_UCAM
        //TODO: something else to do?
#endif
#endif

#ifdef DISPLAY_PACKETIZATION_SEND_STATS
        Serial.print(packetsize);
        Serial.print(" ");
        Serial.println(stopSendTime - startSendTime);
        // Serial.println("ms. ");
#endif
    }

    count += packetsize;
    packetcount++;

    return (BlockOffset);
}

int encode_ucam_file_data() {
    unsigned int offset = 0;
    float CompressionRate;
    long startCamGlobalEncodeTime = 0;
    long startCamEncodeTime = 0;
    long startCamPktEncodeTime = 0;
    long stopCamPktEncodeTime = 0;
    long stopCamQuantizatioTime = 0;
    long totalSendDuration = 0;
    long totalEncodeDuration = 0;

    Serial.print(F("Encoding picture data, Quality Factor is : "));
    Serial.println(QualityFactor);

    Serial.print(F("MSS for packetization is : "));
    Serial.println(MSS);

    startCamGlobalEncodeTime = millis();

#ifdef DISPLAY_ENCODE_STATS

    // Initialisation de la matrice de quantification
    QTinitialization(QualityFactor);

    startCamEncodeTime = millis();

    // Encodage JPEG et fin
    JPEGencoding(&inImage, &outImage);

    Serial.print(F("Q: "));
    Serial.print(startCamEncodeTime - startCamGlobalEncodeTime);

    Serial.print(F(" E: "));
    totalEncodeDuration += millis() - startCamEncodeTime;
    Serial.println(millis() - startCamEncodeTime);
#else
    // Initialisation de la matrice de quantification
    QTinitialization(QualityFactor);

    // Encodage JPEG et fin
    JPEGencoding(&inImage, &outImage);
#endif

    // so that we can know the packetisation time without displaying stats for each packets
    startCamPktEncodeTime = millis();

    do {
#ifdef DISPLAY_PACKETIZATION_STATS
        startCamPktEncodeTime = millis();
        offset = JPEGpacketization(&outImage, offset);
        totalSendDuration += lastSendDuration;
        stopCamPktEncodeTime = millis();
        Serial.print(offset);
        Serial.print("|");
        Serial.println(stopCamPktEncodeTime - startCamPktEncodeTime);
#else
        offset = JPEGpacketization(&outImage, offset);
#endif
    } while (offset != (outImage.imageHsize * outImage.imageVsize / 64));

    Serial.printf("Total encode+packetization+transmission (ms): %ld\n", millis() - startCamGlobalEncodeTime);
    Serial.printf("Encode (ms): %ld\n", totalEncodeDuration);
    Serial.printf("Packetisation (ms): %ld\n", stopCamGlobalEncodeTime - startCamPktEncodeTime);
    Serial.printf("Transmission (ms): %ld\n", totalSendDuration);    

    CompressionRate = (float)count * 8.0 / (CAMDATA_LINE_SIZE * CAMDATA_LINE_SIZE);
    Serial.printf("Compression rate (bpp) : %f\n", CompressionRate);
    Serial.printf("Packets : %d %.2X\n", packetcount, packetcount);
    Serial.printf("Q : %d %.2X\n", QualityFactor, QualityFactor);
    Serial.printf("H : %d %.2X\n", CAMDATA_LINE_SIZE, CAMDATA_LINE_SIZE);
    Serial.printf("V : %d %.2X\n", CAMDATA_LINE_SIZE, CAMDATA_LINE_SIZE)
    Serial.printf("Real encoded image file size : %d\n", count); 

    // reset
    packetcount = 0L;
    count = 0L;
    return 0;
}
#endif  // #ifdef CRAN_NEW_CODING

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// END IMAGE ENCODING METHOD FROM CRAN LABORATORY
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef LUM_HISTO

void clearHistogram(short histo_data[]) {
    for (int i = 0; i < 255; i++) histo_data[i] = 0;
}

void computeHistogram(short histo_data[], uint8_t *data) {
    clearHistogram(histo_data);

    for (int x = 0; x < CAMDATA_LINE_SIZE; x++)
        for (int y = 0; y < CAMDATA_LINE_SIZE; y++) histo_data[data[x][y]]++;
}

long computeMeanLuminosity(short histo_data[]) {
    double luminosity = 0;

    for (int i = 0; i < 255; i++) luminosity += histo_data[i] * i;

    luminosity = luminosity / (CAMDATA_LINE_SIZE * CAMDATA_LINE_SIZE);

    return (long)luminosity;
}

#endif

void display_ucam_data() {
    int x = 0;
    int y = 0;
    int totalBytes = 0;

    for (y = 0; y < inImage.imageHsize; y++)
        for (x = 0; x < inImage.imageVsize; x++) {
            Serial.print(inImage.data[x][y]);
            Serial.print(" ");

            totalBytes++;

            if ((x + 1) % DISPLAY_CAM_DATA_SIZE == 0) {
                Serial.print("\n");
            }
        }

    Serial.print(F("\nTotal bytes read and displayed: "));
    Serial.println(totalBytes);
}

void copy_in_refImage() {
    if (useRefImage) {
        for (int x = 0; x < CAMDATA_LINE_SIZE; x++)
            for (int y = 0; y < CAMDATA_LINE_SIZE; y++)
                refImage.data[x][y] = inImage.data[x][y];

#ifdef LUM_HISTO
        computeHistogram(histoRefImage, refImage.data);
        refImageLuminosity = computeMeanLuminosity(histoRefImage);
#endif
    }
}

void init_custom_cam() {
    bool ok_to_read_picture_data=false;
    bool ok_to_encode_picture_data=false;

    QualityFactor = DEFAULT_QUALITY_FACTOR;

    inImage.imageVsize=inImage.imageHsize=CAMDATA_LINE_SIZE;
    outImage.imageVsize=outImage.imageHsize=CAMDATA_LINE_SIZE;

    // allocate memory to store the image from ucam
    if ((inImage.data = AllocateUintMemSpace(inImage.imageHsize, inImage.imageVsize))==NULL) {
        Serial.println(F("Error calloc inImage"));
        ok_to_read_picture_data=false;
    }
    else
        ok_to_read_picture_data=true;
        
    if (useRefImage) {
        if ((refImage.data = AllocateUintMemSpace(CAMDATA_LINE_SIZE, CAMDATA_LINE_SIZE))==NULL) {
            Serial.println(F("Error calloc refImage"));
            ok_to_read_picture_data=false;                      
        }
        else
            ok_to_read_picture_data=true;
    } 
    else
        refImage.data=NULL;     

    Serial.println(F("InImage memory allocation passed"));

#ifdef CRAN_NEW_CODING
    ok_to_encode_picture_data=true;
#else
#ifdef SHORT_COMPUTATION
    Serial.println(F("OutImage using short"));
    if ((outImage.data = AllocateShortMemSpace(outImage.imageHsize, outImage.imageVsize))==NULL) {
#else
    Serial.println(F("OutImage using float"));
    if ((outImage.data = AllocateFloatMemSpace(outImage.imageHsize, outImage.imageVsize))==NULL) {
#endif
        Serial.println(F("Error calloc outImage"));
        ok_to_encode_picture_data=false;
    }
    else {
        Serial.println(F("OutImage memory allocation passed"));
        ok_to_encode_picture_data=true;
    }        
#endif

    if (!ok_to_read_picture_data || !ok_to_encode_picture_data) {
        Serial.println(F("Sorry, stop process"));
        while (1)
        ;
    }
    else
        Serial.println(F("Ready to encode picture data"));     
}

void set_mss(uint8_t mss) {
    MSS=mss;
}

void set_quality_factor(uint8_t Q) {
    QualityFactor=Q;
}

int encode_image(uint8_t* buf, bool transmit) {
    transmitting_data=transmit;

    //buf contains the BMP Header, the grayscale palette and the image data
    //skip the signature that should be "BM"
    bmp_header_t * bitmap  = (bmp_header_t*)&buf[2];

    uint8_t* start_of_image = &buf[bitmap->fileoffset_to_pixelarray]; 

#ifdef ALLOCATE_DEDICATED_INIMAGE_BUFFER
    for (int i = 0; i < inImage.imageVsize; i++) {
        //copy data of each line
        memcpy(inImage.data[i], start_of_image+inImage.imageHsize*i, inImage.imageHsize);
    }    
#else
    for (int i = 0; i < inImage.imageVsize; i++) {
        //set pointers to beginning of each line
        inImage.data[i]=start_of_image+inImage.imageHsize*i;
    }
#endif

    return(encode_ucam_file_data());
}