//P.35. 2.藉由Function Pointers 实现Strategy模式
#include <iostream>  
using namespace std; 

class GameCharacter; //前置声明(forward declaration)

// 以下函数是计算健康指数的缺省算法。
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

int loseHealthQuickly(const GameCharacter&) //健康指数计算函数l
{
	cout << "loseHealthQuickly" << endl;
	return 1;
}
int loseHealthSlowly(const GameCharacter&) //健康指数计算函数2
{
	cout << "loseHealthSlowly" << endl;
	return 1;
}

int main () 
{
	GameCharacter gc;
	gc.healthValue();

	EvilBadGuy ebgl(loseHealthQuickly); //相同类型的人物搭配
	EvilBadGuy ebg2(loseHealthSlowly); //不同的健康计算方式
	ebgl.healthValue();
	ebg2.healthValue();

	return 0;
}  
