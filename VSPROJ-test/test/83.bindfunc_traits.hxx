#include <iostream>
using namespace std;

class MyFuncBase
{
public:
	virtual int operator() (int,int) = 0;
};

template<class CS>
class MyFuncObj: public MyFuncBase
{
public:
	typedef int (CS::* fp_func)(int,int);
	MyFuncObj(CS* pThis, fp_func pfc): m_pThis(pThis), m_pfc(pfc) {}
	int operator() (int x, int y) { return (m_pThis->*m_pfc)(x,y); }
private:
	CS* m_pThis;
	fp_func m_pfc;
};

template<class CLS>
MyFuncBase * fbind(CLS* pc, int (CLS::* fp_func)(int,int))
{
	MyFuncBase* pf = new MyFuncObj<CLS>(pc, fp_func);
	return pf;
}
#define bind(OBJ,CLS,FUNCNAME) fbind<CLS>(&OBJ,&CLS::FUNCNAME);

class function
{
public:
	function(MyFuncBase * pf=NULL)
        :m_pfbase(pf)
    {
    }

	~function()
	{
		if (NULL!=m_pfbase)
		{
			delete m_pfbase;
			m_pfbase = NULL;
		}
	}

	function& operator=(MyFuncBase * pf)
    {
		m_pfbase = pf;
		return *this;
	}

	int operator() (int x, int y)
    { 
		if (NULL!=m_pfbase)
		{
			return (*m_pfbase)(x,y); 
		}
		return 0;
	}
private:
	MyFuncBase * m_pfbase;
};


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

	func = bind(cobj_a,CTTA,MyAdd);
	func(4,3); //4+3

	func = bind(cobj_b,CTTB,MySub);
	func(4,3); //4-3

	func = bind(cobj_c,CTTC,MyMax);
	func(4,3); //max(4,3)

	return 0;
}

//template <class I>
//typename I func0(I ite) {
//    typedef I value_type;
//    value_type* ptr = ite; 
//    return (*ptr);
//};
//cout<<func0(&x)<<endl;

//1.MyIter
template <class T>
struct MyIter {
    typedef T value_type;
    T* ptr;
    MyIter(T* p=0) : ptr(p){}
    T& operator*() const {return *ptr;}
};

//2.traits
template <class I>
struct iterator_traits2 {
    typedef typename I::value_type value_type;
};

template <class T>
struct iterator_traits2<T*> {
    typedef T value_type;
};
 
template <class I>
typename iterator_traits2<I>::value_type func(I ite) {
    return *ite;
};

int test2()
{
    MyIter<int> ite(new int(8));
    cout<< func(ite)<<endl;

    int *p = new int(520);
    cout<<func(p)<<endl;

    int x = 3;
    cout<<func(&x)<<endl;

    //cout<<typeid(&x).name() << endl;

    return 0;
}

int main()
{
    //test1();
    test2();

    return 0;
}