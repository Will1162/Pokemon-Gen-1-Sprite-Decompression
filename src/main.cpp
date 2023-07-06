#include <iostream>

int main()
{
	// import sprite.bin
	FILE *spriteFile = fopen("sprite.bin", "rb");
	if (spriteFile == NULL)
	{
		std::cout << "Error: Could not open sprite.bin" << std::endl;
		return 1;
	}

	// get size of sprite.bin
	fseek(spriteFile, 0, SEEK_END);
	int spriteFileSize = ftell(spriteFile);
	rewind(spriteFile);

	// copy sprite.bin to spriteData
	unsigned char *spriteData = new unsigned char[spriteFileSize];
	fread(spriteData, 1, spriteFileSize, spriteFile);
	fclose(spriteFile);
	
	int totalBits = spriteFileSize * 8;

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
	int primaryBuffer = (spriteData[1] & 0b10000000) >> 7;
	int packetType = (spriteData[1] & 0b01000000) >> 7;
	int encodingMode = -1;
	const char *packetName = (packetType) ? "Data" : "RLE";

	int currentBit = 10;

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
	unsigned char *bitPlane0 = new unsigned char[spriteWidth * spriteHeight * 64];
	unsigned char *bitPlane1 = new unsigned char[spriteWidth * spriteHeight * 64];
	unsigned char *bitPlaneTemp = new unsigned char[spriteWidth * spriteHeight * 64];
	int bitPlane0Index = 0;
	int bitPlane1Index = 0;

	// fill bit planes with placeholder data
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		bitPlane0[i] = 2;
		bitPlane1[i] = 2;
	}

	// start to decode first bit plane
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

	// metadata for encoding mode
	encodingMode = spriteDataBits[currentBit];
	if (encodingMode == 1)
	{
		encodingMode = packetType << 1;
		encodingMode |= spriteDataBits[currentBit + 1];
		currentBit += 2;
	}
	else
	{
		encodingMode = 1;
		currentBit++;
	}
	std::cout << "Encoding Mode: " << encodingMode << std::endl << std::endl;

	// start to decode second bit plane
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



	// transform row based bit planes to column pair based bit planes
	// bitPlane0
	int x = 0;
	int y = 0;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i+=2)
	{
		bitPlaneTemp[x + y * spriteWidth * 8] = bitPlane0[i];
		bitPlaneTemp[x + y * spriteWidth * 8 + 1] = bitPlane0[i + 1];
		y++;
		if (y == spriteHeight * 8)
		{
			y = 0;
			x += 2;
		}
	}
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		bitPlane0[i] = bitPlaneTemp[i];
	}

	// bitPlane1
	x = 0;
	y = 0;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i+=2)
	{
		bitPlaneTemp[x + y * spriteWidth * 8] = bitPlane1[i];
		bitPlaneTemp[x + y * spriteWidth * 8 + 1] = bitPlane1[i + 1];
		y++;
		if (y == spriteHeight * 8)
		{
			y = 0;
			x += 2;
		}
	}
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		bitPlane1[i] = bitPlaneTemp[i];
	}


	// delta decode bitPlane0
	int val = 0;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		if (bitPlane0[i] == 1)
		{
			val = !val;
		}
		bitPlane0[i] = val;
		// reset val to 0 every new row
		if (((i + 1) % (8 * spriteWidth)) == 0)
			val = 0;
	}
	
	// delta decode bitPlane1
	val = 0;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		if (bitPlane1[i] == 1)
		{
			val = !val;
		}
		bitPlane1[i] = val;
		// reset val to 0 every new row
		if (((i + 1) % (8 * spriteWidth)) == 0)
			val = 0;
	}

	// combine bit planes
	unsigned char *spriteDataDecoded = new unsigned char[spriteWidth * spriteHeight * 64];
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		spriteDataDecoded[i] = bitPlane0[i] | (bitPlane1[i] << 1);
	}

	// output sprite data
	std::cout << "Sprite Data Decoded: " << std::endl;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		switch (spriteDataDecoded[i])
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