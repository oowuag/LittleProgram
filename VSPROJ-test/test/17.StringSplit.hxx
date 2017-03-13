//http://www.cnblogs.com/dongbo/archive/2012/07/13/2590366.html

#include <iostream>
#include <string>
#include <sstream>

#include <vector>
#include <boost/algorithm/string.hpp>

using namespace std;

//1.token
void func1()
{
   char str[] = "DWORD   PulseCounter;	/* パルスカウンタ */";
   char* outer = NULL;
   char* inner = NULL;
   outer = strtok( str, ";");

   char* subouter;

   if (NULL != outer)
   {
	   for( subouter = strtok( outer, " ") ; NULL != subouter; subouter = strtok(NULL, " ") )
	   {
		   printf( "Outer token: %s\n", subouter );
		   //for( inner = strtok( outer, ","); NULL != inner; inner = strtok( NULL, ",") )
		   //{
		   // printf( "Inner token: %s\n", inner );
		   //}
	   }
   }

}

//2.stringstream
void func2()
{
	stringstream str("DWORD   PulseCounter;	/* パルスカウンタ */" );
	string substr;
	string tok;
	getline(str, substr, ';');
	stringstream ssubstr;
	ssubstr << substr;
	while( getline( ssubstr, tok, ' ' ) )
	{
		cout << tok << endl;
	}
}

//3.boost
void func3()
{
   string ss( "DWORD   PulseCounter;	/* パルスカウンタ */" );
   vector<string> tmp;

   // 參炎泣憲催蛍蝕��
   vector<string>& tt = boost::algorithm::split( tmp, ss, boost::algorithm::is_space()||boost::algorithm::is_punct() );
   assert( boost::addressof(tmp) == boost::addressof(tt) );
   copy( tt.begin(),tt.end(),ostream_iterator< string >( cout,"\n" ) );
}

//4.boost iterator
void func4()
{
	string str("DWORD   PulseCounter;	/* パルスカウンタ */");
	boost::algorithm::split_iterator< string::iterator > iStr(str,
		boost::algorithm::token_finder( boost::algorithm::is_any_of( "\t; " ) ));

	boost::algorithm::split_iterator< string::iterator> end;
	while( iStr != end )
	{
		cout<< *iStr << endl;
		++iStr;
	}
}

int main()
{

	func4();


	return 0;
}