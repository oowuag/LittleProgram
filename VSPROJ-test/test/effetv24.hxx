#include <iostream>
using namespace std;


class Rational 
{
public:
	Rational()
		:x(0),y(0)
	{}

	Rational(int a)
		:x(a),y(1)
	{}

	Rational(int a, int b)
		:x(a),y(b)
	{}

	int numerator() const
	{
		return x;
	}

	int denominator() const
	{
		return y;
	}

	Rational operator*(const Rational& lhs)
	{
		x *= lhs.numerator();
		y *= lhs.denominator();
		return *this;
	}

private:
	int x;
	int y;
};

const Rational operator*(const Rational& lhs,
						 const Rational& rhs)
{//现在成了一个non-member函数
	return Rational(lhs.numerator() * rhs.numerator( ),
		lhs.denominator() * rhs.denominator());
}

int main()
{
	Rational oneFourth(1 , 4);
	Rational result;
	result = oneFourth * 2;
	result = 2 * oneFourth;

	return 0;
}