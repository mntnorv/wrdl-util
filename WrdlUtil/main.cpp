#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <functional>
#include <Windows.h>

#include "encoding.h"

using namespace std;

//--------------------------------------------------

const int ALPHABET_SIZE = 26;

//--------------------------------------------------

struct letter
{
	vector <letter*> bordering;
	char character;
	bool used;

	letter (char character):character(character) { used = false; };
};

//--------------------------------------------------

double PCFreq = 0.0;
__int64 CounterStart = 0;

//--------------------------------------------------

void printHelp();
bool compLongs (long long first, long long second);
int parseArgs(int argc, char* argv[]);

void encodeDict (string plainFile, string encodedFile);
void testDict (string fileName);
void printFile (string fileName);
void findWords (string fileName, string letters);

void wordSearch (vector <letter *> grid, vector<long long> dict, string word, vector <string> & words, int maxLength);

vector<long long> loadDictionary (string fileName);
void readDictionary(string fileName, function<void (long long)> action);

vector <letter*> createGrid (string letters);
void destroyGrid (vector <letter *> grid);

template<class Iter, class T>
Iter binary_find(Iter begin, Iter end, T val);

void StartCounter();
double GetCounter();

//--------------------------------------------------

int main (int argc, char* argv[])
{
	switch (parseArgs (argc, argv))
	{
	case 0:
		printHelp();
		break;
	case 1:
		encodeDict (argv[2], argv[3]);
		break;
	case 2:
		testDict (argv[2]);
		break;
	case 3:
		findWords (argv[2], argv[3]);
		break;
	case 4:
		printFile (argv[2]);
		break;
	default:
		cout << "Encountered an error. Stopping.\n";
		break;
	}

	return 0;
}

//--------------------------------------------------

void printHelp()
{
	cout << "WRDLUTIL, a WRDL utility\n";
	cout << "by Mantas Norvaisa, 2012\n\n";
	cout << "Usage:\n";
	cout << "/enc [file1] [file2]    Encode a plain text dictionary for WRDL\n\n";
	cout << "/test [dict]            Test an encoded dictionary. Outputs words that contain\n";
	cout << "                        non-english characters.\n\n";
	cout << "/find [dict] [letters]  Find all words in a letter grid.\n\n";
	cout << "/print [file]           Prints all words from encoded dicitonary.\n";
}

//--------------------------------------------------

bool compLongs (long long first, long long second)
{
	long long mask = 31;
	long long shift = 0;
	long long shiftIncrement = 5;
	long long fChar = 0, sChar = 0;

	while (shift <= 64)
	{
		fChar = (first >> shift) & mask;
		sChar = (second >> shift) & mask;

		if (fChar != sChar)
			return fChar < sChar;

		shift += shiftIncrement;
	}

	return fChar < sChar;
}

//--------------------------------------------------

int parseArgs(int argc, char* argv[])
{
	if (argc > 1)
	{
		if (strcmp(argv[1], "/enc") == 0)
		{
			if (argc == 4)
			{
				ifstream test (argv[2]);
				if (test)
				{
					test.close();

					ofstream test2 (argv[3]);

					if (test2)
					{
						test2.close();

						return 1;
					}
					else
					{
						cout << "ERROR: " << argv[3] << " is not a valid filename.\n";

						return -1;
					}
				}
				else
				{
					cout << "ERROR: file " << argv[2] << " doesn't exist.\n";
					return -1;
				}
			}
			else
				return 0;
		}
		else if (strcmp (argv[1], "/test") == 0)
		{
			if (argc == 3)
			{
				ifstream test1 (argv[2]);
				if (test1)
				{
					test1.close();

					return 2;
				}
				else
				{
					cout << "ERROR: file " << argv[2] << " doesn't exist.\n";

					return -1;
				}
			}
			else
				return 0;
		}
		else if (strcmp (argv[1], "/find") == 0)
		{
			if (argc == 4)
			{
				ifstream test (argv[2]);
				if (test)
				{
					test.close();

					return 3;
				}
				else
				{
					cout << "ERROR: file " << argv[2] << " doesn't exist.\n";

					return -1;
				}
			}
			else
				return 0;
		}
		else if (strcmp (argv[1], "/print") == 0)
		{
			if (argc == 3)
			{
				ifstream test (argv[2]);
				if (test)
				{
					test.close();

					return 4;
				}
				else
				{
					cout << "ERROR: file " << argv[2] << " doesn't exist.\n";

					return -1;
				}
			}
			else
				return 0;
		}
		else
			return 0;
	}
	else
		return 0;

	return -1;
}

//--------------------------------------------------

void encodeDict (string plainFile, string encodedFile)
{
	// Read all words from "dict.lst"
	// Store them encoded in a vector
	vector <long long> words;
	words.reserve(250000);

	ifstream dict (plainFile.c_str());
	
	while (!dict.eof())
	{
		string line;
		getline (dict, line);

		if (!dict.fail())
		{
			if (line.length() <= 12)
				words.push_back(encodeWord(line));
		}
	}

	dict.close();

	//----------------------------------------

	// Sort encoded words by encoded value

	sort (words.begin(), words.end(), compLongs);

	//----------------------------------------

	// Write encoded words to a binary file ("dict.hex")
	// Every word ends with (char)0

	ofstream encodedDict (encodedFile.c_str(), ios::binary);
	
	//encodedDict.put ((unsigned char)0xFF);

	for (vector<long long>::iterator i = words.begin(); i != words.end(); i++)
	{
		long long encoded = *i;
		long long mask = 255;
		long long shift = 0;
		long long shiftIncrement = 8;
		long long currentChar;

		decodeWord (encoded);

		while (((currentChar = (encoded >> shift)) != 0) && (shift <= 64))
		{
			currentChar &= mask;
			
			if (currentChar == 0)
				encodedDict.put((unsigned char)0xFF);
			
			encodedDict.put((unsigned char)currentChar);
			shift += shiftIncrement;
		}

		encodedDict << (unsigned char)0;
	}

	encodedDict.close();
}

//--------------------------------------------------

void testDict (string fileName)
{
	readDictionary(fileName, [](long long word){decodeWord(word);});
}

//--------------------------------------------------

vector<long long> loadDictionary (string fileName)
{
	vector<long long> dictionary;
	dictionary.reserve(250000);
	readDictionary(fileName, [&dictionary](long long word){dictionary.push_back(word);});
	dictionary.shrink_to_fit();
	return dictionary;
}

//--------------------------------------------------

void findWords (string fileName, string letters)
{
	vector<long long> longDict = loadDictionary(fileName);

	vector<string> words;
	vector<letter *> grid = createGrid(letters);

	StartCounter();
	wordSearch(grid, longDict, "", words, 8);
	cout << GetCounter() << "\n";

	// Deduplicate
	sort(words.begin(), words.end());
	words.erase(unique(words.begin(), words.end()), words.end());

	cout << "Found " << words.size() << " words:\n";

	for (vector<string>::iterator iter = words.begin(); iter != words.end(); iter++)
		cout << (*iter) << endl;

	destroyGrid(grid);
}

//--------------------------------------------------

void wordSearch (vector <letter *> grid, vector<long long> dict, string word, vector <string> & words, int maxLength)
{
	if (word.length() <= maxLength)
	{
		for (vector<letter *>::iterator iter = grid.begin(); iter != grid.end(); iter++)
		{
			if (!(*iter)->used)
			{
				string currentWord = word + (*iter)->character;
				vector<long long>::iterator result = lower_bound(dict.begin(), dict.end(), encodeWord(currentWord), compLongs);

				if (result != dict.end())
				{
					string resultString = decodeWord(*result);

					if (resultString == currentWord)
						words.push_back(currentWord);

					if (resultString.length() >= currentWord.length())
					{
						if (resultString.substr(0, currentWord.length()) == currentWord)
						{
							(*iter)->used = true;
							wordSearch((*iter)->bordering, dict, currentWord, words, maxLength);
							(*iter)->used = false;
						}
					}
				}
			}
		}
	}
}

//--------------------------------------------------

void readDictionary(string fileName, function<void (long long)> action)
{
	ifstream dictFile;

	dictFile.open (fileName.c_str(), ios::binary);

	long long shift = 0;
	long long shiftIncrement = 8;
	long long current = 0;

	while (!dictFile.eof())
	{
		char readch;
		dictFile.get (readch);
		unsigned char read = (unsigned char) readch;

		if (!dictFile.fail())
		{
			if (read != 0)
			{
				if (read == 0xFF && dictFile.peek() == 0)
				{
					dictFile.get (readch);
					read = 0;
				}
					
				current |= ((long long)read) << shift;
				shift += shiftIncrement;
			}
			else
			{
				action(current);
				shift = 0;
				current = 0;
			}
		}
	}

	dictFile.close();
}

//--------------------------------------------------

void printFile (string fileName)
{
	readDictionary(fileName, [](long long word){cout << decodeWord(word) << endl;});
}

//--------------------------------------------------

vector <letter*> createGrid (string letters)
{
	int directions[] = {
		-4, // UP
		 4, // DOWN
		-1, // LEFT
		 1, // RIGHT
		-5, // UP LEFT
		-3, // UP RIGHT
		 3, // DOWN LEFT
		 5};// DOWN RIGHT

	vector <letter *> grid;

	for (int i = 0; i < 16; i++)
		grid.push_back (new letter(letters[i]));

	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			int row = i / 4;
			int col = i % 4;

			if (col == 0 && (j == 2 || j == 4 || j == 6))
				continue;
			if (col == 3 && (j == 3 || j == 5 || j == 7))
				continue;
			if (row == 0 && (j == 0 || j == 4 || j == 5))
				continue;
			if (row == 3 && (j == 1 || j == 6 || j == 7))
				continue;

			grid[i]->bordering.push_back (grid[i+directions[j]]);
		}
	}

	return grid;
}

//--------------------------------------------------

void destroyGrid (vector <letter *> grid)
{
	for (vector<letter *>::iterator it = grid.begin(); it != grid.end(); it++)
		delete (*it);
}

//--------------------------------------------------

template<class Iter, class T>
Iter binary_find(Iter begin, Iter end, T val)
{
    // Finds the lower bound in at most log(last - first) + 1 comparisons
    Iter i = lower_bound(begin, end, val);

    if (i != end && *i == val)
        return i; // found
    else
        return end; // not found
}

//--------------------------------------------------

void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
        cout << "QueryPerformanceFrequency failed!\n";

    PCFreq = double(li.QuadPart)/1.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

//--------------------------------------------------

double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}