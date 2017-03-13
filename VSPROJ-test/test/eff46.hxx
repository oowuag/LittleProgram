#include <iostream>
using namespace std;

template<typename T> class Rational; //����Rational template

template<typename T> //����helper template
const Rational<T> doMultiply(const Rational<T>& lhs, const Rational<T>& rhs);

template<typename T>
class Rational
{
public:
    friend const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs)
    {
        return doMultiply(lhs, rhs); //��friend ����helper
	}
};

int main()
{
	return 0;
}