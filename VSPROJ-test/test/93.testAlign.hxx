#include <stdio.h>

#pragma pack(push)
#pragma pack(4)
template<bool> struct CompileTimeAssert;
template<> struct CompileTimeAssert<true> {};
#define COMPILE_TIME_ASSERT_FUNCTION_SCOPE(_exp) \
    CompileTimeAssert<( _exp )>();

#define PAD_SIZE(s) (((s)+3)&~3)

template <typename T>
void foo(T *pArg)
{
    COMPILE_TIME_ASSERT_FUNCTION_SCOPE(PAD_SIZE(sizeof(T)) == sizeof(T));
}



template<int> struct TestCompile;
template<> struct TestCompile<1> {};

int main()
{
    short x;
    printf("%d\n", sizeof(x));
    //foo(&x);


    CompileTimeAssert<true>();

    TestCompile<1>();

    return 0;
}

#pragma pack(pop)
