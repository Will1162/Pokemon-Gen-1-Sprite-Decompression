#include <iostream>

int main()
{
	unsigned char spriteData[] = {
		0x44, 0x37, 0x7A, 0x93, 0xAD, 0x48, 0x64, 0xEE, 0xB6, 0x4E, 0x99, 0xD0, 0xB6, 0x39, 0x92, 0x2F,
		0x48, 0xFF, 0x8C, 0x95, 0x56, 0x08, 0x9B, 0x23, 0xF4, 0xC1, 0xA2, 0xF6, 0x25, 0x88, 0x57, 0xCA,
		0x48, 0x6A, 0x90, 0x81, 0x58, 0xAF, 0xE3, 0x08, 0x21, 0x51, 0x8C, 0x54, 0x2A, 0xD3, 0x38, 0x63,
		0x17, 0x98, 0xD1, 0x50, 0x96, 0x62, 0xB9, 0x8D, 0x66, 0xFA, 0xA2, 0x55, 0xA3, 0x86, 0x6E, 0xA8,
		0x63, 0xA2, 0x64, 0xEF, 0x99, 0x8E, 0xE9, 0xD3, 0x88, 0xDD, 0xFE, 0x4E, 0xB5, 0x18, 0x24, 0xEE,
		0x90, 0x93, 0xA6, 0xA4, 0x24, 0x23, 0x99, 0x2A, 0x48, 0xAA, 0x8C, 0x95, 0x56, 0x34, 0x54, 0xC5,
		0xB6, 0xAA, 0x4B, 0x62, 0x15, 0xF2, 0x90, 0x8C, 0x69, 0x58, 0xE1, 0x89, 0x53, 0x44, 0x6A, 0x53,
		0x39, 0x08, 0xC5, 0x81, 0x8D, 0x12, 0x2D, 0x8B, 0x60, 0x63, 0x59, 0x93, 0x06, 0xF6, 0x8E, 0x19,
		0xAA, 0xA8, 0x23, 0xA2, 0x4F, 0x06, 0x4F, 0x06, 0x54, 0xE2
	};
	int totalBits = sizeof(spriteData) * 8;

	unsigned char *spriteDataBits = new unsigned char[totalBits];
	for (int i = 0; i < totalBits; i++)
	{
		spriteDataBits[i] = (spriteData[i / 8] >> (7 - (i % 8))) & 1;
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

	int spriteWidth   = (spriteData[0] & 0b11110000) >> 4;
	int spriteHeight  = (spriteData[0] & 0b00001111) >> 0;
	int primaryBuffer = (spriteData[1] & 0b10000000) >> 8;
	int packetType = (spriteData[1] & 0b01000000) >> 7;
	const char *packetName = (packetType) ? "Data" : "RLE";

	int currentBit = 10;

	std::cout << "Metadata: " << std::endl;
	std::cout << "Sprite Width: " << spriteWidth << std::endl;
	std::cout << "Sprite Height: " << spriteHeight << std::endl;
	std::cout << "Primary Buffer: " << primaryBuffer << std::endl;
	std::cout << "Initial Packet: " << packetName << std::endl;
	std::cout << "Total Bits: " << totalBits << std::endl;
	std::cout << "Total Pixels: " << spriteWidth * spriteHeight * 64 << std::endl << std::endl;
	std::cout << "Bit Plane 0 Data: " << std::endl;

	unsigned char *bitPlane0 = new unsigned char[spriteWidth * spriteHeight * 64];
	unsigned char *bitPlane1 = new unsigned char[spriteWidth * spriteHeight * 64];
	int bitPlane0Index = 0;
	int bitPlane1Index = 0;

	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		bitPlane0[i] = 2;
		bitPlane1[i] = 2;
	}

	// first bit plane
	while (bitPlane0Index < spriteWidth * spriteHeight * 64)
	{
		if (packetType == 0)
		{
			// RLE packet
			int bitsToRead = 1;

			int val1 = 0;
			while (spriteDataBits[currentBit] != 0)
			{
				val1 = val1 << 1;
				val1 |= spriteDataBits[currentBit];
				currentBit++;
				bitsToRead++;
			}
			currentBit++;
			val1 = val1 << 1;
			
			int val2 = 0;
			while (bitsToRead > 0)
			{
				val2 = val2 << 1;
				val2 |= spriteDataBits[currentBit];
				currentBit++;
				bitsToRead--;
			}

			int zeroPairCount = val1 + val2 + 1;

			for (int i = 0; i < zeroPairCount * 2; i++)
			{
				bitPlane0[i + bitPlane0Index] = 0;
			}
			bitPlane0Index += zeroPairCount * 2;
		}
		else
		{
			// Direct data packet
			// search through pairs of bits until 00 is found (end of packet)
			int currentBitPair = -1;
			while (currentBitPair != 0)
			{
				currentBitPair = spriteDataBits[currentBit] << 1;
				currentBitPair |= spriteDataBits[currentBit + 1];
				currentBit += 2;
				if (currentBitPair == 0)
					break;

				// write bit pair to bit plane
				bitPlane0[bitPlane0Index] = (currentBitPair & 0b10) >> 1;
				bitPlane0Index++;
				bitPlane0[bitPlane0Index] = (currentBitPair & 0b01) >> 0;
				bitPlane0Index++;
			}
		}
		packetType = !packetType;
	}

	// second bit plane
	packetType = spriteDataBits[currentBit];
	currentBit++;

	while (bitPlane1Index < spriteWidth * spriteHeight * 64)
	{
		if (packetType == 0)
		{
			// RLE packet
			int bitsToRead = 1;

			int val1 = 0;
			while (spriteDataBits[currentBit] != 0)
			{
				val1 = val1 << 1;
				val1 |= spriteDataBits[currentBit];
				currentBit++;
				bitsToRead++;
			}
			currentBit++;
			val1 = val1 << 1;
			
			int val2 = 0;
			while (bitsToRead > 0)
			{
				val2 = val2 << 1;
				val2 |= spriteDataBits[currentBit];
				currentBit++;
				bitsToRead--;
			}

			int zeroPairCount = val1 + val2 + 1;

			for (int i = 0; i < zeroPairCount * 2; i++)
			{
				bitPlane1[i + bitPlane1Index] = 0;
			}
			bitPlane1Index += zeroPairCount * 2;
		}
		else
		{
			// Direct data packet
			// search through pairs of bits until 00 is found (end of packet)
			int currentBitPair = -1;
			while (currentBitPair != 0)
			{
				currentBitPair = spriteDataBits[currentBit] << 1;
				currentBitPair |= spriteDataBits[currentBit + 1];
				currentBit += 2;
				if (currentBitPair == 0)
					break;

				// write bit pair to bit plane
				bitPlane1[bitPlane1Index] = (currentBitPair & 0b10) >> 1;
				bitPlane1Index++;
				bitPlane1[bitPlane1Index] = (currentBitPair & 0b01) >> 0;
				bitPlane1Index++;
			}
		}
		packetType = !packetType;
	}




	// output bit plane
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		if (bitPlane0[i] == 2)
			std::cout << "_ ";
		else
			std::cout << (int)bitPlane0[i] << " ";
		if (((i + 1) % (8 * spriteWidth)) == 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;

	std::cout << "Bit Plane 1 Data: " << std::endl;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		if (bitPlane1[i] == 2)
			std::cout << "_ ";
		else
			std::cout << (int)bitPlane1[i] << " ";
		if (((i + 1) % (8 * spriteWidth)) == 0)
			std::cout << std::endl;
	}
	

	// std::cout << "Raw Data: " << std::endl;
	// for (int i = 0; i < totalBits; i++)
	// {
	// 	std::cout << (int)spriteDataBits[i];
	// 	if (((i + 1) % 8) == 0)
	// 		std::cout << " ";
	// 	if (((i + 1) % (8 * spriteWidth)) == 0)
	// 		std::cout << std::endl;
	// }
	// std::cout << std::endl;


	return 0;
}