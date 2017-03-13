#include<iostream>
#include<fstream>
using namespace std;

void main()
{
	ifstream fin;
	fin.open("main.cpp",ios::in);
	fin.seekg(ios::beg);
	//get ch1 first and then get ch2
	char ch1,ch2;//
	//cout<<"a/b=?"<<endl;
	//Experiment.........
	while(fin.get(ch1))
	{
		if(ch1=='/')
		{
			fin.get(ch2);
			if(ch2=='/')
			{
				while(fin.get(ch1))
				{
					if(ch1=='\n')
					{
						cout<<'\n';
						break;
					}
					/*************
					Experiment********/
					continue;
				}
			}
			else if(ch2=='*')
			{
				while(fin.get(ch1))
					if(ch1=='/')
						break;
				continue;
			}
			else cout<<ch1<<ch2;
		}//endif
		cout<<ch1;
	}//endwhile
	fin.close();
}