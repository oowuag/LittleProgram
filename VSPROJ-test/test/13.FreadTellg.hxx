// crt_fread.c
// This program opens a file named FREAD.OUT and
// writes 25 characters to the file. It then tries to open
// FREAD.OUT and read in 25 characters. If the attempt succeeds,
// the program displays the number of actual items read.

#include <stdio.h>
#include <fstream>
using namespace std;

int main( void )
{

	ifstream inFile("C:\\idlist.txt", ios::binary|ios::ate);
	int nSize = inFile.tellg();
	inFile.close();

	FILE *stream;
	char* list = new char[nSize];
	int  i, numread, numwritten;

	if( fopen_s( &stream, "C:\\idlist.txt", "r+t" ) == 0 )
	{
		numread = fread( list, sizeof( char ), nSize, stream ); // 跟nSize大小不同
		printf( "Number of items read = %d\n", numread );
		printf( "Contents of buffer = %s\n", list );
		fclose( stream );
	}
	else
		printf( "File could not be opened\n" );

	delete[] list;
}
