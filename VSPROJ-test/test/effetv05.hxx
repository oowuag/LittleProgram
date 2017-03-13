//p.26.2

#include <iostream>
using namespace std;

class Window 
{
public:
	Window():win(1) {}
	virtual void onResize( ) 
	{
		win = 3;
		cout << win << endl; 
	}

	int win;
};

class SpecialWindow: public Window 
{
public:
	SpecialWindow() { win = 2; }

	virtual void onResize() 
	{
        cout << win << endl; 
		static_cast<Window> (*this) .onResize();
	}
};


int main()
{
	SpecialWindow sw;
	Window *pw = &sw;
	pw->onResize();

	return 0;
}
