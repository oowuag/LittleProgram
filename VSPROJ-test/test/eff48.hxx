#include <iostream>
#include <list>
using namespace std;


//template<typename IterT, typename DistT>
//void advance(IterT& iter, DistT d)
//{
//	if (typeid(typename std::iterator_traits<IterT>::iterator_category)
//		== typeid(std::random_access_iterator_tag)) {
//			iter += d; //���random access��������ʹ�÷������������㡣
//	}else {
//		if (d >= 0) { while (d--) ++iter;} //�����������������
//		else { while (d++) --iter; } //��������++��һ
//	}
//}
//
//
//void advance (std::list<int>::iterator& iter, int d)
//{
//	if (typeid(std::iterator_traits<std::list<int>::iterator>::iterator_category)
//		== typeid(std::random_access_iterator_tag)) {
//			iter += d; //���random access��������ʹ�÷������������㡣
//	}else {
//		if (d >= 0) { while (d--) ++iter;} //�����������������
//		else { while (d++) --iter; } //��������++��һ
//	}
//}

template<unsigned int n> //һ�����: Factorial<n> ��ֵ��
struct Factorial { // n ����Factorial<n-l> ��ֵ��
enum { val = n * Factorial<n-1>::val };
};
template<>
struct Factorial<0> { //�������:
enum { val = 1 }; //Factorial<O> ��ֵ��1
};



int main()
{
	cout << Factorial<5>::val << endl; //public enum item

	return 0;
}