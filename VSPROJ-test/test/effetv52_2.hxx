#include <iostream>
using namespace std;


class StandardNewDeleteForms {
public:
	// normal new/delete
	static void* operator new(std::size_t size) throw(std::bad_alloc)
	{ return ::operator new(size); }
	static void operator delete(void *pMemory) throw()
	{ ::operator delete(pMemory); }

	// placement new/delete
	static void* operator new(std::size_t size, void* ptr) throw()
	{ return ::operator new(size, ptr); }
	static void operator delete(void*pMemory, void* ptr) throw()
	{ return ::operator delete(pMemory, ptr); }

	// nothrownew/delete
	static void* operator new(std::size_t size, const std::nothrow_t& nt) throw()
	{ return ::operator new(size, nt); }
	static void operator delete(void*pMemory, const std::nothrow_t&) throw()
	{ ::operator delete(pMemory); }

};

//凡是想以自定形式扩充标准形式的客户，可利用继承机制及using声明式(见条款33) 取得标准形式:

class Widget : public StandardNewDeleteForms { //继承标准形式
public:
	using StandardNewDeleteForms::operator new; //让这些形式可见
	using StandardNewDeleteForms::operator delete;
	static void* operator new(std::size_t size, //添加一个自定的
		std::ostream& logStrearn)//placement new
		throw(std::bad_alloc);
	static void operator delete(void *pMemory, //添加一个对应的
		std::ostream& logStrearn) //placement delete
		throw() ;
};

int main()
{
	StandardNewDeleteForms *ps = new StandardNewDeleteForms;
	delete ps;

	return 0;
}
