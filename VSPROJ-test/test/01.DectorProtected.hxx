#include<iostream>
using namespace std;
class query
{
public:
	void destroy()const
	{
		cout << "query::destroy()" << endl;
		delete this;
	}//α��������
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

//1�������������ó�private��ʵ�ǲ����ǵģ�����������ܡ�
//2�����ó�protected��private����������߱���������������ǲ��������Լ��ͷŵģ���Ҳ������������ɹ����������ӣ�����query obj��Ҫʧ�ܵġ� ���ǿ��Զ���ָ�룬���ǲ�����delete����Ϊdelete���뵽�ڵ�����������������������ʱ��Ȩ���ʡ�
//3��α���������ĳ��֣��⼴�������ͷ���ȫ�����û��Լ����ơ�