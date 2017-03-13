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
		return	const_cast<char&>( //将op[]返回值的const 移除
			static_cast<const TextBlock&>(*this) //为*this 加上const
			[position] );//调用const op[]
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

	const TextBlock cctb("Hello"); //声明一个常量对象。
	char* pc = (char*)&cctb[0]; //调用const operator []取得一个指针，
	// 指向cctb 的数据。
	*pc = 'J' ; //cctb 现在有了"Jello" 这样的内容。

	cout << pc << endl;

	cout << cctb.getText() << endl;

	return 0;
}