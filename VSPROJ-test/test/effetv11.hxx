//P.35. 1.藉由Non-Virtual Interface(NVI)手法实现Template Method模式
#include <iostream>  
using namespace std; 

class GameCharacter {
public:
	int healthValue() const //我把这个non-virtual函数(healthValue) 称为virtual函数的外覆器(wrapper)
	{
		int retVal = doHealthValue();
		return retVal;
	}
private:
	virtual int doHealthValue() const
	{
		cout << "GameCharacter doHealthValue" << endl;
		return 1;
	}
};

class GamePlayer1 : public GameCharacter {

private:
	virtual int doHealthValue() const
	{
		cout << "GamePlayer1 doHealthValue" << endl;
		return 1;
	}
};

class GamePlayer2 : public GameCharacter {

};


int main () 
{
	GameCharacter gc;
	gc.healthValue();

	GamePlayer1 gp;
	gp.healthValue();

	GamePlayer2 gp2;
	gp2.healthValue();
	
	return 0;
}  
