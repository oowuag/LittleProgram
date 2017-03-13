//P.40

#include <iostream>
using namespace std;

class Borrowableltem {
public:
	void checkOut( ){}
};

class ElectronicGadget {
private:
	bool checkOut( ) const {}
};

class MP3Player : public Borrowableltem, public ElectronicGadget
{
};

int main()
{
	MP3Player mp;

	//mp.checkOut( ); //ambiguous access of 'checkOut'
	mp.Borrowableltem::checkOut(); //必须明白指出你要调用哪一个base class 内的函数z

	return 0;

}