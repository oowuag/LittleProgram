//P.35. 1.����Non-Virtual Interface(NVI)�ַ�ʵ��Template Methodģʽ
#include <iostream>  
using namespace std; 

class GameCharacter {
public:
	int healthValue() const //�Ұ����non-virtual����(healthValue) ��Ϊvirtual�������⸲��(wrapper)
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
