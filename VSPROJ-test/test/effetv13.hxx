//P.35. 3.藉由trl::function 完成Strategy模式
#include <iostream>
#include "boost/function.hpp"
#include "boost/bind.hpp"
using namespace std;

//boost::function
class GameCharacter;//如前
int defaultHealthCalc(const GameCharacter& gc) //如前
{
	cout << "defaultHealthCalc" << endl;
	return 1;
}

class GameCharacter {
public:
	//HealthCalcFunc可以是任何"可调用物" (callable entity) ，可被调用并接受
	//任何兼容于GameCharacter 之物，返回任何兼容于int 的东西。详下。
	typedef boost::function<int (const GameCharacter&)> HealthCalcFunc;
	explicit GameCharacter(HealthCalcFunc hcf = defaultHealthCalc)
		:healthFunc (hcf )
	{}
	int healthValue() const
	{ return healthFunc( * this); }
private:
	HealthCalcFunc healthFunc;
};


short calcHealth(const GameCharacter&) //健康计算函数:
{
	cout << "calcHealth" << endl;
	return 1;
}


//这之后部分好好琢磨？？
//注意其返回类型为non-int
struct HealthCalculator { //为计算健康而设计的函数对象
	int operator() (const GameCharacter&) const
	{
		cout << "HealthCalculator" << endl;
		return 1;
	}
};
class GameLevel {
public:
	float health(const GameCharacter&) const //成员函数，用以计算健康:
	{
		cout << "GameLevel::health" << endl;
		return 1.0;
	}
	//注意其non-int返回类型
};

class EvilBadGuy: public GameCharacter { // 同前
public:
	explicit EvilBadGuy(HealthCalcFunc hcf = defaultHealthCalc)
		:GameCharacter(hcf)
	{}
};

class EyeCandyCharacter: public GameCharacter { //另一个人物类型:
public:
	explicit EyeCandyCharacter(HealthCalcFunc hcf = defaultHealthCalc)
		:GameCharacter(hcf)
	{}
};


int main() 
{
	GameCharacter gc; 
	gc.healthValue(); //用到boost::function创建的

	//EvilBadGuy 同前
	EvilBadGuy ebg1(calcHealth); //人物1，使用某个函数计算健康指数
	ebg1.healthValue();
	
	HealthCalculator hc;
	EyeCandyCharacter ecc1(hc); //人物2 ，使用某个函数对象计算健康指数
	//EyeCandyCharacter ecc1(HealthCalculator()); //不可用该形式，会被误以为是声明
	ecc1.healthValue(); 

	GameLevel currentLevel;  
	EvilBadGuy ebg2(  //人物3 ，使用某个成员函数计算健康指数
		boost::bind(&GameLevel::health,
		currentLevel,
		_1));
	ebg2.healthValue();
	
	return 0;
}