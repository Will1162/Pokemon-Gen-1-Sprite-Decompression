#include "utility.h"

int main()
{
	// import sprite.bin and copy to spriteData
	const char* spriteFileName = "sprite.bin";
	const int spriteFileSize = GetFileSizeBytes(fopen(spriteFileName, "rb"));
	
	unsigned char *spriteDataBytes = new unsigned char[spriteFileSize];
	CopyFileToSpriteDataBytes(fopen(spriteFileName, "rb"), spriteDataBytes);
	
	int totalBits = spriteFileSize * 8;
	unsigned char *spriteDataBits = new unsigned char[totalBits];
	for (int i = 0; i < totalBits; i++)
	{
		spriteDataBits[i] = (spriteDataBytes[i / 8] >> (7 - (i % 8))) & 1;
	}

	/*
		metadata
		-      4 bits: sprite width
		-      4 bits: sprite height
		-      1 bit : primary buffer flag
		-      1 bit : initial packet type
		-      x bits: compressed sprite data
		- 1 or 2 bits: encoding mode
		-      1 bit : initial packet type
		-      x bits: compressed sprite data
	*/

	int spriteWidth   = (spriteDataBytes[0] & 0b11110000) >> 4;
	int spriteHeight  = (spriteDataBytes[0] & 0b00001111) >> 0;
	int primaryBuffer = (spriteDataBytes[1] & 0b10000000) >> 7;
	int packetType    = (spriteDataBytes[1] & 0b01000000) >> 7;
	int encodingMode  = -1; // is found after decoding first bit plane
	int currentBit    = 10;
	const char *packetName = (packetType) ? "Data" : "RLE";

	// output metadata
	std::cout << "Metadata: " << std::endl;
	std::cout << "Sprite Width: " << spriteWidth << std::endl;
	std::cout << "Sprite Height: " << spriteHeight << std::endl;
	std::cout << "Primary Buffer: " << primaryBuffer << std::endl;
	std::cout << "Initial Packet: " << packetName << std::endl;
	std::cout << "Input Bits: " << totalBits << std::endl;
	std::cout << "Output Bits: " << spriteWidth * spriteHeight * 64 * 2 << std::endl;
	std::cout << "Compression Ratio: " << (float)(spriteWidth * spriteHeight * 64 * 2) / (float)totalBits << std::endl;

	// define bit planes
	unsigned char *bitPlane0    = new unsigned char[spriteWidth * spriteHeight * 64];
	unsigned char *bitPlane1    = new unsigned char[spriteWidth * spriteHeight * 64];
	unsigned char *bitPlaneTemp = new unsigned char[spriteWidth * spriteHeight * 64];
	int bitPlane0Index = 0;
	int bitPlane1Index = 0;

	// decode the first bit plane
	DecodeBitPlane(bitPlane0, spriteDataBits, spriteWidth, spriteHeight, bitPlane0Index, packetType, currentBit);

	// metadata for encoding mode
	encodingMode = GetEncodingMode(spriteDataBits, currentBit);
	std::cout << "Encoding Mode: " << encodingMode << std::endl << std::endl;

	// decode the second bit plane
	packetType = spriteDataBits[currentBit++];
	DecodeBitPlane(bitPlane1, spriteDataBits, spriteWidth, spriteHeight, bitPlane1Index, packetType, currentBit);

	// transform row based bit planes to column pair based bit planes
	TransformBitPlaneIndexOrder(bitPlane0, bitPlaneTemp, spriteWidth, spriteHeight);
	TransformBitPlaneIndexOrder(bitPlane1, bitPlaneTemp, spriteWidth, spriteHeight);


	// merge bit planes depending on encoding mode
	switch (encodingMode)
	{
		case 1:
		{
			DeltaDecodeBitPlane(bitPlane0, spriteWidth, spriteHeight);
			DeltaDecodeBitPlane(bitPlane1, spriteWidth, spriteHeight);
			
			break;
		}
		case 2:
		{
			DeltaDecodeBitPlane(bitPlane0, spriteWidth, spriteHeight);
			XorBitPlanes(bitPlane0, bitPlane1, spriteWidth, spriteHeight);

			break;
		}
		case 3:
		{
			DeltaDecodeBitPlane(bitPlane0, spriteWidth, spriteHeight);
			DeltaDecodeBitPlane(bitPlane1, spriteWidth, spriteHeight);
			XorBitPlanes(bitPlane0, bitPlane1, spriteWidth, spriteHeight);

			break;
		}
	}

	// combine bit planes, depending on primary buffer flag
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		bitPlaneTemp[i] = bitPlane0[i] << primaryBuffer | (bitPlane1[i] << !primaryBuffer);
	}

	// output sprite data using 2 characters to make the sprite more visible/square
	std::cout << "Sprite Data Decoded: " << std::endl;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		switch (bitPlaneTemp[i])
		{
			case 0b00:
				std::cout << "$$";
				break;
			case 0b01:
				std::cout << ";;";
				break;
			case 0b10:
				std::cout << "..";
				break;
			case 0b11:
				std::cout << "  ";
				break;
		}

		if (((i + 1) % (8 * spriteWidth)) == 0)
			std::cout << std::endl;	
	}

	return 0;
}