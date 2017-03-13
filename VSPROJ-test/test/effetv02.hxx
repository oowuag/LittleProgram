#include <iostream>
using namespace std;

class FileSystem 
{ 
public:
	int numDisks()
	{
		return nDisk;
	}
private:
	int nDisk;
};

FileSystem& tfs ()
{
	static FileSystem fs;
	return fs;
}

class Directory 
{ 
public:
	Directory ( int params );
};

Directory::Directory( int params )
{
	// 同前
	//这个函数用来替换tfs 对象:它在
	//FileSystemclass 中可能是个static 。
	//定义并初始化一个local static 对象，
	//返回一个reference 指向上述对象。
	// 同前
	// 同前，但原本的reference to tfs
	//现在改为tfs ()
	std::size_t disks = tfs().numDisks( );
}

Directory& tempDir() //这个函数用来替换tempDir对象:
{//它在Directoryclass 中可能是个statico
	static Directory td(0); //定义并初始化local static 对象，
	return td; //返回一个reference指向上述对象。
}

class Uncopyable {
protected: //允许derived对象构造和析构
	Uncopyable () {}
	~Uncopyable() { }
private:
	Uncopyable(const Uncopyable&); //但阻止copying
	Uncopyable& operator=(const Uncopyable&);
};



int main()
{
	tempDir();


	typedef std::string AddressLines[4];
	std::string* pal = new AddressLines; 

	delete pal; //行为未有定义!
	delete[] pal; //很好。


	return 0;
}



