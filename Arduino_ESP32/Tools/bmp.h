/************************************************************************
 *									                                                    *
 *	Procedures for use bmp files    				                            *
 *									                                                    *
 *	Author: Vincent LECUIRE, CRAN UMR 7039, Nancy-Université, CNRS	    *
 *	Date: march, 16 2012						                                    *
 *									                                                    *
 ************************************************************************/

typedef struct BMP_struct {
    char signature[2]; // BM (Bitmap windows), BA (Bitmap OS/2), CI (Icone couleur OS/2),...
    int filesize; // Taille totale du fichier BMP
    int offset;
    int headersize;
    int imageHsize;
    int imageVsize;
    int plans;
    int bpp; // nombre de bits par pixel
    int compression;
    int imagesize;
    int Hres; // résolution horizontale
    int Vres; // résolution verticale
    int colors;
    int primarycolors;
    unsigned char * palette;
    double ** data;
}
BMPImageStruct;

int hex2dec(unsigned char * hex, int n) // Conversion d'un nombre codé sur n octets (du poids fort au poids faible) en décimal
{
    int value = 0;

    for (int i = n - 1; i >= 0; i--) {
        value = value * 256 + (uint8_t) hex[i];
    }
    return value;
}

void dec2hex(int value, unsigned char * hex, int n) // Conversion d'un entier en nombre codé sur n octets (du poids fort au poids faible)
{
    for (int i = 0; i < n; i++) hex[i] = 0;

    for (int i = 0; i < n; i++) {
        hex[i] = value % 256;
        value = value / 256;
    }
}

unsigned char max(unsigned char val1, unsigned char val2) {
    if (val1 > val2)
        return val1;
    else
        return val2;
}

unsigned char min(unsigned char val1, unsigned char val2) {
    if (val1 < val2)
        return val1;
    else
        return val2;
}

double ** AllocateMemSpace(int Horizontal, int Vertical) {
    double ** Array;

    // Allocation mémoire pour un tableau 2D de grande taille

    if ((Array = (double ** ) malloc(sizeof(double * ) * Vertical)) == NULL) return NULL;

    for (int i = 0; i < Vertical; i++)
        Array[i] = (double * ) malloc(sizeof(double) * Horizontal);

    return Array;
}

int ReadBitmapFile(char * FileName, BMPImageStruct * Image) {
    unsigned char entier32[4]; //Zone mémoire temporaire où on encode les informations avant de les enregistrer
    unsigned char pixel;
    FILE * ImageFile;

    #ifdef DEBUG_CODING
    printf("ReadBitmapFile-0\n\n");

    printf("sizeof(short): %d\n", sizeof(short));
    printf("sizeof(int): %d\n", sizeof(int));
    printf("sizeof(unsigned int): %d\n", sizeof(unsigned int));
    printf("sizeof(long): %d\n", sizeof(long));
    printf("sizeof(uint32_t): %d\n", sizeof(uint32_t));
    printf("sizeof(double): %d\n", sizeof(double));
    #endif

    if ((ImageFile = fopen(FileName, "r")) == NULL) return -1;

    fread(Image -> signature, sizeof(short), 1, ImageFile); // signature codée sur 2 octets

    fread( & entier32, sizeof(uint32_t), 1, ImageFile);
    Image -> filesize = hex2dec(entier32, sizeof(uint32_t)); // taille totale du fichier, 4 octets
    printf("filesize=%d\n", Image -> filesize);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // reservé

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // offset de début de l'image, 4 octets
    Image -> offset = hex2dec(entier32, sizeof(uint32_t));
    printf("offset=%d\n", Image -> offset);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // taille de l'entete, 4 octets
    Image -> headersize = hex2dec(entier32, sizeof(uint32_t));
    printf("headersize=%d\n", Image -> headersize);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // largeur de l'image, 4 octets
    Image -> imageHsize = hex2dec(entier32, sizeof(uint32_t));
    printf("imageHsize=%d\n", Image -> imageHsize);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // hauteur de l'image, 4 octets
    Image -> imageVsize = hex2dec(entier32, sizeof(uint32_t));
    printf("imageVsize=%d\n", Image -> imageVsize);

    fread( & entier32, sizeof(short), 1, ImageFile); // nombre de plans (toujour =1), 2 octets
    Image -> plans = hex2dec(entier32, sizeof(short));
    printf("plans=%d\n", Image -> plans);

    fread( & entier32, sizeof(short), 1, ImageFile); // nombre de bits par pixel, 2 octets
    Image -> bpp = hex2dec(entier32, sizeof(short));
    printf("bpp=%d\n", Image -> bpp);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // compression (0=rien), 4 octets
    Image -> compression = hex2dec(entier32, sizeof(uint32_t));
    printf("compression=%d\n", Image -> compression);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // taille de l'image, 4 octets
    Image -> imagesize = hex2dec(entier32, sizeof(uint32_t));
    printf("imagesize=%d\n", Image -> imagesize);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // résolution horizontale en pixels par mètre, 4 octets
    Image -> Hres = hex2dec(entier32, sizeof(uint32_t));
    printf("Hres=%d\n", Image -> Hres);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // résolution verticale, en pixels par mètre, 4 octets
    Image -> Vres = hex2dec(entier32, sizeof(uint32_t));
    printf("Vres=%d\n", Image -> Vres);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // nombre de couleurs utilisées (0=toutes), 4 octets
    Image -> colors = hex2dec(entier32, sizeof(uint32_t));
    printf("colors=%d\n", Image -> colors);

    fread( & entier32, sizeof(uint32_t), 1, ImageFile); // nombre de couleurs importantes (0=toutes), 4 octets
    Image -> primarycolors = hex2dec(entier32, sizeof(uint32_t));
    printf("primarycolors=%d\n", Image -> primarycolors);

    #ifdef DEBUG_CODING
    printf("ReadBitmapFile-1\n");
    #endif

    // si c'est une image 8 bpp, cette entête est suivie de la palette.
    if ((Image -> palette = (unsigned char * ) malloc(sizeof(unsigned char) * Image -> colors * sizeof(uint32_t))) == NULL)
        return -1;

    #ifdef DEBUG_CODING
    printf("ReadBitmapFile-2\n");
    #endif

    for (int i = 0; i < Image -> colors; i++)
        fread( & Image -> palette[i * sizeof(uint32_t)], sizeof(uint32_t), 1, ImageFile);

    #ifdef DEBUG_CODING
    printf("ReadBitmapFile-3\n");
    #endif

    // après la palette, ce sont les données de l'image

    if ((Image -> data = AllocateMemSpace(Image -> imageHsize, Image -> imageVsize)) == NULL) return -1;

    for (int row = Image -> imageVsize - 1; row >= 0; row--)
        for (int col = 0; col < Image -> imageHsize; col++) {
            fread( & pixel, 1, 1, ImageFile);
            Image -> data[row][col] = pixel;
        }

    #ifdef DEBUG_CODING
    printf("ReadBitmapFile-4\n");
    #endif

    fclose(ImageFile);
    return 0;
}

int WriteBitmapFile(char * FileName, BMPImageStruct * Image, bool vflip_flag = false) {
    unsigned char entier32[4]; //Zone mémoire temporaire où on encode les informations avant de les enregistrer
    unsigned char pixel;
    FILE * ImageFile;

    if ((ImageFile = fopen(FileName, "w")) == NULL) return -1;

    fwrite(Image -> signature, sizeof(short), 1, ImageFile); // signature codée sur 2 octets

    dec2hex(Image -> filesize, entier32, sizeof(uint32_t)); // taille totale du fichier, 4 octets
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile);

    dec2hex(0, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile);

    dec2hex(Image -> offset, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // offset de début de l'image, 4 octets

    dec2hex(Image -> headersize, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // taille de l'entete, 4 octets

    dec2hex(Image -> imageHsize, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // largeur de l'image, 4 octets

    dec2hex(Image -> imageVsize, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // hauteur de l'image, 4 octets

    dec2hex(Image -> plans, entier32, sizeof(short));
    fwrite( & entier32, sizeof(short), 1, ImageFile); // nombre de plans (toujour =1), 2 octets

    dec2hex(Image -> bpp, entier32, sizeof(short));
    fwrite( & entier32, sizeof(short), 1, ImageFile); // nombre de bits par pixel, 2 octets

    dec2hex(Image -> compression, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // compression (0=rien), 4 octets

    dec2hex(Image -> imagesize, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // taille de l'image, 4 octets

    dec2hex(Image -> Hres, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // résolution horizontale en pixels par mètre, 4 octets

    dec2hex(Image -> Vres, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // résolution verticale, en pixels par mètre, 4 octets

    dec2hex(Image -> colors, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // nombre de couleurs utilisées (0=toutes), 4 octet 

    dec2hex(Image -> primarycolors, entier32, sizeof(uint32_t));
    fwrite( & entier32, sizeof(uint32_t), 1, ImageFile); // nombre de couleurs importantes (0=toutes), 4 octets

    // si c'est une image 8 bpp, cette entête est suivie de la palette.
    for (int i = 0; i < Image -> colors; i++) fwrite( & Image -> palette[i * sizeof(uint32_t)], sizeof(uint32_t), 1, ImageFile);

    // après la palette, ce sont les données de l'image
    if (vflip_flag == true) {
        // normal order of lines
        for (int row = 0; row < Image -> imageVsize; row++)
            for (int col = 0; col < Image -> imageHsize; col++) {
                pixel = (unsigned char) Image -> data[row][col];
                fwrite( & pixel, 1, 1, ImageFile);
            }
    } else {

        for (int row = Image -> imageVsize - 1; row >= 0; row--)
            // reverse order of lines
            for (int col = 0; col < Image -> imageHsize; col++) {
                pixel = (unsigned char) Image -> data[row][col];
                fwrite( & pixel, 1, 1, ImageFile);
            }
    }

    fclose(ImageFile);
    return 0;
}