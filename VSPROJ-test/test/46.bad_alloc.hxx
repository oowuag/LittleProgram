// new_set_new_handler.cpp
// compile with: /EHsc
#include<new>
#include<iostream>
#include <Windows.h>

using namespace std;
void __cdecl newhandler( )
{
   cout << "The new_handler is called:" << endl;
   throw bad_alloc( );
   return;
}


int main( ) 
{
   set_new_handler (newhandler);
   try
   {
	   int n = 5;
      while ( n-- ) 
      {
		  cout << n << "-";
		  char *p = new char[1024*1024*1024]();
         cout << "Allocating 5000000 ints." << endl;
		 Sleep(1);
		 //break;
      }
   }
   catch ( exception e )
   {
      cout << e.what( ) << " xxx" << endl;
   }
}