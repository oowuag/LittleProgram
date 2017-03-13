#include <stdio.h>
#include <Windows.h>
#include <iostream>

#include <string>
using namespace std;

UINT codePage = 936;
//gbk to utf8
string GBKToUTF8(const string& strGBK)
{
    string strOutUTF8 = "";
    WCHAR * str1;
    int n = MultiByteToWideChar(codePage, 0, strGBK.c_str(), -1, NULL, 0);
    str1 = new WCHAR[n];
    MultiByteToWideChar(codePage, 0, strGBK.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char * str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    strOutUTF8 = str2;
    delete[]str1;
    str1 = NULL;
    delete[]str2;
    str2 = NULL;
    return strOutUTF8;
}

//utf-8 to gbk
string UTF8ToGBK(const string& strUTF8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
    unsigned short * wszGBK = new unsigned short[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);

    len = WideCharToMultiByte(codePage, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
    char *szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(codePage,0, (LPWSTR)wszGBK, -1, szGBK, len, NULL, NULL);
    //strUTF8 = szGBK;
    std::string strTemp(szGBK);
    delete[]szGBK;
    delete[]wszGBK;
    return strTemp;
}

int main()
{
    UINT oldcodepage = GetConsoleOutputCP();
    SetConsoleOutputCP(65001);

    string x = "����";
    printf("%s\n", x.c_str());

    x = GBKToUTF8(x);
    printf("%s\n", x.c_str());

    SetConsoleOutputCP(oldcodepage);
    return 0;
}