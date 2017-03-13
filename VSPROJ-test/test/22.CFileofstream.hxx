//#include <atlstr.h>
#include <afx.h>
#include <iostream>
#include <fstream>
using namespace std;

CFile cLog;

ofstream logStream;


int main()
{
	CString cs = "vs05.txt";

	cLog.Open("C:\\proj\\LOG\\vs05.txt", CFile::modeWrite|CFile::modeCreate);
	cLog.Write(cs, cs.GetLength());
	cLog.Close();

	logStream.open("C:\\proj\\LOG\\streamvs05.txt");
	logStream.write(cs, cs.GetLength());
	logStream.close();


	return 0;
}