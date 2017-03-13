#include <iostream>
#include <list>
using namespace std;


//template<typename IterT, typename DistT>
//void advance(IterT& iter, DistT d)
//{
//	if (typeid(typename std::iterator_traits<IterT>::iterator_category)
//		== typeid(std::random_access_iterator_tag)) {
//			iter += d; //针对random access迭代器，使用法代器算术运算。
//	}else {
//		if (d >= 0) { while (d--) ++iter;} //针对其他迭代器分类
//		else { while (d++) --iter; } //反复调用++或一
//	}
//}
//
//
//void advance (std::list<int>::iterator& iter, int d)
//{
//	if (typeid(std::iterator_traits<std::list<int>::iterator>::iterator_category)
//		== typeid(std::random_access_iterator_tag)) {
//			iter += d; //针对random access迭代器，使用法代器算术运算。
//	}else {
//		if (d >= 0) { while (d--) ++iter;} //针对其他迭代器分类
//		else { while (d++) --iter; } //反复调用++或一
//	}
//}

template<unsigned int n> //一般情况: Factorial<n> 的值是
struct Factorial { // n 乘以Factorial<n-l> 的值。
enum { val = n * Factorial<n-1>::val };
};
template<>
struct Factorial<0> { //特殊情况:
enum { val = 1 }; //Factorial<O> 的值是1
};



int main()
{
	cout << Factorial<5>::val << endl; //public enum item

	return 0;
}