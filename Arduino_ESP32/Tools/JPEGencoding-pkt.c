/************************************************************************
 *																																			*
 *	Fast DCT for image compression scheme																*
 *																																			*
 *	Author: Vincent LECUIRE, CRAN UMR 7039, Nancy-UniversitŽ, CNRS			*
 *	Date: march, 16 2012																								*
 *																																			*
 ************************************************************************/

/* packet-per-packet encoding version */

/* modified and enhanced by Congduc Pham, University of Pau, 2013-2015  */
/* updated Jan. 14th, 2025 Ð compiled on MacOS M2/M3 Apple Silicon      */

//#define DEBUG_CODING

#include <inttypes.h>
#include <stdio.h>
// #include <malloc.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"
#include "mqc.h"

#define DISPLAY_BLOCK
#define DISPLAY_FILLPKT

float CordicLoefflerScalingFactor[8]={0.35355339, 0.35355339, 0.31551713, 0.5, 0.35355339, 0.5, 0.31551713, 0.35355339};

short LuminanceJPEGTable[8][8] = {
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};

struct position { uint8_t row;	uint8_t col;
	} ZigzagCoordinates[8*8]=	// Matrice Zig-Zag
	{0, 0, 0, 1, 1, 0, 2, 0, 1, 1, 0, 2, 0, 3, 1, 2, 2, 1, 3, 0,
	 4, 0, 3, 1, 2, 2, 1, 3, 0, 4, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1,
	 5, 0, 6, 0, 5, 1, 4, 2, 3, 3, 2, 4, 1, 5, 0, 6, 0, 7, 1, 6,
	 2, 5, 3, 4, 4, 3, 5, 2, 6, 1, 7, 0, 7, 1, 6, 2, 5, 3, 4, 4,
	 3, 5, 2, 6, 1, 7, 2, 7, 3, 6, 4, 5, 5, 4, 6, 3, 7, 2, 7, 3,
	 6, 4, 5, 5, 4, 6, 3, 7, 4, 7, 5, 6, 6, 5, 7, 4, 7, 5, 6, 6,
	 5, 7, 6, 7, 7, 6, 7, 7};

char TESTIMAGE[100] = "";
unsigned int MSS = 64, QualityFactor = 50, count = 0L, packetcount = 0L;
FILE *TRACEFILE;

opj_mqc_t mqobjet, mqbckobjet, *objet=NULL;
uint8_t buffer[MQC_NUMCTXS], bckbuffer[MQC_NUMCTXS];
uint8_t packet[MQC_NUMCTXS];
int packetsize, packetoffset, buffersize;
   
/*------------------------------- Compression JPEG ----------------------------------------*/
void QTinitialization(int Quality)
{
 float Qs, scale;

 if (Quality <= 0)  Quality = 1;
 if (Quality > 100) Quality = 100;
 if (Quality < 50)   Qs = 50.0 / (float) Quality;
 	else	     Qs = 2.0 - (float) Quality/50.0;


 // Calcul des coefficients de la table de quantification
 for (int u=0; u<8; u++) {
   for (int v=0; v<8; v++)
	{
	 scale = (float)LuminanceJPEGTable[u][v] * Qs;
	 if (scale < 1.0) scale=1.0;
	 LuminanceJPEGTable[u][v] = (short)round (scale / (CordicLoefflerScalingFactor[u]*CordicLoefflerScalingFactor[v]));
	}
	}

 return;
}

void JPEGencoding(int Block[8][8])
{
 int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
 int tmp10, tmp11, tmp12, tmp13, tmp20, tmp23;
 int z11, z12, z21, z22;


    // On calcule la DCT, puis on quantifie
    for (int u=0; u<8; u++)
      {
       
       tmp0=Block[u][0]+Block[u][7];
       tmp7=Block[u][0]-Block[u][7];
       tmp1=Block[u][1]+Block[u][6];
       tmp6=Block[u][1]-Block[u][6];
       tmp2=Block[u][2]+Block[u][5];
       tmp5=Block[u][2]-Block[u][5];
       tmp3=Block[u][3]+Block[u][4];
       tmp4=Block[u][3]-Block[u][4];

       tmp10=tmp0+tmp3;
       tmp13=tmp0-tmp3;
       tmp11=tmp1+tmp2;
       tmp12=tmp1-tmp2;
       
       Block[u][0]=tmp10+tmp11;
       Block[u][4]=tmp10-tmp11;
       z11=tmp13+tmp12;	
       z12=tmp13-tmp12;
       z21=z11+(z12>>1);
       z22=z12-(z11>>1);
       Block[u][2]=z21-(z22>>4);
       Block[u][6]=z22+(z21>>4);

       z11=tmp4+(tmp7>>1);
       z12=tmp7-(tmp4>>1);
       z21=z11+(z12>>3);
       z22=z12-(z11>>3);
       z21=z21-(z21>>3);
       z22=z22-(z22>>3);
       tmp10=z21+(z21>>6);
       tmp13=z22+(z22>>6);
       z11=tmp5+(tmp6>>3);
       z12=tmp6-(tmp5>>3);
       tmp11=z11+(z12>>4);
       tmp12=z12-(z11>>4);

       tmp20=tmp10+tmp12;
       Block[u][5]=tmp10-tmp12;
       tmp23=tmp13+tmp11;
       Block[u][3]=tmp13-tmp11;
       Block[u][1]=tmp23+tmp20;
       Block[u][7]=tmp23-tmp20;
    }		

    // On attaque ensuite colonne par colonne
    for (int v=0; v<8; v++)
      {
       // 1ère étape
       tmp0= Block[0][v]+Block[7][v];
       tmp7= Block[0][v]-Block[7][v];
       tmp1= Block[1][v]+Block[6][v];
       tmp6= Block[1][v]-Block[6][v];
       tmp2= Block[2][v]+Block[5][v];
       tmp5= Block[2][v]-Block[5][v];
       tmp3= Block[3][v]+Block[4][v];
       tmp4= Block[3][v]-Block[4][v];

       tmp10=tmp0+tmp3;
       tmp13=tmp0-tmp3;
       tmp11=tmp1+tmp2;
       tmp12=tmp1-tmp2;

       Block[0][v]=tmp10+tmp11;
       Block[4][v]=tmp10-tmp11;
       z11=tmp13+tmp12;	
       z12=tmp13-tmp12;
       z21=z11+(z12>>1);
       z22=z12-(z11>>1);
       Block[2][v]=z21-(z22>>4);
       Block[6][v]=z22+(z21>>4);

       z11=tmp4+(tmp7>>1);
       z12=tmp7-(tmp4>>1);
       z21=z11+(z12>>3);
       z22=z12-(z11>>3);
       z21=z21-(z21>>3);
       z22=z22-(z22>>3);
       tmp10=z21+(z21>>6);
       tmp13=z22+(z22>>6);
       z11=tmp5+(tmp6>>3);
       z12=tmp6-(tmp5>>3);
       tmp11=z11+(z12>>4);
       tmp12=z12-(z11>>4);

       tmp20=tmp10+tmp12;
       Block[5][v]=tmp10-tmp12;
       tmp23=tmp13+tmp11;
       Block[3][v]=tmp13-tmp11;
       Block[1][v]=tmp23+tmp20;
       Block[7][v]=tmp23-tmp20;
    }

    // on centre sur l'interval [-128, 127]
    Block[0][0]-=8192;

#ifdef DISPLAY_BLOCK
    // Quantification
    printf("JPEGencoding:\n");
    for (int u=0; u<8; u++) {
       for (int v=0; v<8; v++) {
	   	Block[u][v] = (int) round((float) Block[u][v] / (float) LuminanceJPEGTable[u][v]);
	   	printf("%d\t", Block[u][v]);
	   }
	   
	   printf("\n");
	}		
#else
    for (int u=0; u<8; u++) {
       for (int v=0; v<8; v++) {
	   	Block[u][v] = (int) round((float) Block[u][v] / (float) LuminanceJPEGTable[u][v]);
	   }
#endif		
   return;
}

void CreateNewPacket(unsigned int BlockOffset)
{
   // On initialise le codeur MQ
   objet=&mqobjet;
   for (int x=0; x< MQC_NUMCTXS; x++) buffer[x]=0;
   mqc_init_enc(objet, buffer);
   mqc_resetstates(objet);
   packetoffset = BlockOffset;
   packetsize = 0;
   mqc_backup(objet, &mqbckobjet, bckbuffer);
   mqc_flush(objet);

}

void SendPacket()
{
   if (packetsize == 0) return;
   
   printf("%.4X 00 %.2X\n", packetsize + 2, packetoffset);
   // On écrit le packet dans le fichier trace
   fprintf(TRACEFILE, "%.4X ", packetsize + 2);
   fprintf(TRACEFILE, "%.2X ", (packetoffset & 0xFF00) >> 8);
   fprintf(TRACEFILE, "%.2X ",  packetoffset & 0xFF);
   for (int x=0; x<packetsize; x++) fprintf(TRACEFILE, "%.2X ", packet[x]);
   count += packetsize;
   packetcount++;

}

int FillPacket(int Block[8][8], bool *full)
{
   unsigned int index, q, r, K;

   mqc_restore(objet, &mqbckobjet, bckbuffer);

   // On cherche où se trouve le dernier coef <> 0 selon le zig-zag
   K=63;   while ((Block[ZigzagCoordinates[K].row][ZigzagCoordinates[K].col]==0) && (K>0)) K--; K++;

#ifdef DEBUG_CODING
   printf("*********->K=%d\n", K);
#endif 

   // On code la valeur de K, nombre de coefs encodé dans le bloc
   q=K / 2;	r=K % 2;

#ifdef DEBUG_CODING
   printf("*********->q=%d\n", q);
#endif 
     
   for (int x=0; x<q; x++) mqc_encode(objet, 1);
   mqc_encode(objet, 0);
   mqc_encode(objet, r);

   // On code chaque coef significatif par Golomb-Rice puis par MQ
   for(int x=0; x<K; x++)
     {
      if(Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]>=0)
	   { index=2*Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col]; }
      else { index=2*abs(Block[ZigzagCoordinates[x].row][ZigzagCoordinates[x].col])-1; }

      // Golomb
      q=index / 2;
      r=index % 2;
#ifdef DEBUG_CODING
   printf("****->q=%d\n", q);
#endif       
      for (int x=0; x<q; x++) mqc_encode(objet, 1);
      mqc_encode(objet, 0);
      mqc_encode(objet, r);
     }

   // On regarde si le paquet est plein
   mqc_backup(objet, &mqbckobjet, bckbuffer);
   mqc_flush(objet);
   buffersize=mqc_numbytes(objet);

   if (buffersize > (MSS-2)) return -1;  // ça déborde (il faut tenir compte du champ offset (2 octets) dans le paquet

   packetsize = buffersize;
   
#ifdef DISPLAY_FILLPKT

   printf("filling pkt\n");
   
   for (int x=0; x<packetsize; x++) {
    printf("%.2X ", buffer[x]);
   	packet[x]=buffer[x];
   }
   
   printf("\n");
#else
   for (int x=0; x<packetsize; x++) {
   	packet[x]=buffer[x];
   }
#endif   
   
   if (buffersize < (MSS - 6)) {  *full = false; }  else { *full = true; }
   return 0;
}


/*********************************************
	Programme principal
 *********************************************/

int main (int argc, char *argv[])
{
 	BMPImageStruct OriginalImage;
	double CompressionRate;
	bool Help    = false;
	bool RTS     = false;
	int err, offset;


	// Lecture des arguments du programme
	if (argc == 0) Help = true;

	for (int arg = 1; arg < argc; arg++) {
	      if (argv[arg][0] == '-') {
        	 switch(toupper(argv[arg][1])) {
	            case 'M':
				if ((arg+1) < argc) {
					MSS = (unsigned int)atoi(argv[arg+1]);
					arg += 1;
				} else {
					printf(" JPEGencoding-ERROR: Maximal Packet Payload Size error\n\n");
					Help = true;
				}
				break;
	            case 'Q':
				if ((arg+1) < argc) {
					QualityFactor = (unsigned int)atoi(argv[arg+1]);
					arg += 1;
				} else {
					printf(" JPEGencoding-ERROR: Quantization Scale Factor error\n\n");
					Help = true;
				}
				break;
			default : Help = true;
		 }
	      }
	      else { strcpy(TESTIMAGE, argv[arg]); }
	}

	if (strlen(TESTIMAGE) == 0) Help = true;

	if (Help){
		 printf(" JPEGencoding: Illegal argument:\n\n");
		 printf(" Usage: JPEGencoding [-M <MSS>] [-Q <QUALITY>] <Filename>\n");
		 printf(" Flags:             -M : Maximal Packet Payload Size\n");
		 printf("                    -Q : Quantization Scale Factor [1..100]\n");
		 printf("                    Filename : Image file in BMP format required\n\n");
      	 return(1);
	 }
 
	//  Lecture de l'image originale qui est au format BMP
	err = ReadBitmapFile(TESTIMAGE, &OriginalImage);
	if (err) { printf("\nERR: erreur de lecture du fichier BMP.\n"); return 1;}

	//  Ouverture du fichier de trace des packets
	if ((TRACEFILE = fopen("packets.txt", "w")) == NULL) {
		printf("\n\nErreur d'ouverture du fichier : packet.txt\n");
		return 2;
	  }

        // Initialisation de la matrice de quantification
        QTinitialization(QualityFactor);
	offset = 0;
	CreateNewPacket(offset);

	int Block[8][8];
	int row, col, row_mix, col_mix, N;
	int i, j;

	// N = 16 for 128x128
	N = OriginalImage.imageVsize / 8;

	for (row = 0; row < N; row++) {
	
		//printf("row=%d\n", row);
		
		for (col = 0; col < N; col++)
		{
			// Lecture du bloc
			row_mix = ((row * 5) + (col *  8)) % N;
			col_mix = ((row * 8) + (col * 13)) % N;

			//printf("\trow_mix*8=%d col_mix*8=%d\n", row_mix*8, col_mix*8);
		
			for (i=0; i<8; i++)
			for (j=0; j<8; j++) {
				//printf("\t\trow_mix*8+i=%d col_mix*8+j=%d\n", row_mix*8+i, col_mix*8+j);
				Block[i][j] = (int)OriginalImage.data[(row_mix * 8) + i][(col_mix * 8) + j];
			
			}	

			// Encodage JPEG du bloc 8x8
			JPEGencoding(Block);

#ifdef DISPLAY_BLOCK
			printf("main encode:\n");
			for (int u=0; u<8; u++) {
			   for (int v=0; v<8; v++) {
			   	printf("%d\t", Block[u][v]);
			   }
			   printf("\n");
			}		
#endif	

		err = FillPacket(Block, &RTS);
		if (err == -1) {
			    printf("err\n");
				SendPacket();
				CreateNewPacket(offset);
				FillPacket(Block, &RTS);
				  }
		offset ++;
		if (RTS == true) {
				SendPacket();
				CreateNewPacket(offset);
				RTS = false;
				  }
		}
	}
	
	SendPacket();
	fclose(TRACEFILE);

	CompressionRate = (double) count * 8.0 / (OriginalImage.imageHsize * OriginalImage.imageVsize);
	printf("Packet count : %d\tCompression rate : %2.2f bpp (%d octets)\n", packetcount, CompressionRate, count);	
}
