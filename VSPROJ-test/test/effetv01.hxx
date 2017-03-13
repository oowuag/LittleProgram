#include <iostream>
#include <string>
using namespace std;


class TextBlock {
public:
	TextBlock(const char* input)
		:text(input)
	{};
	const char& operator[] (std::size_t position) const 
	{ return text[position]; }
	char& operator[] (std::size_t position)
	{ //return text[position]; 
		return	const_cast<char&>( //��op[]����ֵ��const �Ƴ�
			static_cast<const TextBlock&>(*this) //Ϊ*this ����const
			[position] );//����const op[]
	}

	const string getText() const
	{
		return text;
	}
private:
	std::string text;
};

int main()
{ 

	const TextBlock cctb("Hello"); //����һ����������
	char* pc = (char*)&cctb[0]; //����const operator []ȡ��һ��ָ�룬
	// ָ��cctb �����ݡ�
	*pc = 'J' ; //cctb ��������"Jello" ���������ݡ�

	cout << pc << endl;

	cout << cctb.getText() << endl;

	return 0;
}