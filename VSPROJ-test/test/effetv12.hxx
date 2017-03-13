//P.35. 2.����Function Pointers ʵ��Strategyģʽ
#include <iostream>  
using namespace std; 

class GameCharacter; //ǰ������(forward declaration)

// ���º����Ǽ��㽡��ָ����ȱʡ�㷨��
int defaultHealthCalc(const GameCharacter& gc)
{
	cout << "defaultHealthCalc" << endl;
	return 1;
}

class GameCharacter {
public:
	typedef int (*HealthCalcFunc) (const GameCharacter&);
	explicit GameCharacter(HealthCalcFunc hcf = defaultHealthCalc)
		: m_healthFunc(hcf)
	{}
	int healthValue() const
	{ return m_healthFunc(*this); }
private:
	HealthCalcFunc m_healthFunc;
};


class EvilBadGuy: public GameCharacter {
public:
	explicit EvilBadGuy(HealthCalcFunc hcf = defaultHealthCalc)
		:GameCharacter(hcf)
	{}
};

int loseHealthQuickly(const GameCharacter&) //����ָ�����㺯��l
{
	cout << "loseHealthQuickly" << endl;
	return 1;
}
int loseHealthSlowly(const GameCharacter&) //����ָ�����㺯��2
{
	cout << "loseHealthSlowly" << endl;
	return 1;
}

int main () 
{
	GameCharacter gc;
	gc.healthValue();

	EvilBadGuy ebgl(loseHealthQuickly); //��ͬ���͵��������
	EvilBadGuy ebg2(loseHealthSlowly); //��ͬ�Ľ������㷽ʽ
	ebgl.healthValue();
	ebg2.healthValue();

	return 0;
}  
