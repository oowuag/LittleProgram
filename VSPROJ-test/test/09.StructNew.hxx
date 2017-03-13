//http://msdn.microsoft.com/en-us/library/wewb47ee(v=vs.80).aspx

// C4345.cpp
// compile with: /W2
#include <stdio.h>

struct S* gpS;

struct S
{
   // this class has no user-defined default ctor
   void *operator new (size_t size, void*p, int i)
   {
      ((S*)p)->i = i;   // ordinarily, should not initialize
                        // memory contents inside placement new
      return p;
   }
   int i;
};

int main()
{
   S s;
   // Visual C++ .NET 2003 will default-initialize pS->i
   // by assigning the value 0 to pS->i.
   S *pS2 = new (&s, 10) S();   // C4345
   // try the following line instead
   // S *pS2 = new (&s, 10) S;   // not zero initialized
   printf("%d\n", pS2->i);
}