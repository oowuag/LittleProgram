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
	virtual void draw() // �����ˣ���Ϊ�����������һ��
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
	pc->draw(); //���õ�ȴ��Shape::draw
    Circle *pd = dynamic_cast<Circle*>(pc);
    pd->draw();


	return 0;
}
