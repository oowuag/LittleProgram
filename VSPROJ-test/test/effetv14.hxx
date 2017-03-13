//p.37.

#include <iostream>
using namespace std;

class Shape
{
public:

	virtual void draw(int in = 0)
	{
		cout << "Shape::draw" << endl;
	}
};

class Rectangle : public Shape
{  
public:
	virtual void draw(int in = 1)
	{
		cout << "Rectangle::draw" << endl;
	}
};

class Circle : public Shape
{  
public:
	virtual void draw() // 不虚了，因为跟基类参数不一样
	{
		cout << "Circle::draw" << endl;
	}
};


int main()
{

	Shape *ps;
	Shape *pr = new Rectangle;
	Shape *pc = new Circle;

	pr->draw(1); //Rectangle::draw
	pr->draw(); //Rectangle::draw

	pc->draw(1);//Shape::draw
	pc->draw(); //调用的却是Shape::draw
    Circle *pd = dynamic_cast<Circle*>(pc);
    pd->draw();


	return 0;
}
