#include <iostream>

void* operator new(std::size_t size) throw(std::bad_alloc)
{
	return NULL;
}

void operator delete(void* rawMemory) throw()
{
	if (rawMemory == 0) return; 
}

class Base{
public:
	static void* operator new(std::size_t size) throw(std::bad_alloc)
	{
		using namespace std;

        if (size != sizeof(Base)) { //Classר��//�����С����
			return ::operator new(size); //���׼��operator new�������
        }

		if (size == 0) {
			size =1;
		}

		while(true) {
			//���Է���size bytes;
			//if(����ɹ�)
			//	return (һ��ָ�룬ָ�����������ڴ�);
			 void* pMem = malloc(size); // ����malloc ȡ���ڴ�
			 if (pMem)
				 return pMem;

			//����ʧ��:�ҳ�Ŀǰ��new-handling����(����)
			new_handler globalHandler= set_new_handler(0);
			set_new_handler(globalHandler);
			if (globalHandler) (*globalHandler) ();
			else throw std::bad_alloc();
		}
	}
	static void operator delete(void *rawMemory, std::size_t size) throw()
	{
		if (rawMemory == 0) return; //�������ɾ�����Ǹ�nullָ�룬�Ǿ�ʲô��������
		
		if (size != sizeof(Base)) {	//Classר��//�����С�������׼��				
			::operator delete(rawMemory); //operator delete�����һ���롣
			return;
		}
		//���ڣ��黹rawMemory��ָ���ڴ�;
		free(rawMemory);

	}

};
class Derived: public Base //����Derivedδ����operatornew
{ 
private:
	int d;
};

int main()
{
	Derived *pd = new Derived; //������õ���Base::operator new
	delete pd;

	int *pi = new int(3);
	delete pi;

	return 0;
}
