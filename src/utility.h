#include <iostream>

int GetFileSizeBytes(FILE *file);
void CopyFileToSpriteDataBytes(FILE *file, unsigned char *spriteData);
void DecodeBitPlane(unsigned char *bitPlane, unsigned char *spriteDataBits, int spriteWidth, int spriteHeight, int &bitPlaneIndex, int packetType, int &currentBit);
void TransformBitPlaneIndexOrder(unsigned char *bitPlane, unsigned char *bitPlaneTemp, int spriteWidth, int spriteHeight);
int GetEncodingMode(unsigned char *spriteDataBits, int &currentBit);
void DeltaDecodeBitPlane(unsigned char *bitPlane, int spriteWidth, int spriteHeight);
void XorBitPlanes(unsigned char *bitPlane0, unsigned char *bitPlane1, int spriteWidth, int spriteHeight);