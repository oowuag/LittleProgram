#include <iostream>
using namespace std;

class A
{
public:
	enum {
    NHANDLER_STUB_GPS_FUNC_ID_START = 0x00000500 ///< ¥Õ¥¡¥ó¥¯¥·¥ç¥óIDé_Ê¼
	};
};

int foo(int para)
{
	para = 3;
	return 0;
}

int main()
{
	cout << A::NHANDLER_STUB_GPS_FUNC_ID_START << endl;

	int *px = new int(5);
	foo(*px);

	return 0;
}