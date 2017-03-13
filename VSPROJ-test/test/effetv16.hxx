//p.39. 2

#include <iostream>
using namespace std;

class Timer {
public:
explicit Timer (int tickFrequency);
virtual void onTick() const; //��ʱ��ÿ�δ�һ�Σ��˺����ͱ��Զ�����һ�Ρ�
};

//1.�ֲ����� -- private�̳�
//class Widge: private Timer { 
//private:
//	virtual void onTick() const; //�鿴Widget ������...�ȵȡ�
//};

//2.������� -- �̳мӸ���
class Widge {
private:
	class WidgeTimer : public Timer { 
	public:
		virtual void onTick() const; //�鿴Widget ������...�ȵȡ�
	};

	WidgeTimer timer;
};

int main()
{

}
