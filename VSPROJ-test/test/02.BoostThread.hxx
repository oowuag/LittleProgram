#include <boost/thread.hpp>
using namespace boost;
using namespace std;

mutex io_mu; // ����ȫ�ֻ������

/*
* ģ����: �̰߳�ȫ�ļ����������ɿ���
*/
template<typename T>
class basic_atom: noncopyable
{
private:
	T n;
	mutex mu;
public:
	basic_atom(T x = T()) :n(x) {}
	T operator++() 
	{
		//mutex::scoped_lock lock(mu);  //��scoped_lock��ס��Ա�����źű���
		return ++n;
	}
	operator T() { return n; }
};

typedef basic_atom<int> atom_int;  // ��ģ���ඨ�徫����atom_int

int to_interrupt(atom_int& x, const string& str)
{
	try 
	{
		for (int i = 0; i < 10009; ++i) 
		{
			//this_thread::sleep(posix_time::seconds(1));  // �˴�Ӧ�������߳�sleep1��
			//mutex::scoped_lock lock(io_mu); //io�����̰߳�ȫ��������ס
			cout << str << ": " << ++x << endl;
		}
	} 
	catch(thread_interrupted& ) 
	{
		cout << "thread interrupted" << endl;
	}
	return 0;
}

int main()
{
	atom_int x;
	thread t(to_interrupt, ref(x), "hello"); // t����to_interrupt�����������hello x
	this_thread::sleep(posix_time::seconds(13)); //�˴�Ӧ�������߳�˯��3��
	//t.interrupt();  // �����̵߳��жϲ���
	t.join();  // ��Ϊ�߳����жϣ������߳���������

	return 0;
}