
#include <Windows.h>
#include <string>


void FileMapping(void)
{
	//打开共享的文件对象。
	HANDLE m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,
		FALSE,"TestFileMap");
	if (m_hMapFile)
	{
		//显示共享的文件数据。
		LPTSTR lpMapAddr = (LPTSTR)MapViewOfFile(m_hMapFile,FILE_MAP_ALL_ACCESS,
			0,0,0);
		OutputDebugString(lpMapAddr);
	}
	else
	{
		//创建共享文件。
		m_hMapFile = CreateFileMapping( (HANDLE)-1,NULL,
			PAGE_READWRITE,0,1024,"TestFileMap");

		//拷贝数据到共享文件里。
		LPTSTR lpMapAddr = (LPTSTR)MapViewOfFile(m_hMapFile,FILE_MAP_ALL_ACCESS,
			0,0,0);
		std::string strTest("TestFileMap");
		strcpy(lpMapAddr,strTest.c_str());                

		FlushViewOfFile(lpMapAddr,strTest.length()+1);
	}
}

class TestClass
{
public:
	TestClass():
	  x1(0x7246),x2('N') {}
	WORD x1;
	CHAR x2;

	int calcSize()
	{
		int y = (BYTE*)&x2 - (BYTE*)&x1 + 1;
		return y;
	}
};

int main()
{
	//FileMapping();

	//while (1)
	//{
	//}

	//TestClass* cl = new TestClass();
	//cl->calcSize();

	int* x = (int*)0x7246;


	


	return 0;
}