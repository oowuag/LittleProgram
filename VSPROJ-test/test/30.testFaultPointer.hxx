#include <iostream>
using namespace std;

typedef unsigned char BYTE;
typedef BYTE* PBYTE;

class AA
{
public:
	int ma;
};

int callFunctionSync(int i, PBYTE pInStr, int nInSize, PBYTE pOutStr, int nOutSize)
{
	AA *pA = reinterpret_cast<AA*>(pInStr);
	cout << pA->ma << endl;
	
	return 1;
}

int SetMatchingData(AA *psDataAdr )
{

    BYTE        *Addr = new BYTE[sizeof( int )];
    int		    error = 0;

    // GPS¥Ï¥ó¥É¥é¤ËÒªÇó
    if( 1 == callFunctionSync( 1,
                 reinterpret_cast<PBYTE>(15), sizeof( AA ),
                 reinterpret_cast<PBYTE>(Addr), sizeof( int ) ) ) {
 
		// ¥¨¥é©`Çéˆó¤ò¥³¥Ô©`
        memcpy( &error, Addr, sizeof( int ) );
    }
    else {
        error = 1;
    }
    delete []Addr;
    return ( error );

}

int main()
{
	AA* px = new AA();
	SetMatchingData(px);



	return 0;
}