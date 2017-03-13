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

//���������Զ���ʽ�����׼��ʽ�Ŀͻ��������ü̳л��Ƽ�using����ʽ(������33) ȡ�ñ�׼��ʽ:

class Widget : public StandardNewDeleteForms { //�̳б�׼��ʽ
public:
	using StandardNewDeleteForms::operator new; //����Щ��ʽ�ɼ�
	using StandardNewDeleteForms::operator delete;
	static void* operator new(std::size_t size, //���һ���Զ���
		std::ostream& logStrearn)//placement new
		throw(std::bad_alloc);
	static void operator delete(void *pMemory, //���һ����Ӧ��
		std::ostream& logStrearn) //placement delete
		throw() ;
};

int main()
{
	StandardNewDeleteForms *ps = new StandardNewDeleteForms;
	delete ps;

	return 0;
}
