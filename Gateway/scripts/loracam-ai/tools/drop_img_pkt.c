/*
 *  Fake Sending an image encoded with CRAN encoding scheme
 *
 *  Copyright (C) 2025 Congduc Pham
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

using namespace std;

int nbPktSent = 0;
int nbDroppedPkt = 0;

int qualityFactor = 50;

bool pktDisplay = false;
// do not write a pkt list file if set to true with -npktf
bool pktFile = false;
// fake sending, just write into a file, same format than input file, useful with drop mode
bool fakeSend = false;
// indicate drop mode, usefull with fake send mode to write a partially truncated file
bool dropPkt = false;
// the pkt drop percentage
unsigned int dropPercentage = 0;

void send_img(char* file_to_send) {
    FILE* pFile;
    FILE* dFile;
    int res;
    char targetFilename[110] = "";
    char newtargetFilename[110] = "";
    char shellCmd[200] = "";
    unsigned int final_dropPercentage;
		
    uint8_t ImageData[260];

    pFile = fopen(file_to_send, "r");

    if (pFile) {
        int chunkSize;
        uint8_t pktSN = 0;
        unsigned int storedNbPackets, imageSizeX, imageSizeY, qualityFactor;

        // file to write the sent packet
        if (pktFile) {
            snprintf(targetFilename, sizeof(targetFilename), "%s-DP%d", file_to_send, dropPercentage);
            printf("Writing to %s\n", targetFilename);
            dFile = fopen(targetFilename, "w");
        }

        fscanf(pFile, "%04X", &storedNbPackets);
        fscanf(pFile, "%04X", &imageSizeX);
        fscanf(pFile, "%04X", &imageSizeY);
        fscanf(pFile, "%04X", &qualityFactor);

        fprintf(dFile, "%04X ", storedNbPackets);
        fprintf(dFile, "%04X ", imageSizeX);
        fprintf(dFile, "%04X ", imageSizeY);
        fprintf(dFile, "%04X ", qualityFactor);

        int j;

        do {
            bool dropThisPkt = false;

            // get the size of the packet
            res = fscanf(pFile, "%04X", &chunkSize);

            if (res == EOF) continue;

            if (dropPkt && (rand() % 100 < dropPercentage)) {
                dropThisPkt = true;
                nbDroppedPkt++;
            }

            fprintf(stderr, "\npkt %d: %04X ", pktSN, chunkSize);

            if (pktFile && !dropThisPkt) fprintf(dFile, "%04X ", chunkSize);

            for (j = 0; j < chunkSize; j++) {
                res = fscanf(pFile, "%2X", &ImageData[j]);

                if (pktDisplay) fprintf(stderr, "%02X", ImageData[j]);

                if (pktFile && !dropThisPkt) fprintf(dFile, "%02X ", ImageData[j]);

                if (((j == 0 || j == 1) && pktFile && !dropThisPkt && !fakeSend))
                    fprintf(dFile, " ");
            }

            if (pktFile && !dropThisPkt && !fakeSend) fprintf(dFile, "\n");

            if (dropThisPkt) {
                fprintf(stderr, "\nDROPPED\n");
            } else {
                fprintf(stderr, "\nFake send\n");
            }

            pktSN++;
            nbPktSent++;

        } while (res != EOF);
    }

    if (pktFile) fclose(dFile);

    fprintf(stderr, "sent pkt: %d | dropped: %d | dropped/sent ratio: %.2f\n", nbPktSent,
            nbDroppedPkt, (float)(nbDroppedPkt) / (float)nbPktSent);
            
    final_dropPercentage = (int) (( (float)nbDroppedPkt / (float)nbPktSent ) * 100);
		
		printf("Renaming in %s-%d-P%d.dat\n", targetFilename, final_dropPercentage,
						nbPktSent-nbDroppedPkt);

    snprintf(shellCmd, sizeof(shellCmd), "mv %s %s-%d-P%d.dat", targetFilename, targetFilename,
             final_dropPercentage, nbPktSent-nbDroppedPkt);

    printf("Execute shell cmd: %s\n", shellCmd);

    system(shellCmd);    
            
}

void* printERROR(char* argv[]) {
    fprintf(stderr, "USAGE:\t%s -drop d -pktf img_file\n", argv[0]);
    fprintf(stderr, "USAGE:\t-fake, emulate sending\n");
    fprintf(stderr, "USAGE:\t-drop 50, will introduce 50% of packet drop. Useful with -fake\n");
    fprintf(stderr, "USAGE:\t-pktf, generate a pkt list file\n");
    fprintf(stderr, "USAGE:\t img_file, image .dat file with CRAN encoding\n");

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printERROR(argv);
        exit(-1);
    }

    char* file_to_send;

    fakeSend = true;
    pktDisplay = true;
    pktFile = true;

    for (int i = 1; i < argc - 1; i++) {
        if (!strcmp(argv[i], "-pktf")) {
            pktFile = true;
        }

        if (!strcmp(argv[i], "-drop")) {
            dropPercentage = atoi(argv[i + 1]);
            dropPkt = true;
        }
    }

    srand(time(NULL));

    file_to_send = argv[argc - 1];

    fprintf(stderr, "Preparing to send file %s\n", file_to_send);

    send_img(file_to_send);

    exit(0);
}
