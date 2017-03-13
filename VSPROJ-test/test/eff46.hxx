#include <iostream>
using namespace std;

template<typename T> class Rational; //声明Rational template

template<typename T> //声明helper template
const Rational<T> doMultiply(const Rational<T>& lhs, const Rational<T>& rhs);

template<typename T>
class Rational
{
public:
    friend const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs)
    {
        return doMultiply(lhs, rhs); //令friend 调用helper
	}
};

int main()
{
	return 0;
}