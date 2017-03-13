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
	// ͬǰ
	//������������滻tfs ����:����
	//FileSystemclass �п����Ǹ�static ��
	//���岢��ʼ��һ��local static ����
	//����һ��reference ָ����������
	// ͬǰ
	// ͬǰ����ԭ����reference to tfs
	//���ڸ�Ϊtfs ()
	std::size_t disks = tfs().numDisks( );
}

Directory& tempDir() //������������滻tempDir����:
{//����Directoryclass �п����Ǹ�statico
	static Directory td(0); //���岢��ʼ��local static ����
	return td; //����һ��referenceָ����������
}

class Uncopyable {
protected: //����derived�����������
	Uncopyable () {}
	~Uncopyable() { }
private:
	Uncopyable(const Uncopyable&); //����ֹcopying
	Uncopyable& operator=(const Uncopyable&);
};



int main()
{
	tempDir();


	typedef std::string AddressLines[4];
	std::string* pal = new AddressLines; 

	delete pal; //��Ϊδ�ж���!
	delete[] pal; //�ܺá�


	return 0;
}



