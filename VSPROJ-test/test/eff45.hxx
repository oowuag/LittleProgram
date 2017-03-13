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
	SmartPtr(const SmartPtr<U>& other) //为了生成copy构造函数
		:heldPtr(other.get())  //初始化this 的heldptr
	{
		cout << "copy ctor" << endl;
	}

	T* get() const { return heldPtr;}
private:
	T* heldPtr; //这个SmartPtr 持有的内置(原始)指针
};

template<class T>
class shared_ptr
{
public:
	template<class Y> //构造，来自任何兼容的
	explicit shared_ptr(Y* p); //内置指针、
	shared_ptr(shared_ptr const& r); //copy构造函数
	template<class Y>
	shared_ptr(shared_ptr<Y> const& r);//泛化copy构造函数 
	shared_ptr& operator=(shared_ptr const& r); //copy assignment函数
	template<class Y> 
	shared_ptr& operator=(shared_ptr<Y> const& r); //泛化copy assignment函数
};


int main()
{
	//A a;
	SmartPtr<A> smptr1(B());
	SmartPtr<A> smptr2 = SmartPtr<B>(new B());

	return 0;
}