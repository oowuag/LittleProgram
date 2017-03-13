#include <string>  
#include <map>  
#include <iostream>
using namespace std;

typedef void* (*CreateFuntion)(void);

class ClassFactory
{
public:
    static void* GetClassByName(std::string name)
    {
        std::map<std::string,CreateFuntion>::const_iterator find;
        find = m_clsMap.find(name);
        if(find==m_clsMap.end())
        {
            return NULL;
        }
        else
        {
            return find->second();
        }

    }

    static void RegistClass(std::string name,CreateFuntion method)
    {
        m_clsMap.insert(std::make_pair(name,method));
    }

private:
    static std::map<std::string,CreateFuntion> m_clsMap;
};


std::map<std::string, CreateFuntion> ClassFactory::m_clsMap;  

class RegistyClass
{
public:
    RegistyClass(std::string name, CreateFuntion method)
    {
        ClassFactory::RegistClass(name, method);
    }

};

template<class T, const char name[]>
class Register
{
public:
    Register()
    {
        const RegistyClass tmp=rc;
    }

    static void* CreateInstance()
    {
        return new T;
    }

public:
    static const RegistyClass rc;
};



template<class T,const char name[]>
const RegistyClass Register<T,name>::rc(name, Register<T, name>::CreateInstance);

#define DEFINE_CLASS(class_name) \
    char NameArray[]=#class_name; \
    class class_name:public Register<class_name, NameArray>


#define DEFINE_CLASS_EX(class_name,father_class) \
    char NameArray[]=#class_name; \
    class class_name:public Register<class_name, NameArray>,public father_class

DEFINE_CLASS(CG)
{
public:
    void Display()
    {
        printf("I am Here/n");
    }
};

int main(int tt)
{
    {
        CG aa; // compiler will not omit construct

        CG* tmp=(CG*)ClassFactory::GetClassByName("CG");
        tmp->Display();
    }

    {
        CG* tmp=(CG*)ClassFactory::GetClassByName("CG");
        tmp->Display();
    }


    return 0;
}

