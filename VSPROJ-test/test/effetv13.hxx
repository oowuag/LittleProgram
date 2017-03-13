//P.35. 3.����trl::function ���Strategyģʽ
#include <iostream>
#include "boost/function.hpp"
#include "boost/bind.hpp"
using namespace std;

//boost::function
class GameCharacter;//��ǰ
int defaultHealthCalc(const GameCharacter& gc) //��ǰ
{
	cout << "defaultHealthCalc" << endl;
	return 1;
}

class GameCharacter {
public:
	//HealthCalcFunc�������κ�"�ɵ�����" (callable entity) ���ɱ����ò�����
	//�κμ�����GameCharacter ֮������κμ�����int �Ķ��������¡�
	typedef boost::function<int (const GameCharacter&)> HealthCalcFunc;
	explicit GameCharacter(HealthCalcFunc hcf = defaultHealthCalc)
		:healthFunc (hcf )
	{}
	int healthValue() const
	{ return healthFunc( * this); }
private:
	HealthCalcFunc healthFunc;
};


short calcHealth(const GameCharacter&) //�������㺯��:
{
	cout << "calcHealth" << endl;
	return 1;
}


//��֮�󲿷ֺú���ĥ����
//ע���䷵������Ϊnon-int
struct HealthCalculator { //Ϊ���㽡������Ƶĺ�������
	int operator() (const GameCharacter&) const
	{
		cout << "HealthCalculator" << endl;
		return 1;
	}
};
class GameLevel {
public:
	float health(const GameCharacter&) const //��Ա���������Լ��㽡��:
	{
		cout << "GameLevel::health" << endl;
		return 1.0;
	}
	//ע����non-int��������
};

class EvilBadGuy: public GameCharacter { // ͬǰ
public:
	explicit EvilBadGuy(HealthCalcFunc hcf = defaultHealthCalc)
		:GameCharacter(hcf)
	{}
};

class EyeCandyCharacter: public GameCharacter { //��һ����������:
public:
	explicit EyeCandyCharacter(HealthCalcFunc hcf = defaultHealthCalc)
		:GameCharacter(hcf)
	{}
};


int main() 
{
	GameCharacter gc; 
	gc.healthValue(); //�õ�boost::function������

	//EvilBadGuy ͬǰ
	EvilBadGuy ebg1(calcHealth); //����1��ʹ��ĳ���������㽡��ָ��
	ebg1.healthValue();
	
	HealthCalculator hc;
	EyeCandyCharacter ecc1(hc); //����2 ��ʹ��ĳ������������㽡��ָ��
	//EyeCandyCharacter ecc1(HealthCalculator()); //�����ø���ʽ���ᱻ����Ϊ������
	ecc1.healthValue(); 

	GameLevel currentLevel;  
	EvilBadGuy ebg2(  //����3 ��ʹ��ĳ����Ա�������㽡��ָ��
		boost::bind(&GameLevel::health,
		currentLevel,
		_1));
	ebg2.healthValue();
	
	return 0;
}