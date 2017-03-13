#include <boost/thread.hpp>
using namespace boost;
using namespace std;

mutex io_mu; // 定认全局互斥变量

/*
* 模板类: 线程安全的计数器，不可拷贝
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
		//mutex::scoped_lock lock(mu);  //用scoped_lock锁住成员互斥信号变量
		return ++n;
	}
	operator T() { return n; }
};

typedef basic_atom<int> atom_int;  // 将模板类定义精简名atom_int

int to_interrupt(atom_int& x, const string& str)
{
	try 
	{
		for (int i = 0; i < 10009; ++i) 
		{
			//this_thread::sleep(posix_time::seconds(1));  // 此处应该是子线程sleep1秒
			//mutex::scoped_lock lock(io_mu); //io流非线程安全，必须锁住
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
	thread t(to_interrupt, ref(x), "hello"); // t调用to_interrupt函数，并输出hello x
	this_thread::sleep(posix_time::seconds(13)); //此处应该是主线程睡眠3秒
	//t.interrupt();  // 调用线程的中断操作
	t.join();  // 因为线程已中断，所以线程立即返回

	return 0;
}