#include <iostream>
using namespace std;

//������
class Widget{
public:
	static void* operator new(std::size_t size, std::ostream &logStream)
		throw(std::bad_alloc) //��������ʽ��new // placement new
	{
		throw (1); //����throw wrong
		return NULL;
	}
	static void operator delete(void *pMemory, std::ostream &logStream) 
		throw() //placement deletes
	{
	}
	static void operator delete(void *pMemory, std::size_t size)
		throw() //������classר��delete
	{
	}
};

//void* operator new(std::size_t size , void* pMemory) throw() //placementnew
//{
//	return NULL;
//}

int main()
{
	Widget* pw1 = new (std::cerr)Widget;	//����operator new������errΪ��
											//ostreamʵ��:�����������Widget
											//���캯���׳��쳣ʱй©�ڴ�
	delete pw1;

	return 0;
}
