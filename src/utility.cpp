#include "utility.h"

int GetFileSizeBytes(FILE *file)
{
	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	rewind(file);
	return fileSize;
}

void CopyFileToSpriteDataBytes(FILE *file, unsigned char *spriteData)
{
	int fileSize = GetFileSizeBytes(file);
	fread(spriteData, 1, fileSize, file);
	fclose(file);
}

void DecodeBitPlane(unsigned char *bitPlane, unsigned char *spriteDataBits, int spriteWidth, int spriteHeight, int &bitPlaneIndex, int packetType, int &currentBit)
{
	// take compressed sprite data and decode it into a bit plane depending on packet type

	// while there are still bits to read for the bit plane
	while (bitPlaneIndex < spriteWidth * spriteHeight * 64)
	{
		if (packetType == 0)
		{
			/*
				RLE packet

				This is hard to explain in words, so see 11:10 to 15:58 in the
				referenced video (Pokémon Sprite Decompression Explained) for a visual explanation.
			*/
		
			int bitsToRead = 1;

			// calculate L
			int L = 0;
			while (spriteDataBits[currentBit] != 0)
			{
				L = L << 1;
				L |= spriteDataBits[currentBit++];
				bitsToRead++;
			}
			currentBit++;
			L = L << 1;
			
			// calculate V
			int V = 0;
			while (bitsToRead > 0)
			{
				V = V << 1;
				V |= spriteDataBits[currentBit++];
				bitsToRead--;
			}

			// write L + V + 1 zero pairs to bit plane
			int zerosToWrite = (L + V + 1) * 2;
			for (int i = 0; i < zerosToWrite; i++)
			{
				bitPlane[bitPlaneIndex++] = 0;
			}
		}
		else
		{
			// Direct data packet
			// search through pairs of bits, copying them to the bit plane until 00 is found (end of packet)
			int currentBitPair = -1;
			while (currentBitPair != 0)
			{
				currentBitPair = spriteDataBits[currentBit++] << 1;
				currentBitPair |= spriteDataBits[currentBit++];
				if (currentBitPair == 0)
					break;

				// write bit pair to bit plane
				bitPlane[bitPlaneIndex++] = (currentBitPair & 0b10) >> 1;
				bitPlane[bitPlaneIndex++] = (currentBitPair & 0b01) >> 0;
			}
		}
		packetType = !packetType;
	}
}

void TransformBitPlaneIndexOrder(unsigned char *bitPlane, unsigned char *bitPlaneTemp, int spriteWidth, int spriteHeight)
{
	/*
		transform row based bit planes to column pair based bit planes

		Example:
		Row based bit plane:
		0 1 2 3
		4 5 6 7
		8 9 A B
		C D E F

		Column pair based bit plane:
		0 1 8 9
		2 3 A B
		4 5 C D
		6 7 E F
	*/

	int x = 0;
	int y = 0;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i+=2)
	{
		bitPlaneTemp[x + y * spriteWidth * 8] = bitPlane[i];
		bitPlaneTemp[x + y * spriteWidth * 8 + 1] = bitPlane[i + 1];
		y++;
		if (y == spriteHeight * 8)
		{
			y = 0;
			x += 2;
		}
	}
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		bitPlane[i] = bitPlaneTemp[i];
	}
}

int GetEncodingMode(unsigned char *spriteDataBits, int &currentBit)
{
	// encoding mode can be 1 or 2 bits long, 0, 10, or 11

	int encodingMode = -1;

	encodingMode = spriteDataBits[currentBit++];
	if (encodingMode == 1)
	{
		encodingMode = encodingMode << 1;
		encodingMode |= spriteDataBits[currentBit++];
	}
	else
	{
        // change encoding mode 0 to 1, to make encoding modes 1, 2 and 3 instead of 0, 2 and 3, which makes more sense
		encodingMode = 1;
	}

	return encodingMode;
}

void DeltaDecodeBitPlane(unsigned char *bitPlane, int spriteWidth, int spriteHeight)
{
	int val = 0;
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		if (bitPlane[i] == 1)
		{
			val = !val;
		}
		bitPlane[i] = val;
		// reset val to 0 every new row
		if (((i + 1) % (8 * spriteWidth)) == 0)
			val = 0;
	}
}

void XorBitPlanes(unsigned char *bitPlane0, unsigned char *bitPlane1, int spriteWidth, int spriteHeight)
{
	for (int i = 0; i < spriteWidth * spriteHeight * 64; i++)
	{
		bitPlane1[i] = bitPlane0[i] ^ bitPlane1[i];
	}
}