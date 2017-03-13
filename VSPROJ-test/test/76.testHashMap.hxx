#include <string>
#include <iostream>
using namespace std;

#if defined(_WIN32)
#include <hash_set>
#include <hash_map>
using namespace stdext;
#else
#include <ext/hash_set>
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif


//define the class
class ClassA{
public:
    ClassA(int a):c_a(a){}
    int getvalue()const { return c_a;}
    void setvalue(int a){c_a=a;}

private:
    int c_a;
};


#if defined(_WIN32)

class hash_and_compare{  
public:
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;
    //  Hash Function    
    size_t operator()(const ClassA &a) const    
    {
        return stdext::hash_value(a.getvalue());
    }      
    //  Hash Comparetor Function      
    bool operator()(const class ClassA & a1, const class ClassA & a2) const{    
        return (a1.getvalue() < a2.getvalue());    
    }    
}; 

#else

//1 define the hash function
struct hash_A{
    size_t operator()(const class ClassA & A) const
    {
        //  return  hash<int>(classA.getvalue());
        return A.getvalue();
    }
};

//2 define the equal function
struct equal_A{
    bool operator()(const class ClassA & a1, const class ClassA & a2) const
    {
        return  (a1.getvalue() == a2.getvalue());
    }
};

#endif

int main()
{
    
#if defined(_WIN32)
    hash_map <ClassA, string, hash_and_compare> hmap;
#else
    hash_map<ClassA, string, hash_A, equal_A> hmap;
#endif

    ClassA a1(12);
    hmap[a1] = "I am 12";
    ClassA a2(198877);
    hmap[a2] = "I am 198877";
    cout<<hmap[a1]<<endl;
    cout<<hmap[a2]<<endl;

    return 0;
}