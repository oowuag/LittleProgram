#include <iostream>
using namespace std;

//struct input_iterator_tag { };
//struct output_iterator_tag { };
//struct forward_iterator_tag: public input_iterator_tag { };
//struct bidirectional_iterator_tag: public forward_iterator_tag { };
//struct random_access_iterator_tag: public bidirectional_iterator_tag { };
//

template<typename IterT, typename DistT> //���ʵ������
void doAdvance(IterT& iter, DistT d , //random access
			   std::random_access_iterator_tag) //������
{
	iter += d;
}

template<typename IterT, typename DistT> 
void doAdvance(IterT& iter, DistT d , //bidireetional
			   std::bidirectional_iterator_tag)//������
{
	if (d >= 0) { while (d--) ++iter; }
	else { while (d++) --iter; }
}

template<typename IterT, typename DistT>
void doAdvance(IterT& iter, DistT d , //input
			   std::input_iterator_tag)//������
{
	if (d < 0 ) {
		throw std::out_of_range("Negative distance"); //����
	}
	while (d--) ++iter;
}

template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d)
{
	doAdvance( //���õ�doAdvance�汾
		iter, d , //��iter ֮�������������
		typename //�������ʵ��ġ�
		std::iterator_traits<IterT>::iterator category()
		);
}

int main()
{
	return 0;
}