#include<iostream>
using namespace std;
class query
{
public:
	void destroy()const
	{
		cout << "query::destroy()" << endl;
		delete this;
	}//伪析构函数
protected:
	virtual ~query()
	{
		cout<<"~query() "<<endl;
	}
};

class Andquery :public query  
{
public:
	virtual ~Andquery()
	{
		cout<<"~Andquery() "<<endl;
	}
};

void main()
{
	query *p = new Andquery;
	p->destroy();  
	Andquery obj;  

	query *p2 = new query;

	query q; //fail
}

//1、析构函数设置成private其实是不明智的，派生类很难受。
//2、设置成protected和private，就是想告诉编译器，这个对象是不能让他自己释放的，你也不能让他定义成功。如上例子，定义query obj是要失败的。 但是可以定义指针，但是不能用delete，因为delete就想到于调用了他的析构函数，而此时无权访问。
//3、伪析构函数的出现，意即这个类的释放完全交由用户自己控制。