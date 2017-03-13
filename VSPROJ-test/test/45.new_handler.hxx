 //new_handler example
#include <iostream>     // std::cout
//#include <cstdlib>      // std::exit
//#include <new>          // std::set_new_handler

//void no_memory () {
//  std::cout << "Failed to allocate memory!\n";
//  //std::exit (1);
//}
//
//int main () {
//  //std::set_new_handler(no_memory);
//  std::cout << "Attempting to allocate 1 GiB...\n";
//  
//  char* p = NULL; 
//  
// // try {
//	//p = new char [1024*1024*991];
// // }
// // catch(std::exception  &exp){
//	//  std::cout << "Attempting to allocate 1 GiB...\n";
// // }
//  
//  p = new(std::nothrow) char [1024*1024*991];
//
//
//  std::cout << "Ok\n";
//  delete[] p;
//  return 0;
//}


//#ifndef _PTRDIFF_T_DEFINED
//typedef int ptrdiff_t;
//#define _PTRDIFF_T_DEFINED
//#endif
//
//#ifndef _FARQ   /* specify standard memory model */
// #define _FARQ
// #define _PDFT  ptrdiff_t
// #define _SIZT  size_t
//#endif
//
//
////		// TEMPLATE OPERATOR new
////template<class _Ty>
////	void *operator new(size_t _N, std::allocator<_Ty>& _Al)
////	{
////		return (_Al._Charalloc(_N)); 
////	}
//
//namespace my{
//	void *__CRTDECL operator new(size_t, void *_Where)
//	{
//		return (_Where); 
//	}
//
//    //void* operator new (std::size_t size) throw (std::bad_alloc) {
//    //    return malloc(size);
//    //}
//}
//
//using namespace my;
//
//#define new_
//
//int main() 
//{
//	int *pn =new int;
//
//
//
//}

//3.

//void* operator new(std::size_t size) throw(std::bad_alloc)
//{
//	return ::operator new(size);
//}

void* my_new(std::size_t size) throw(std::bad_alloc)
{
	return new int[size];
}


int main()
{
	int *p = NULL;
	p = (int *)my_new(1024*1024*1024);

	return 0;
}