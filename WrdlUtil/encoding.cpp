#include "encoding.h"

//--------------------------------------------------

long long encodeWord (std::string word)
{
	long long encoded = 0;
	long long shift = 0;
	long long shiftIncrement = 5;

	for (int i = 0; i < word.length(); i++)
	{
		encoded |= (long long)(word[i] - 64) << shift;
		shift += shiftIncrement;
	}

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
	bool error = false;

	while ((currentChar = ((encoded >> shift) & mask)) != 0)
	{
		decoded += (char)(currentChar + 64);
		shift += shiftIncrement;

		if (currentChar > 26)
			error = true;
	}

	if (error)
		std::cout << "ERROR: " << decoded << "   " << encoded << "\n";

	return decoded;
}