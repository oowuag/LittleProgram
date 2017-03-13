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

        if (size != sizeof(Base)) { //Class专属//如果大小错误，
			return ::operator new(size); //令标准的operator new起而处理
        }

		if (size == 0) {
			size =1;
		}

		while(true) {
			//尝试分配size bytes;
			//if(分配成功)
			//	return (一个指针，指向分配得来的内存);
			 void* pMem = malloc(size); // 调用malloc 取得内存
			 if (pMem)
				 return pMem;

			//分配失败:找出目前的new-handling函数(见下)
			new_handler globalHandler= set_new_handler(0);
			set_new_handler(globalHandler);
			if (globalHandler) (*globalHandler) ();
			else throw std::bad_alloc();
		}
	}
	static void operator delete(void *rawMemory, std::size_t size) throw()
	{
		if (rawMemory == 0) return; //如果将被删除的是个null指针，那就什么都不做。
		
		if (size != sizeof(Base)) {	//Class专属//如果大小错误，令标准版				
			::operator delete(rawMemory); //operator delete处理此一申请。
			return;
		}
		//现在，归还rawMemory所指的内存;
		free(rawMemory);

	}

};
class Derived: public Base //假设Derived未声明operatornew
{ 
private:
	int d;
};

int main()
{
	Derived *pd = new Derived; //这里调用的是Base::operator new
	delete pd;

	int *pi = new int(3);
	delete pi;

	return 0;
}
