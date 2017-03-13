//p.39. 2

#include <iostream>
using namespace std;

class Timer {
public:
explicit Timer (int tickFrequency);
virtual void onTick() const; //定时器每滴答一次，此函数就被自动调用一次。
};

//1.粗糙做法 -- private继承
//class Widge: private Timer { 
//private:
//	virtual void onTick() const; //查看Widget 的数据...等等。
//};

//2.良好设计 -- 继承加复合
class Widge {
private:
	class WidgeTimer : public Timer { 
	public:
		virtual void onTick() const; //查看Widget 的数据...等等。
	};

	WidgeTimer timer;
};

int main()
{

}
