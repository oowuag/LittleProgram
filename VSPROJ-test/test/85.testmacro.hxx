#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

#define DEF_X 5

int test1()
{
    int x = DEF_X;
    cout << x << endl;
    return 0;
}


//2.stringstream
void func2()
{
	stringstream str("a\nb\n\nc\n\n\nd\n" );
    
    string line;	
	while(getline(str, line))
	{
		cout << line << endl;
	}
}


    std::string removeCountryAustralia(const std::string& phoneNumber)
    {
        std::string outNumber;
        std::string prefix1("+61");
        std::string prefix2("61");
        std::string prefix3("0");
        if (0 == phoneNumber.compare(0, prefix1.length(), prefix1)) {
            outNumber = phoneNumber.substr(prefix1.length());
        }
        else if (0 == phoneNumber.compare(0, prefix2.length(), prefix2)) {
            outNumber = phoneNumber.substr(prefix2.length());
        }
        else if (0 == phoneNumber.compare(0, prefix3.length(), prefix3)) {
            outNumber = phoneNumber.substr(prefix3.length());
        }
        else {
            outNumber = phoneNumber;
        }
        return outNumber;
    }

int main()
{

#define DEF_X 6
    test1();
#undef DEF_X

    func2();


    std::string phonenum = "+61";
    std::string outNumber = removeCountryAustralia(phonenum);


    return 0;
}