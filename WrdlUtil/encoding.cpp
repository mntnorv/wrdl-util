#include "encoding.h"

//--------------------------------------------------

long long encodeWord (std::string word)
{
	long long encoded = 0;
	long long shift = 0;
	long long shiftIncrement = 5;
	long long length = word.length();

	for (unsigned int i = 0; i < word.length(); i++)
	{
		encoded |= (long long)(word[i] - 64) << shift;
		shift += shiftIncrement;
	}

	encoded |= length << (long long)(60);

	return encoded;
}

//--------------------------------------------------

std::string decodeWord (long long encoded)
{
	std::string decoded = "";
	long long mask = 31;
	long long shift = 0;
	long long shiftIncrement = 5;
	long long currentChar;
	long long length;
	bool error = false;

	length = (encoded >> 60) & 0xF;

	while (decoded.length() < length)
	{
		currentChar = (encoded >> shift) & mask;
		decoded += (char)(currentChar + 64);
		shift += shiftIncrement;

		if (currentChar > 26)
			error = true;
	}

	if (error)
		std::cout << "ERROR: " << decoded << "   " << encoded << "\n";

	return decoded;
}