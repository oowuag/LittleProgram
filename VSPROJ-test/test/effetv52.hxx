#include <iostream>
using namespace std;

//有问题
class Widget{
public:
	static void* operator new(std::size_t size, std::ostream &logStream)
		throw(std::bad_alloc) //非正常形式的new // placement new
	{
		throw (1); //？？throw wrong
		return NULL;
	}
	static void operator delete(void *pMemory, std::ostream &logStream) 
		throw() //placement deletes
	{
	}
	static void operator delete(void *pMemory, std::size_t size)
		throw() //正常的class专属delete
	{
	}
};

//void* operator new(std::size_t size , void* pMemory) throw() //placementnew
//{
//	return NULL;
//}

int main()
{
	Widget* pw1 = new (std::cerr)Widget;	//调用operator new并传递err为其
											//ostream实参:这个动作会在Widget
											//构造函数抛出异常时泄漏内存
	delete pw1;

	return 0;
}
