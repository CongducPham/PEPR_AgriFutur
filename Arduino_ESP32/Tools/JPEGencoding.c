/************************************************************************
 *																																			*
 *	Fast DCT for image compression scheme																*
 *																																			*
 *	Author: Vincent LECUIRE, CRAN UMR 7039, Nancy-Université, CNRS			*
 *	Date: march, 16 2012																								*
 *																																			*
 ************************************************************************/

/* modified and enhanced by Congduc Pham, University of Pau, 2013-2015  */
/* updated Jan. 14th, 2025 – compiled on MacOS M2/M3 Apple Silicon      */

// #define DEBUG_CODING

#include <stdio.h>
// #include <malloc.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"
#include "mqc.h"

#define DISPLAY_PKT

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

double LuminanceJPEGTable[8][8] = {
    16.0,  11.0,  10.0,  16.0,  24.0, 40.0, 51.0,  61.0,  12.0,  12.0,  14.0,  19.0,  26.0,
    58.0,  60.0,  55.0,  14.0,  13.0, 16.0, 24.0,  40.0,  57.0,  69.0,  56.0,  14.0,  17.0,
    22.0,  29.0,  51.0,  87.0,  80.0, 62.0, 18.0,  22.0,  37.0,  56.0,  68.0,  109.0, 103.0,
    77.0,  24.0,  35.0,  55.0,  64.0, 81.0, 104.0, 113.0, 92.0,  49.0,  64.0,  78.0,  87.0,
    103.0, 121.0, 120.0, 101.0, 72.0, 92.0, 95.0,  98.0,  112.0, 100.0, 103.0, 99.0};

struct position {
    int row;
    int col;
} ZigzagCoordinates[8 * 8] =  // Matrice Zig-Zag
    {0, 0, 0, 1, 1, 0, 2, 0, 1, 1, 0, 2, 0, 3, 1, 2, 2, 1, 3, 0, 4, 0, 3, 1, 2, 2,
     1, 3, 0, 4, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1, 5, 0, 6, 0, 5, 1, 4, 2, 3, 3, 2, 4,
     1, 5, 0, 6, 0, 7, 1, 6, 2, 5, 3, 4, 4, 3, 5, 2, 6, 1, 7, 0, 7, 1, 6, 2, 5, 3,
     4, 4, 3, 5, 2, 6, 1, 7, 2, 7, 3, 6, 4, 5, 5, 4, 6, 3, 7, 2, 7, 3, 6, 4, 5, 5,
     4, 6, 3, 7, 4, 7, 5, 6, 6, 5, 7, 4, 7, 5, 6, 6, 5, 7, 6, 7, 7, 6, 7, 7};

unsigned int MSS = 64, QualityFactor = 50, packetcount = 0L;
long int count = 0L;
FILE *TRACEFILE;

/*------------------------------- Compression JPEG ----------------------------------------*/
void QTinitialization(int Quality) {
    double Qs;

    if (Quality <= 0) Quality = 1;
    if (Quality > 100) Quality = 100;
    if (Quality < 50)
        Qs = 50.0 / (double)Quality;
    else
        Qs = 2.0 - (double)Quality / 50.0;

    // Calcul des coefficients de la table de quantification
    for (int u = 0; u < 8; u++)
        for (int v = 0; v < 8; v++) {
            LuminanceJPEGTable[u][v] = LuminanceJPEGTable[u][v] * Qs;
            if (LuminanceJPEGTable[u][v] < 1.0) LuminanceJPEGTable[u][v] = 1.0;
            if (LuminanceJPEGTable[u][v] > 255.0) LuminanceJPEGTable[u][v] = 255.0;
        }

    return;
}

void JPEGencoding(BMPImageStruct *InputImage, BMPImageStruct *OutputImage) {
    double Block[8][8];
    double tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    double tmp10, tmp11, tmp12, tmp13, tmp20, tmp23;
    double tmp;

    // Encodage bloc par bloc
    for (int i = 0; i < abs(InputImage->imageVsize); i = i + 8)
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
                for (int v = 0; v < 8; v++) OutputImage->data[i + u][j + v] = Block[u][v];
        }
    return;
}

unsigned int JPEGpacketization(BMPImageStruct *InputImage, unsigned int BlockOffset) {
    int Block[8][8], row, col, row_mix, col_mix, packetsize, packetoffset, buffersize;
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
           (BlockOffset != (InputImage->imageHsize * abs(InputImage->imageVsize) / 64))) {
        // On lit le bloc
        row = (BlockOffset * 8) / InputImage->imageHsize * 8;
        col = (BlockOffset * 8) % InputImage->imageHsize;
        row_mix = ((row * 5) + (col * 8)) % (InputImage->imageHsize);
        col_mix = ((row * 8) + (col * 13)) % (abs(InputImage->imageVsize));

        // printf("(%d %d) ", row_mix, col_mix);

        for (int u = 0; u < 8; u++)
            for (int v = 0; v < 8; v++)
                Block[u][v] = (int)round(InputImage->data[row_mix + u][col_mix + v]);

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
                return (BlockOffset);
            }
        }
    }

    // On écrit le packet dans le fichier trace
    fprintf(TRACEFILE, "%.4X ", (uint16_t)packetsize + 2);
    fprintf(TRACEFILE, "%.2X ", (uint8_t)(((uint16_t)packetoffset & 0xFF00) >> 8));
    fprintf(TRACEFILE, "%.2X ", (uint8_t)((uint16_t)packetoffset & 0xFF));

#ifdef DISPLAY_PKT
    printf("%.4X (%d)", packetsize + 2, packetsize + 2);
    printf("%.2X ", (packetoffset & 0xFF00) >> 8);
    printf("%.2X ", packetoffset & 0xFF);
#endif

    for (int x = 0; x < packetsize; x++) fprintf(TRACEFILE, "%.2X ", (uint8_t)packet[x]);

#ifdef DISPLAY_PKT
    for (int x = 0; x < packetsize; x++) printf("%.2X", packet[x]);
    printf("\n");
#endif

    count += packetsize;
    packetcount++;

    return (BlockOffset);
}

/*********************************************
        Programme principal
 *********************************************/

int main(int argc, char *argv[]) {
    BMPImageStruct OriginalImage;
    double CompressionRate;
    bool Help = false;
    int err, offset;

    char originalFilename[100] = "";
    char targetFilename[100] = "";
    char newtargetFilename[100] = "";
    char shellCmd[200] = "";

    // Lecture des arguments du programme
    if (argc == 0) Help = true;

    for (int arg = 1; arg < argc; arg++) {
        if (argv[arg][0] == '-') {
            switch (toupper(argv[arg][1])) {
                case 'M':
                    if ((arg + 1) < argc) {
                        MSS = (unsigned int)atoi(argv[arg + 1]);
                        arg += 1;
                    } else {
                        printf(" JPEGencoding-ERROR: Maximal Packet Payload Size error\n\n");
                        Help = true;
                    }
                    break;
                case 'Q':
                    if ((arg + 1) < argc) {
                        QualityFactor = (unsigned int)atoi(argv[arg + 1]);
                        arg += 1;
                    } else {
                        printf(" JPEGencoding-ERROR: Quantization Scale Factor error\n\n");
                        Help = true;
                    }
                    break;
                default:
                    Help = true;
            }
        } else {
            strcpy(originalFilename, argv[arg]);
        }
    }

    if (strlen(originalFilename) == 0) Help = true;

    if (Help) {
        printf(" JPEGencoding: Illegal argument:\n\n");
        printf(" Usage: JPEGencoding [-M <MSS>] [-Q <QUALITY>] <Filename>\n");
        printf(" Flags:             -M : Maximal Packet Payload Size\n");
        printf("                    -Q : Quantization Scale Factor [1..100]\n");
        printf("                    Filename : Image file in BMP format required\n\n");
        return (1);
    }

#ifdef DEBUG_CODING
    printf("0\n");

    printf("sizeof(short): %d\n", sizeof(short));
    printf("sizeof(int): %d\n", sizeof(int));
    printf("sizeof(unsigned int): %d\n", sizeof(unsigned int));
    printf("sizeof(long): %d\n", sizeof(long));
    printf("sizeof(uint32_t): %d\n", sizeof(uint32_t));
    printf("sizeof(double): %d\n", sizeof(double));
#endif

    //  Lecture de l'image originale qui est au format BMP
    err = ReadBitmapFile(originalFilename, &OriginalImage);

    printf("%d\n", OriginalImage.imageHsize);
    printf("%d\n", OriginalImage.imageVsize);

#ifdef DEBUG_CODING
    printf("1\n");
#endif

    if (err) {
        printf("\nERR: erreur de lecture du fichier BMP.\n");
        return 1;
    }

    snprintf(targetFilename, 80, "%s.M%d-Q%d.dat", originalFilename, MSS, QualityFactor);

    //  Ouverture du fichier de trace des packets
    if ((TRACEFILE = fopen(targetFilename, "w")) == NULL) {
        printf("\n\nErreur d'ouverture du fichier: %s\n", targetFilename);
        return (2);
    }

    // On ajoute dans le fichier de trace la taille H et V
    fprintf(TRACEFILE, "%.4X ", (uint16_t)OriginalImage.imageHsize);
    fprintf(TRACEFILE, "%.4X ", (uint16_t)abs(OriginalImage.imageVsize));
    // On ajoute aussi le QualityFactor
    fprintf(TRACEFILE, "%.4X ", (uint16_t)QualityFactor);

    // Initialisation de la matrice de quantification
    QTinitialization(QualityFactor);

#ifdef DEBUG_CODING
    printf("2\n");
#endif

    // Encodage JPEG et fin
    JPEGencoding(&OriginalImage, &OriginalImage);

#ifdef DEBUG_CODING
    printf("3\n");
#endif

    offset = 0;

    do {
        offset = JPEGpacketization(&OriginalImage, offset);

    } while (offset != (OriginalImage.imageHsize * abs(OriginalImage.imageVsize) / 64));

    fclose(TRACEFILE);

    CompressionRate = (double)count * 8.0 / (OriginalImage.imageHsize * abs(OriginalImage.imageVsize));
    printf("Compression rate : %2.2f bpp\n", CompressionRate);
    printf("Packets : %d, Packets: %.4X \n", packetcount, packetcount);
    printf("Q : %d, Q: %.4X \n", QualityFactor, QualityFactor);
    printf("H : %d, H: %.4X, V : %d, V: %.4X \n", OriginalImage.imageHsize,
           OriginalImage.imageHsize, abs(OriginalImage.imageVsize), abs(OriginalImage.imageVsize));
    printf("Real encoded image file size : %ld \n", count);

    snprintf(newtargetFilename, sizeof(newtargetFilename), "%s.M%d-Q%d-P%d-S%ld.dat",
    					originalFilename, MSS, QualityFactor, packetcount, count);

    printf("Renaming in %s\n", newtargetFilename);

    snprintf(shellCmd, sizeof(shellCmd), "sed '1s/^/%.4X /' %s > %s", packetcount, targetFilename,
             newtargetFilename);

    printf("Execute shell cmd: %s\n", shellCmd);

#ifdef DEBUG_CODING
    printf("4\n");
#endif

    system(shellCmd);
    remove(targetFilename);
}
