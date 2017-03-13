#include <afx.h>

#include<iostream>
#include<vector>
using namespace std;

int main()
{
	//vector<int> v(3);
	//v[0]=2;
	//v[1]=7;
	//v[2]=9;

	////在最前面的元素前插入8 

	//v.insert(v.begin(),8);

	////在第二个元素前插入新元素1
	//v.insert(v.begin()+2,1);

	////在末尾插入新元素1
	//v.insert(v.end(),3);      

	//for(vector<int>::iterator it=v.begin();it!=v.end();it++)
	//	cout<<*it<<endl;
	//system("pause");


	CString   cStrA   =   "A "; 
	//std::string   strB   =   "B "; 

	//strA   =   strB.data();     //   CString   ->   string 
	////strB   =   LPCSTR(strA);   //   string   ->   CString

	//1 string to CString   

	// cStrA.Format(L"%s",strB.c_str()); 

	//2 CString to string

	//CString csTemp; //……………… 
	//CT2CA caTemp(csTemp); 
	//string strTemp(caTemp); 

	//std::string   strB(cStrA.GetBuffer());
	//
	//CString mystring = _TEXT("Hello World");
	////string astring = (LPCTSTR)mystring; // assuming UNICODE is NOT defined
	//string astring = mystring.GetBuffer(0);


	//CString theString( "This is a test" );
	//int     sizeOfString = (theString.GetLength() + 1);
	//LPTSTR  lpsz = new TCHAR[ sizeOfString ];
	//_tcscpy_s(lpsz, sizeOfString, theString);


	CString csTemp;
	CT2CA caTemp(csTemp); 
	string strTemp(caTemp); 
	
	//不如用streamstring实现之

	return 0;    
}