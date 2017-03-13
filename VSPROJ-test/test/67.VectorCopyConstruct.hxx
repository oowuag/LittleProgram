#include <iostream>  
#include <fstream>  
#include <vector>  
#include <list>  
using namespace std;  

//ofstream out("test.out");  
#define out cout
class CDemo{  
public:  
    CDemo():str(NULL){  
        out << "constructor is called" << endl;  
    }  
    CDemo(const CDemo &cd)  
    {  
        out << "copy constructor is called" << endl;  
        this->str = new char[strlen(cd.str)+1];  
        strcpy(str, cd.str);  
    }  

    ~CDemo(){  
        if(str){  
            out << "destructor is called" << endl;  
            delete[] str;  
        }  
    }  
    char *str;  
};

int main()  
{  
    list <CDemo> *a1 = new list <CDemo>();  
    //a1 -> reserve(1);  
    //out << a1 -> capacity() << endl;  

    CDemo d1;  
    d1.str = new char[32];  
    strcpy(d1.str, "trend micro1");  
    std::out << "/////////////////////////////////" << endl;  
    a1->push_back(d1);  
    out << "/////////////////////////////////" << endl;  
    CDemo d2;  
    d2.str = new char[32];  
    strcpy(d2.str, "trend micro2");  
    out << "/////////////////////////////////" << endl;  
    a1->push_back(d2);  
    out << "/////////////////////////////////" << endl;  
    delete a1;  

    return 0;  
} 