#include <iostream>
using namespace std;


class FuncBase
{
public:
    virtual int operator() (int,int) = 0;
};

template <class T>
class FuncDo : public FuncBase
{
public:
    typedef int (T::* Typefp_func)(int,int);
    FuncDo(T* pobj, Typefp_func fp_func)
        : m_pThis(pobj)
        , m_func(fp_func)
    {
    }

    virtual int operator() (int x, int y)
    {
        return (m_pThis->*m_func)(x,y);
    }
private:
    T* m_pThis;
    Typefp_func m_func;
};

class function
{
public:
    function()
        :m_funcBase(NULL)
    {
    }

    ~function()
    {
        if (m_funcBase) {
            delete m_funcBase;
            m_funcBase = NULL;
        }
    }
    function& operator=(FuncBase* func)
    {
        if (m_funcBase) {
            delete m_funcBase;
            m_funcBase = NULL;
        }
        m_funcBase = func;
        return *this;
    }

    int operator() (int x, int y)
    {
        return (*m_funcBase)(x, y);
    }
private:
    FuncBase* m_funcBase;
};

#define bind(OBJ,CLS,FUNCNAME) fbind<CLS>(&OBJ,&CLS::FUNCNAME);

template <class T>
FuncBase* fbind(T* pobj, int (T::* fp_func)(int,int))
{
    FuncBase* pBase = new FuncDo<T>(pobj, fp_func);
    return pBase;
}


class CTTA
{
public:
	int MyAdd(int a,int b){
		int c = a + b;
		printf("%d+%d=%d\n",a,b,c);
		return c;
	}
};
class CTTB
{
public:
	int MySub(int a,int b){
		int c = a - b;
		printf("%d-%d=%d\n",a,b,c);
		return c;
	}
};
class CTTC
{
public:
	int MyMax(int a,int b){
		int c = a>=b?a:b;
		printf("MyMax %d %d is %d\n",a,b,c);
		return c;
	}
};

int test1()
{	
	CTTA cobj_a;
	CTTB cobj_b;
	CTTC cobj_c;
	function func;

	func = bind(cobj_a, CTTA, MyAdd);
	func(4,3); //4+3

	func = bind(cobj_b, CTTB, MySub);
	func(4,3); //4-3

	func = bind(cobj_c, CTTC, MyMax);
	func(4,3); //max(4,3)

	return 0;
}


int main()
{
    test1();

    return 0;
}