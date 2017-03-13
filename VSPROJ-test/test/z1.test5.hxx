#include <string>
#include <stdio.h>

using namespace std;


int ATE_AdbCmd(char* szCmd) 
{
	string* strCmd = NULL;
	strCmd = new string(szCmd);

	// delete tail space
	if(strCmd->empty()) {
		return 0;
	}
	strCmd->erase(0,strCmd->find_first_not_of(" "));//head space
	strCmd->erase(strCmd->find_last_not_of(" ")+1); //tail space

	int bj=0;
	int ej=0;
	char *param[10];
	int iParamCnt = 0;
	char tempCCre[10][128];
	
	while(ej = strCmd->find(" ",bj))
	{
		if (ej<1 && bj < strCmd->size())
		{
			strcpy(tempCCre[iParamCnt],(strCmd->substr(bj,strCmd->size()-bj)).c_str());
			param[iParamCnt] = tempCCre[iParamCnt];
			iParamCnt++;
			break;
		}
		strcpy(tempCCre[iParamCnt],(strCmd->substr(bj,ej-bj)).c_str());
		param[iParamCnt] = tempCCre[iParamCnt];

		
		iParamCnt++;

		bj = ej+1;
		ej = 0;
	}
	delete strCmd;
	strCmd = NULL;

    return 0;
	//return adb_main(iParamCnt,param);	
}

int main()
{
    ATE_AdbCmd("/data/autokey_tool&");

    return 0;
}