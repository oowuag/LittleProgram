#include "PcmWaveOut.h"

#include <fstream>
using namespace std;

int main()
{
	char str[] = "D:\\MyDocument\\TestMusic.wav";

	ifstream infile(str, ios::binary|ios::ate);
	if (!infile.good())
	{
		return -1;
	}
	int nSize = infile.tellg();
	infile.seekg(0, ios::beg);
	char *buffer = new char[nSize];
	infile.read((char*)buffer, nSize);

	CPcmWaveOut playwav;
	playwav.Play(buffer, nSize);

	while (playwav.IsPlaying())
	{	
	}


	return 0;
}