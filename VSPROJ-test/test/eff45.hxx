#include <iostream>
//#include <boost/shared_ptr.hpp>
using namespace std;

class A
{
};

class B : public A
{
};


template<typename T>
class SmartPtr 
{
public:

	template<class Y>
	explicit SmartPtr( Y * p )
		:heldPtr(p)
	{
	}

	template<typename U>  //member template,
	SmartPtr(const SmartPtr<U>& other) //Ϊ������copy���캯��
		:heldPtr(other.get())  //��ʼ��this ��heldptr
	{
		cout << "copy ctor" << endl;
	}

	T* get() const { return heldPtr;}
private:
	T* heldPtr; //���SmartPtr ���е�����(ԭʼ)ָ��
};

template<class T>
class shared_ptr
{
public:
	template<class Y> //���죬�����κμ��ݵ�
	explicit shared_ptr(Y* p); //����ָ�롢
	shared_ptr(shared_ptr const& r); //copy���캯��
	template<class Y>
	shared_ptr(shared_ptr<Y> const& r);//����copy���캯�� 
	shared_ptr& operator=(shared_ptr const& r); //copy assignment����
	template<class Y> 
	shared_ptr& operator=(shared_ptr<Y> const& r); //����copy assignment����
};


int main()
{
	//A a;
	SmartPtr<A> smptr1(B());
	SmartPtr<A> smptr2 = SmartPtr<B>(new B());

	return 0;
}