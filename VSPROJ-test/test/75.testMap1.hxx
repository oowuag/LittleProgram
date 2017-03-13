// map::insert (C++98)
#include <iostream>
#include <map>
#include <string>

using namespace std;

// map insert
int test1()
{
    std::map<char,int> mymap;

    // first insert function version (single parameter):
    mymap.insert ( std::pair<char,int>('a',100) );
    mymap.insert ( std::pair<char,int>('z',200) );

    std::pair<std::map<char,int>::iterator,bool> ret;
    ret = mymap.insert ( std::pair<char,int>('z',500) );
    if (ret.second==false) {
        std::cout << "element 'z' already existed";
        std::cout << " with a value of " << ret.first->second << '\n';
    }

    // second insert function version (with hint position):
    std::map<char,int>::iterator it = mymap.begin();
    mymap.insert (it, std::pair<char,int>('b',300));  // max efficiency inserting
    mymap.insert (it, std::pair<char,int>('c',400));  // no max efficiency inserting

    // third insert function version (range insertion):
    std::map<char,int> anothermap;
    anothermap.insert(mymap.begin(),mymap.find('c'));

    // showing contents:
    std::cout << "mymap contains:\n";
    for (it=mymap.begin(); it!=mymap.end(); ++it)
        std::cout << it->first << " => " << it->second << '\n';

    std::cout << "anothermap contains:\n";
    for (it=anothermap.begin(); it!=anothermap.end(); ++it)
        std::cout << it->first << " => " << it->second << '\n';

    return 0;
}

// bound
int test2()
{
    map<int, string> mapStudent;
    mapStudent[1] =  "student_one";
    mapStudent[3] =  "student_three";
    mapStudent[5] =  "student_five";

    map<int, string>::iterator  iter;
    iter = mapStudent.lower_bound(1);
    cout<<iter->second<<endl;
    iter = mapStudent.lower_bound(2);
    cout<<iter->second<<endl;
    iter = mapStudent.lower_bound(3);
    cout<<iter->second<<endl;
    iter = mapStudent.upper_bound(2);
    cout<<iter->second<<endl;
    iter = mapStudent.upper_bound(3);
    cout<<iter->second<<endl;

    pair<map<int, string>::iterator, map<int, string>::iterator> mapPair;
    mapPair = mapStudent.equal_range(2);
    if(mapPair.first != mapPair.second)
    {
        cout<<"Do not Find"<<endl;
    }
    else
    {
        cout<<"Find"<<endl;
    }

    mapPair = mapStudent.equal_range(3);
    if(mapPair.first != mapPair.second)
    {
        cout<<"Do not Find"<<endl;
    }
    else
    {
        cout<<"Find"<<endl;
    }

    return 0;
}

// word,count
int test3()
{

    map<string, int> wordCount;
    typedef map<string, int> ::iterator valType;
    string word;
    cout << "Enter some words (Ctrl + Z to end):" << endl;
    while (cin >> word)
    { 
        // 如果读入的word存在在容器中，则使键值为word的值++，否则添加以此元素为索引的键，然后键值初始化为0，后++，即为1.
        ++wordCount[word]; 
    }
    cout << "word\t\t" << "times" << endl;
    for (valType iter = wordCount.begin(); iter != wordCount.end(); ++iter)
    {
        cout << (*iter).first << "\t\t" << (*iter).second << endl;
    }
    return 0;
}

// mulit map
int test4()
{
    map<int,map<int,int> >multiMap; //对于这样的map嵌套定义
    map<int, int> temp;    //定义一个map<int, string>变量，对其定义后在插入multiMap

    if (temp.find(1) != temp.end()) {
        cout << temp[1] << endl;
    }

    temp[9] = 9;
    temp[10] = 10;
    multiMap[10] = temp;
    multiMap[10][11]=11;
    multiMap[5][30]=30;
    map<int,map<int,int> >::iterator multitr;  // 以下是如何遍历本multiMap
    map<int,int>::iterator intertr;
    for(multitr=multiMap.begin();multitr!=multiMap.end(); multitr++)
    {
        for(intertr= multitr ->second.begin(); intertr != multitr ->second.end(); intertr ++)
            cout<< multitr->first << " " << intertr->first << " (" << intertr->second << ")" << endl;

    }
    return 0;
}

int main()
{
    //test2();
    test4();

    return 0;
}