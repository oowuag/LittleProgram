#include   <string>  
#include   <map>  
#include   <iostream>
using namespace std;

typedef void* (*CreateFuntion)(void);
/*
以下为工厂类，用来创建类和注册类
*/
class ClassFactory
{
public:
    static void* GetClassByName(std::string name)
    {
        std::map<std::string,CreateFuntion>::const_iterator find;
        find=m_clsMap.find(name);
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

std::map<std::string,CreateFuntion>   ClassFactory::m_clsMap;

class RegistyClass
{
public:
    RegistyClass(std::string name,CreateFuntion method)
    {
        ClassFactory::RegistClass(name,method);
    }
};

template<class T,const char name[]>
class Register
{
public:
    Register()
    {
        //这个一定要加，因为编译器不保证程序开始时就初始化变量rc
        const RegistyClass tmp=rc;
    }
    static void* CreateInstance()
    {
        printf("CreateInstance");
        return new T;
    }
public:
    static const RegistyClass rc;
};

template<class T,const char name[]>
const RegistyClass Register<T,name>::rc(name,Register<T,name>::CreateInstance);

#define DEFINE_CLASS(class_name) \
    char NameArray[]=#class_name; \
    class class_name:public Register<class_name,NameArray>

#define DEFINE_CLASS_EX(class_name,father_class) \
    char NameArray[]=#class_name; \
    class class_name:public Register<class_name,NameArray>,public father_class

DEFINE_CLASS(CG)
{
public:
    void Display()
    {
        printf("fff");
    }
};

int main(int argc, char* argv[])
{

    CG* tmp=(CG*)ClassFactory::GetClassByName("CG");
    tmp->Display();
    return 0;
}